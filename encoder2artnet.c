#include "encoder2artnet.h"

#define kMAX_GPIO 2

// globals
static volatile int keepRunning = 1;            // handled by intHandler
int gpioValues[kMAX_GPIO];                      // GPIOValues;
const int pattern [4] = {0, 1, 3, 2};           // the Grey coding pattern of the encoder
const int gpioCount = kMAX_GPIO;                // count of GPIOs, currently only support 2
int fpValue [kMAX_GPIO];                        // file pointer for each GPIO value
struct pollfd fds[kMAX_GPIO];                   // file change poll for each GPIO
struct pollfd fdsConf;                          // file change poll for Conf
long counter = 0;                               // position counter

Conf conf; // TODO: Do we really need this?

// enums
typedef enum {kWAIT, kRIGHT, kLEFT} encodermode;

//
// main
// TODO: handle arguments
int main(int argc, const char * argv[])
{
    printf("encoder2Artnet starting up.\n\n");
    
    // setup signal handler
    signal (SIGINT, intHandler);
    
    // start conf thread
    int threadResult;
    pthread_t threadConf;
    threadResult = pthread_create(&threadConf, NULL, &conf_thread, NULL);
    if (threadResult) {
        printf("failed to create configuration worker thread [%d]\n", threadResult);
    }
    
    // start artnet thread
    pthread_t threadArtnet;
    threadResult = pthread_create(&threadArtnet, NULL,&artnet_thread, NULL);
    if (threadResult) {
        printf("failed to create artnet thread [%d]\n", threadResult);
    }
    
    // turn the cursor off and print the debug header
    printf("\n\e[?25l");
    printf("\n");
    printf("GPIO 10│11\n");
    printf("     ──┼──\n");
    fflush(stdout);
    
    encodermode mode = kWAIT;
    int lastPosition = -1;
    int currentPosition = -1;
    int cycle = -1;
    while (keepRunning){
       if ((fds[0].fd == 0)  && (fds[1].fd == 0)) {
           struct timespec ts;
           ts.tv_sec = 0;
           ts.tv_nsec = 1000000000/2;
           nanosleep(&ts, NULL);
            continue;
        }
        
        int ret = poll(fds, gpioCount, -1);
        
        //check for error
        if (ret <= 0)
            printf("PollError %#08x", ret);
        
        // check each fds for servicing
        for (int i = 0; i < gpioCount ; i++) {
            if (fds[i].revents == (POLLPRI | POLLERR)) {
                char s;
                lseek(fds[i].fd, 0, SEEK_SET); // seek back to the beginning
                read (fds[i].fd, &s, 1);       // peel off the first character
                gpioValues[i] = s - '0';       // convert to int.
            }
        }
        
        // Process each the current position
        // This needs to be on its own thread?
        
        // calculate the current position
        currentPosition = gpioValues[0] + (gpioValues[1] << 1);
        
        // if we havent moved then start over
        if (lastPosition == currentPosition)
            continue;
        
        //not yet sure where we are
        if (mode == kWAIT) {
            for (int i = 0; i <= 3; i++) {
                if (pattern[i] == currentPosition) {
                    cycle = i;
                    break;
                }
            }
        }
        
        // if the last is +/- 1 place in the pattern then adjust the cycle
        if (lastPosition == pattern[cycle == 3 ? 0 : cycle + 1]) {
            mode = kLEFT;
            cycle = cycle == 0 ? 3 : --cycle;
            counter++;
        }
        else if (lastPosition == pattern[cycle == 0 ? 3 : cycle - 1]) {
            mode = kRIGHT;
            cycle = cycle == 3 ? 0 : ++cycle;
            counter--;
        }
        else // we skipped 1 or more ticks.
            mode = kWAIT;
        
        // build direction indicator
        char resultString[3];
        memset (resultString, 0, sizeof(resultString));
        switch (mode) {
            case kWAIT:
                strncpy(resultString, "--", 2);
                break;
            case kRIGHT:
                strncpy(resultString, ">>", 2);
                break;
            case kLEFT:
                strncpy(resultString, "<<", 2);
                break;
        }
        
        printf("\e[100D      %d│ %d %s %ld    ",
               gpioValues[0],
               gpioValues[1],
               resultString,
               counter
               );
        
        fflush(stdout) ;
        lastPosition = currentPosition;
    }
    
    // turn the curson back on
    printf("\n\n");
    printf("\e[?25h");
    
    return 0;
    
}


void setup_io()
{
    Conf conf = readPrefs();
    int fp;                                     //generic file pointer
    int gpios[2] = {conf.gpioA, conf.gpioB};
    
    for (int i = 0 ; i < gpioCount ; i++){
        char string[128];
        // enable for user
        //TODO: user needs to be in the gpio group
        strncpy(string,"/sys/class/gpio/export", 128);
        fp = open(string, O_WRONLY);
        if (fp == -1) {
            printf("Error opening %s\n",string);
            close(fp);
        } else {
            // write port for userspace
            sprintf(string, "%d\n", gpios[i]);
            write(fp, &string, 128);
        }
        close(fp);
        
        //write port direction
        //TODO: for some reason this file is generated as root:root
        sprintf (string, "/sys/class/gpio/gpio%d/direction", gpios[i]) ;
        fp = open(string, O_WRONLY);
        if (fp == -1) {
            printf("Error opening %s\n",string);
            close(fp);
        }
        sprintf (string, "in\n") ;
        write(fp, &string, 128);
        close(fp);
        
        //pointer to value
        //TODO: for some reason this file is generated as root:root
        sprintf (string, "/sys/class/gpio/gpio%d/value", gpios[i]) ;
        fpValue[i] = open(string, O_RDONLY);
        if (fpValue[i] == -1) {
            printf("Error opening %s\n",string);
        }
        
        // write configure interrupt
        //TODO: for some reason this file is generated as root:root
        sprintf (string, "/sys/class/gpio/gpio%d/edge", gpios[i]) ;
        fp = open(string, O_WRONLY);
        if (fp == -1) {
            printf("Error opening %s\n",string);
        }
        sprintf (string, "both\n") ;
        write(fp, &string, 128);
        close(fp);
        
        // configure fds strut
        fds[i].fd = fpValue[i];
        fds[i].events = POLLPRI | POLLERR;
    }
}

void destroy_io(Conf conf)
{
    printf("Destroying previous GPIO\n");
    int fp;        //generic file pointer
    char string[128];
    int gpios[kMAX_GPIO] = { conf.gpioA , conf.gpioB };

    for (int i = 0 ; i < gpioCount ; i++){
        close(fpValue[i]);
        
        strncpy(string,"/sys/class/gpio/unexport", 128);
        fp = open(string, O_WRONLY);
        if (fp == -1) {
            printf("Error opening %s\n",string);
            close(fp);
        } else {
            // write port for userspace
            sprintf(string, "%d\n", gpios[i]);
            write(fp, &string, 128);
        }
        close(fp);
    }
}

//
// void intHandler (int dummy) {
//
void intHandler (int dummy) {
    keepRunning = 0;
}

//
// Threads
// Worker threads:
// artnet  - responsible for sending Art-Net packets
// conf    - responsible for reading the configuration file and setting up the GPIOs
// encoder - responsible reading GPIOs
//

// void *artnet_thread (void *arg)
void *artnet_thread (void *arg) {
    
    //init Art-Net socket
    int socket = buildSocket();
    struct timespec ts, ts2;
    
    while (keepRunning) {
        if (conf.artnetUpdateRate){
            ts.tv_sec = 0;
            ts.tv_nsec = conf.artnetUpdateRate == 0 ? 1 : 1000000000/conf.artnetUpdateRate;
            sendPacket (socket, counter);
        } else {
            ts.tv_sec = 0;
            ts.tv_nsec = 1000000000/2;
        }
        nanosleep(&ts, &ts2);
    }
    return NULL;
}

// void *conf_thread (void *arg)
void *conf_thread (void *arg){
    while (keepRunning){
        Conf newConf = readPrefs();
        // if this is the first time through.
        if (conf.lastTime == 0) {
            setup_io(readPrefs());
            conf = newConf;
        }
        
        // if the GPIOs are diffrent
        if ((newConf.gpioA != conf.gpioA) || (newConf.gpioB != conf.gpioB)) {
            destroy_io(conf);
            setup_io(newConf);
        }
        // copy to global conf (though it should always point to the same place)
        conf = newConf;
        
        // sleep until next check
        sleep(2);
    }
    return NULL;
}

// void *encoder_thread (void *arg)
void *encoder_thread (void *arg){
    return NULL;
}