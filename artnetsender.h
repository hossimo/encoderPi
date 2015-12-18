#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "preferences.h"



//DMX Packet
typedef struct {
    char ID[8];
    uint16_t OpCode;
    uint16_t version; /*Protocol Version High and Low*/
    uint8_t  seq;
    uint8_t  physical;
    uint8_t  subUni;
    uint8_t  net;
    uint16_t length;
    uint8_t  data[512];
} ArtnetDmxHeader;

// Type of Channel to send
typedef enum ArtnetDataSize {
    k8Bit  = 8,
    k16Bit = 16,
    k24Bit = 24,
    k32Bit = 32
} dataSize;

int buildSocket (void);
size_t sendPacket (unsigned int socket, Conf *conf, unsigned long value);
