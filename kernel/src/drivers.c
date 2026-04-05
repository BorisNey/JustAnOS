#include "drivers.h"

static void initEth(int irq_id){
    pci_dev_t* eth_adapter = pciFindDevType(PCI_CLASS_NET_CTL, PCI_SUBCLASS_ETH_CTL, 0xFF);
    if (eth_adapter == NULL){
        biosTermPrintf("ERR: No Ethernet Controller found\n");
        return;
    }

    switch (eth_adapter->vendor_id){
        case INTEL_VENDOR_ID:
            switch (eth_adapter->device_id){
                case i8254x_DEVICE_ID:
                    initI8254x(eth_adapter, irq_id);
                    break;
                default:
                    biosTermPrintf("ERR: No Supported Ethernet Driver for Intel\n");
                    break;
            }
            break;
        case AMD_VENDOR_ID:
            biosTermPrintf("ERR: No Supported Ethernet Driver for AMD\n");
            break;
        default:
            biosTermPrintf("ERR: No Ethernet Driver found\n");
            break;
    }

    return;
}

void initDrivers(){
    initKeyboard(1);
    initEth(2);

    return;
}
