#include "encoder2artnet.h"

#define kMAX_GPIO 32

// globals
static volatile int keepRunning = 1;            // handled by intHandler
int gpioValues[kMAX_GPIO];                      // GPIOValues;
const int pattern [4] = {0, 1, 3, 2};           // the pattern of the encoder
const int gpios [] = {10, 11};                  // what GPIOs to watch
const int gpioCount = sizeof(gpios)/sizeof(int); // count of GPIOs
int fpValue [kMAX_GPIO];                        // file pointer for each GPIO value
struct pollfd fds[kMAX_GPIO];                   // file change poll for each GPIO

// enums
typedef enum {kWAIT, kRIGHT, kLEFT} encodermode;

//
// main
// TODO: handel arguments
// TODO: watch a config file
int main(int argc, char **argv)
{
  //init Art-Net socket
  int socket = buildSocket();

  // setup signal handler
  signal (SIGINT, intHandler);

  // setup GPIOs
  setup_io();

  // Switch GPIO 7..11 to output mode

  /************************************************************************\
  * You are about to change the GPIO settings of your computer.          *
  * Mess this up and it will stop working!                               *
  * It might be a good idea to 'sync' before running this program        *
  * so at least you still have your code changes written to the SD-card! *
  \************************************************************************/

  // Set GPIO pins 7-11 to inputs
  // for (int g=7; g<=11; g++)
  // {
  //   INP_GPIO(g); // must use INP_GPIO before we can use OUT_GPIO
  // }

  // turn the cursor off
  printf("\n\e[?25l");
  printf("\n");
  printf("GPIO 10│11\n");
  printf("     ──┼──\n");
  fflush(stdout);

  encodermode mode = kWAIT;
  int lastPosition = -1;
  int currentPosition = -1;
  int cycle = -1;
  long counter = 0;
  while (keepRunning){
    // block until file has changed.
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
        if (pattern[i] == currentPosition)
        {
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
  //TODO: fork() artnet stuff.
  sendPacket (socket, 0, 0, k16Bit, counter);

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
  int fp;        //generic file pointer
  for (int i = 0 ; i < gpioCount ; i++){
    char string[128];
    // enable for user
    //TODO: user needs to be in the gpio group
    strncpy(string,"/sys/class/gpio/export", 128);
    fp = open(string, O_WRONLY);
    if (fp == -1) {
      printf("Error opening %s\n",string);
      close(fp);
      exit(1);
    }
    // weite port for userspace max 127 char
    sprintf(string, "%d\n", gpios[i]);
    write(fp, &string, 128);
    close(fp);

    //write port direction
    //TODO: for some reason this file is greated as root:root
    sprintf (string, "/sys/class/gpio/gpio%d/direction", gpios[i]) ;
    fp = open(string, O_WRONLY);
    if (fp == -1) {
      printf("Error opening %s\n",string);
      close(fp);
      exit(2);
    }
    sprintf (string, "in\n") ;
    write(fp, &string, 128);
    close(fp);

    //pointer to value
    //TODO: for some reason this file is greated as root:root
    sprintf (string, "/sys/class/gpio/gpio%d/value", gpios[i]) ;
    fpValue[i] = open(string, O_RDONLY);
    if (fpValue[i] == -1) {
      printf("Error opening %s\n",string);
      exit(2);
    }

    // wtrite configure interupt
    //TODO: for some reason this file is greated as root:root
    sprintf (string, "/sys/class/gpio/gpio%d/edge", gpios[i]) ;
    fp = open(string, O_WRONLY);
    if (fp == -1) {
      printf("Error opening %s\n",string);
      exit(2);
    }
    sprintf (string, "both\n") ;
    write(fp, &string, 128);
    close(fp);

    // configure fds struct
    fds[i].fd = fpValue[i];
    fds[i].events = POLLPRI | POLLERR;
  }
}

//
// void intHandler (int dummy) {
//
void intHandler (int dummy) {
  keepRunning = 0;
}
