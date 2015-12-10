#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>

#include <poll.h>
#include <pthread.h>
#include <semaphore.h>

#include "artnetsender.h"

// fuction definations
void setup_io();
void destroy_io();
void printButton(int g);
void intHandler (int dummy);
