#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "preferences.h"
#include "artnetsender.h"

// Function definitions
void setup_io(Conf* conf);
void destroy_io(Conf* conf);
void printButton(int g);
void intHandler (int dummy);
void *artnet_thread (void *arg);
void *conf_thread (void *arg);
