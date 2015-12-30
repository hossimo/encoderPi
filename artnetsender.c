#include "artnetsender.h"

//globals
ArtnetDmxHeader artnetPacket;
struct sockaddr_in address;
int seq = 0;
time_t lastConfTime = 0;
Conf lastConf;

int buildSocket (){
    
    strcpy(artnetPacket.ID, "Art-Net");
    artnetPacket.OpCode = 0x5000;
    artnetPacket.version = htons(0x000e);
    artnetPacket.length = htons(0x200);
    artnetPacket.physical = 1;
    memset(artnetPacket.data, 0, sizeof(artnetPacket.data));
    
    int localSocket = -1;
    
    if (localSocket < 0)
        localSocket = socket(AF_INET, SOCK_DGRAM, 0);
    
    address.sin_addr.s_addr = INADDR_ANY;
    
    int bindResult = bind(localSocket, (const struct sockaddr*) &address, sizeof(address));
    if (bindResult < 0) {
        perror("failed to bind:");
        return -1;
    }
    
    int on = 1;
    if(setsockopt(localSocket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) != 0){
        perror("error setting SO_BROADCAST:");
    }
    
    // who should we send this data too?
    inet_pton(AF_INET, "255.255.255.255", &address.sin_addr);
    
    return localSocket;
}

size_t sendPacket (unsigned int socket, /* Conf *conf, */ unsigned long value){
    Conf localConf = readPrefs();
    //TODO: mutex access of *conf
    //TODO: keep a local conf
    //TODO: check if conf has changes
    //TODO: Get Conf yourself
    
    // setup Packet
    // who should we send this data too?
    if (!localConf.sendToAddress)
        return 0;
    
    if (lastConfTime != localConf.lastTime) {
        lastConf = localConf;
        memset(artnetPacket.data, 0,  sizeof(artnetPacket.data));   // zero out DMX data to avoid stale values
    }
    
    inet_pton(AF_INET, localConf.sendToAddress, &address.sin_addr);
    
    //tmp local variables
    unsigned int startChannel = localConf.startAddress < 1 ? 0 : localConf.startAddress - 1;
    
    artnetPacket.subUni = localConf.universe;
    artnetPacket.seq = seq++ % 512;
    address.sin_port = htons(0x1936);
    address.sin_family = AF_INET;
    
    switch (localConf.dataBits) {
        case k8Bit:
            artnetPacket.data[startChannel] = value & 0xff;
            break;
        case k16Bit:
            artnetPacket.data[startChannel+1] = value & 0xff;
            artnetPacket.data[startChannel]   = value >> 8 & 0xff;
            break;
        case k24Bit:
            artnetPacket.data[startChannel+2] = value & 0xff;
            artnetPacket.data[startChannel+1] = value >> 8 & 0xff;
            artnetPacket.data[startChannel]   = value >> 16 & 0xff;
            break;
        case k32Bit:
            artnetPacket.data[startChannel+3] = value & 0xff;
            artnetPacket.data[startChannel+2] = value >> 8 & 0xff;
            artnetPacket.data[startChannel+1] = value >> 16 & 0xff;
            artnetPacket.data[startChannel]   = value >> 24 & 0xff;
            break;
        default:
            artnetPacket.data[startChannel] = value & 0xff;
            break;

    }
    
    ssize_t sent = sendto(
                          socket,                         //what socket
                          (void*)&artnetPacket,           //data
                          sizeof(artnetPacket),           //sizeof(data)
                          0,                              //flags
                          (struct sockaddr*) &address,    //to address
                          sizeof(struct sockaddr_in)      //sizeof(address)
                          );
    return sent;
}
