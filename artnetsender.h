#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>



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
    k8Bit,
    k16Bit,
    k24Bit,
    k32Bit
} dataSize;

int buildSocket (void);
int sendPacket (unsigned int socket, unsigned int universe, unsigned int startChannel, dataSize valueType, unsigned long value);
