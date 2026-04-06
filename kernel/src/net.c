#include "net.h"

void netRecievePacket(void* packet, uint32_t size){
    uint8_t* p = (uint8_t*)packet;

    uint8_t* dst_mac = p;
    uint8_t* src_mac = p + 6;
    uint16_t ether_type = (p[12] << 8) | p[13]; // big-endian to host
    void* payload = (void*)(p + 14);
    uint32_t payload_len = size - 14;

    UNUSED(dst_mac);
    UNUSED(payload_len);

    biosTermPrintf("====================\nMSG RX from %x:%x:%x:%x:%x:%x with Type %x:\n", 
        src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5], 
        ether_type);
    biosTermPrintf("->");
    biosTermPrintf(payload);
    biosTermPrintf("<-\n");

    kfree(packet);
    return;
}