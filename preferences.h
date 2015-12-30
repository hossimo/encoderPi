#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
//#include <errno.h>


#define kCONFPATH "/var/local/encoder2artnet/"
#define kCONFFILE "encoder2artnet.conf"
#define kFQPN     kCONFPATH kCONFFILE

#ifndef CONF_TYPEDEF
#define CONF_TYPEDEF
typedef struct Conf {
  time_t lastTime;
  unsigned int universe;
  unsigned int startAddress;
  unsigned int dataBits;
  unsigned int gpioA;
  unsigned int gpioB;
  unsigned int artnetUpdateRate;
  char sendToAddress[INET_ADDRSTRLEN];
} Conf;
#endif

Conf readPrefs (void);
void writePrefs (Conf prefs);
FILE* openPrefs (const char *mode);

//debug:
void dumpCurrentConf ();