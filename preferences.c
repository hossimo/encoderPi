#include "preferences.h"

//globals
Conf currentConf;

/**
 ** getDefault - returns a default configuration struct.
 **/
 Conf getDefault (void){
    Conf result;
    result.universe = 0;
    result.startAddress = 1;
    result.gpioA = 10;
    result.gpioB = 11;
    result.artnetUpdateRate = 30;
    result.dataBits = 16;
    strcpy(result.sendToAddress, "255.255.255.255");
    return result;
}


/**
 ** openPrefs - returns a FILE* to the current default file.
 **/
FILE* openPrefs (const char *mode){
    // if path does not exits then make it
    struct stat st = {0};
    if (stat(kCONFPATH, &st) == -1){
        mkdir(kCONFPATH, 0777);
    }

    // if file does not exits, make it and set defaults
    if (access(kFQPN, W_OK)){
        FILE* fp = fopen(kFQPN, "w+");
        fclose(fp);
        chmod(kFQPN, 0666);

        Conf defaults = getDefault();
        writePrefs (defaults);
    }
    //now open it for real
    FILE* fp = fopen(kFQPN, mode);
    return fp;
}

/**
 ** writePrefs - write a CONF struct to the current file
 **/
void writePrefs (const Conf conf)
{
    FILE *fp = openPrefs ("w");
    //TODO: need to check for valid FILE*
    
    // poor mans conf file.
    const int MAX_BUF = 1024;
    char buffer[MAX_BUF];
    int length = 0;

    length += snprintf(buffer + length, MAX_BUF - length, "universe         %d\n", conf.universe);
    length += snprintf(buffer + length, MAX_BUF - length, "startAddress     %d\n", conf.startAddress);
    length += snprintf(buffer + length, MAX_BUF - length, "dataBits         %d\n", conf.dataBits);
    length += snprintf(buffer + length, MAX_BUF - length, "gpioA            %d\n", conf.gpioA);
    length += snprintf(buffer + length, MAX_BUF - length, "gpioB            %d\n", conf.gpioB);
    length += snprintf(buffer + length, MAX_BUF - length, "artnetUpdateRate %d\n", conf.artnetUpdateRate);
    length += snprintf(buffer + length, MAX_BUF - length, "sendToAddress    %s\n", conf.sendToAddress);

    fwrite(buffer, length, 1, fp);
    fclose(fp);
}

/**
 ** readPrefs - write a CONF struct to the current file
 **/
Conf readPrefs (){
    Conf readConf;
    FILE *fp = NULL;
    const int MAX_BUFFER = 128;
    char line[MAX_BUFFER];
    struct stat st = {0};
    
    // has the file changed?
    
    // if there is no file then create a default one and rebuild stat
    int statResult = stat(kFQPN, &st);
    if (statResult != 0) {
        printf("Configuration file missing, creating a default file\n");
        Conf defaultConf = getDefault();
        writePrefs(defaultConf);
        statResult = stat(kFQPN, &st);
    }
    
    if (st.st_mtime != currentConf.lastTime){
        currentConf.lastTime = st.st_mtime;
        fp = openPrefs("r");
        while (fgets(line, MAX_BUFFER, fp)) {
            char key[MAX_BUFFER];
            int value;
            
            sscanf(line, "%s", key);
            // if key value is a string
            if (strcmp(key, "sendToAddress") == 0) {
                sscanf(line, "%s %s", key, readConf.sendToAddress);
            } else {
                sscanf(line, "%s %d", key, &value);
                if (strcmp(key, "universe") == 0)
                    readConf.universe = value;
                else if (strcmp(key, "startAddress") == 0)
                    readConf.startAddress = value;
                else if (strcmp(key, "gpioA") == 0)
                    readConf.gpioA = value;
                else if (strcmp(key, "gpioB") == 0)
                    readConf.gpioB = value;
                else if (strcmp(key, "artnetUpdateRate") == 0)
                    readConf.artnetUpdateRate = value;
                else if (strcmp(key, "dataBits") == 0)
                    readConf.dataBits = value;
            }
        }
        currentConf = readConf; //TODO: need to guard this!
    }
    
    if (fp)
        fclose(fp);
    
    return currentConf;
}

int setPrefs (Conf prefs){
    int result = 0;

    return result;
}

/**
 ** dumpCurrentConf - print current conf
 **/
void dumpCurrentConf (){
    //debug data:
    printf("lastTime         %ld\n", currentConf.lastTime);
    printf("universe         %d\n",  currentConf.universe);
    printf("startAddress     %d\n",  currentConf.startAddress);
    printf("dataBits         %d\n",  currentConf.dataBits);
    printf("gpioA            %d\n",  currentConf.gpioA);
    printf("gpioB            %d\n",  currentConf.gpioB);
    printf("artnetUpdateRate %d\n",  currentConf.artnetUpdateRate);
    printf("sendToAddress    %s\n",  currentConf.sendToAddress);
}

