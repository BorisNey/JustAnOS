#include "i8254x.h"

/*
IMPROVEMENTS:
    
*/

//=====================================================

#define REG_CTRL 0x0        // Device Control
#define REG_STATUS 0x8      // Device Status
#define REG_EECD 0x10       // EEPROM/Flash Control/Data
#define REG_EERD 0x14       // EEPROM Read
#define REG_ICR 0xC0        // Interrupt Cause Read
#define REG_IMS 0xD0        // Interrupt Mask

#define REG_RCTL 0x100      // Receive Control
#define REG_RDBAL 0x2800    // Receive Desc Base Low
#define REG_RDBAH 0x2804    // Receive Desc Base High
#define REG_RDLEN 0x2808    // Receive Desc Lenght
#define REG_RDH 0x2810      // Receive Desc Head
#define REG_RDT 0x2818      // Receive Desc Tail

#define REG_TCTL 0x400      // Transmit Control
#define REG_TIPG 0x0410     // TX inter-packet gap
#define REG_TDBAL 0x3800    // Transmit Desc Base Low
#define REG_TDBAH 0x3804    // Transmit Desc Base High
#define REG_TDLEN 0x3808    // Transmit Desc Lenght
#define REG_TDH 0x3810      // Transmit Desc Head
#define REG_TDT 0x3818      // Transmit Desc Tail

#define REG_RAL 0x5400      // Receive Addr Low
#define REG_RAH 0x5404      // Receive Addr High

//==========================================================

#define EECD_PRES (1 << 8)
#define EERD_START (1 << 0)
#define EERD_DONE (1 << 4)

//=============================================================

#define CTRL_RST (1 << 26)  // Reset the Controller
#define CTRL_ASDE (1 << 5)  // Auto-Speed Detection Enable
#define CTRL_SLU (1 << 6)   // Set Link Up

//============================================================

#define NUM_OF_TX_DESCRIPTORS 8
#define SIZE_OF_TX_DESCRIPTOR_BUFFER 4096

#define TCTL_EN (1 << 1)        // Transmit Enable
#define TCTL_PSP (1 << 3)       // Pad Short Packets
#define TCTL_CT (15 << 4)       // Collision Threshold
#define TCTL_COLD (64 << 12)    // Collision Distance

#define TIPG_IPGT 10            // IPG Transmit Time
#define TIPG_IPGR1 (10 << 10)   // PG Receive Time 1
#define TIPG_IPGR2 (10 << 20)   // PG Receive Time 2

//=============================================================

#define NUM_OF_RX_DESCRIPTORS 32
#define SIZE_OF_RX_DESCRIPTOR_BUFFER 4096

#define RCTL_EN (1 << 1)        // Receiver Enable
#define RCTL_LPE (1 << 5)       // Long Packet Reception Enable
#define RCTL_BAM (1 << 15)      // Broadcast Accept Mode.
#define RCTL_BSEX (1 << 25)     // Buffer Size Extension
#define RCTL_BSIZE (0b11 << 16) // Receive Buffer Size

//=============================================================

#define IMS_LSC (1 << 2)    // Link Status Change
#define IMS_RXO (1 << 6)    // Receiver FIFO Overrun
#define IMS_RXT0 (1 << 7)   // Receiver Timer Interrupt

//=============================================================

#define TX_CMD_EOP (1 << 0)     // End Of Packet
#define TX_CMD_IFCS (1 << 1)    // Insertion of the FCS/CRC

//==================================================================

#define RX_STATUS_DD (1 << 0)   // Descriptor Done
#define RX_STATUS_EOP (1 << 1)  // End Of Packet

//==============================================================

static uint32_t mmio_addr = 0;  // MMIO Address to Ethernet Controller
static uint8_t mac_adr[6];      // MAC Address

static tx_descriptor_t* tx_ring;    // Array of 8 Transmit Descriptors
static rx_descriptor_t* rx_ring;    // Array of 32 Recieve Descriptors

static uint8_t rx_next = 0;

//==============================================================



static void writeMMIO(uint16_t offset, uint32_t value){
    *(uint32_t *)(mmio_addr + offset) = value;
}

static uint32_t readMMIO(uint16_t offset){
    return *(uint32_t *)(mmio_addr + offset);
}

static uint16_t readEEPROM(uint8_t addr) {
    uint16_t data;
    uint32_t readEEPROM;

    if((readMMIO(REG_EECD) & EECD_PRES) == 0) {
        biosTermPrintf("ERR: EEPROM present bit is not set\n");
        return 0;
    }

    readEEPROM = ((uint32_t)addr & 0xff) << 8 | EERD_START;
    writeMMIO(REG_EERD, readEEPROM);

    // Wait until the read is finished - then the DONE bit is cleared
    while((readMMIO(REG_EERD) & EERD_DONE) == 0);

    // Read the data
    data = (uint16_t)(readMMIO(REG_EERD) >> 16);

    // Tell EEPROM to stop reading
    readEEPROM = readMMIO(REG_EERD) & ~(uint32_t)EERD_START;
    writeMMIO(REG_EERD, readEEPROM);

    return data;
}

//===================================================================

/*
Resets the NIC internally and reads the MAC Addresss
*/
static void resetNIC(){    
    // Set the reset bit
    uint32_t device_control = readMMIO(REG_CTRL) | CTRL_RST;
    writeMMIO(REG_CTRL, device_control);
    
    // wait for it to reset
    while((readMMIO(REG_CTRL) & CTRL_RST) != 0);
    
    // Enable Auto Speed Detection
    device_control = readMMIO(REG_CTRL) | CTRL_ASDE | CTRL_SLU;
    writeMMIO(REG_CTRL, device_control);
    
    // Read the MAC address from the EEPROM
    uint16_t b0 = readEEPROM(0);
    uint16_t b1 = readEEPROM(1);
    uint16_t b2 = readEEPROM(2);
    
    mac_adr[0] = b0 & 0xFF;
    mac_adr[1] = b0 >> 8;
    mac_adr[2] = b1 & 0xFF;
    mac_adr[3] = b1 >> 8;
    mac_adr[4] = b2 & 0xFF;
    mac_adr[5] = b2 >> 8;
    
    // Write the MAC address to RAL/RAH 0.
    uint32_t recieve_adr_low = ((uint32_t)b1 << 16) | b0;
    uint32_t recieve_adr_high = b2;
    writeMMIO(REG_RAL, recieve_adr_low);
    writeMMIO(REG_RAH, recieve_adr_high);

    return;
}

/*
Sets up a Transmit Descriptor Ring.
Allocates the descriptors and buffers in RAM and tells the NIC where the descriptors are
*/
static void setupTXRing(){
    uint32_t tx_descs_phys = allocPageFrame();
    uint32_t tx_descs_virt = TX_DESCRIPTORS;
    mapAddr(tx_descs_virt, tx_descs_phys, PAGE_FLAG_WRITE);
    tx_ring = (tx_descriptor_t*)tx_descs_virt;

    for (int i = 0; i < NUM_OF_TX_DESCRIPTORS; i++){
        tx_ring[i].buffer_address_low = allocPageFrame();
        tx_ring[i].buffer_address_high = 0;
    }
    
    writeMMIO(REG_TDBAL, tx_descs_phys);
    writeMMIO(REG_TDBAH, 0);
    writeMMIO(REG_TDLEN, NUM_OF_TX_DESCRIPTORS * sizeof(tx_descriptor_t));
    writeMMIO(REG_TDH, 0);
    writeMMIO(REG_TDT, 0);
    
    uint32_t tctl = 0x0 | TCTL_EN | TCTL_PSP | TCTL_CT | TCTL_COLD;
    writeMMIO(REG_TCTL, tctl);

    uint32_t tipg = TIPG_IPGT | TIPG_IPGR1 | TIPG_IPGR2;
    writeMMIO(REG_TIPG, tipg);
    
    return;
}

/*
Sets up a Recieve Descriptor Ring.
Allocates the descriptors and buffers in RAM and tells the NIC where the descriptors are
*/
static void setupRXRing(){
    uint32_t rx_descs_phys = allocPageFrame();
    uint32_t rx_descs_virt = RX_DESCRIPTORS;
    mapAddr(rx_descs_virt, rx_descs_phys, PAGE_FLAG_WRITE);
    rx_ring = (rx_descriptor_t*)rx_descs_virt;

    for (int i = 0; i < NUM_OF_RX_DESCRIPTORS; i++){
        rx_ring[i].buffer_address_low = allocPageFrame();
        rx_ring[i].buffer_address_high = 0;
    }
    
    writeMMIO(REG_RDBAL, rx_descs_phys);
    writeMMIO(REG_RDBAH, 0);
    writeMMIO(REG_RDLEN, NUM_OF_RX_DESCRIPTORS * sizeof(rx_descriptor_t));
    writeMMIO(REG_RDH, 0);
    writeMMIO(REG_RDT, 0);

    // BSIZE = 0b11 and BSEX = 1 -> 4096 buffers
    uint32_t rctl = RCTL_EN | RCTL_LPE | RCTL_BAM | RCTL_BSEX | RCTL_BSIZE;
    writeMMIO(REG_RCTL, rctl);
    
    return;
}

/*
Memcopys data to a the Tail of the Transmit Descriptor Ring and adjusts the Tail descriptor
*/
static void sendData(void* data, uint32_t size, uint8_t EOP){
    uint32_t tail = readMMIO(REG_TDT);
    tx_descriptor_t* tx = tx_ring + tail; // Get the descriptor the tail is pointing at (next available descriptor)

    memcpy((void*)tx->buffer_address_low, data, size); // Copy the data to the previously allocated buffer

    tx->length = size; // Set the length of the descriptor

    // If its the last one, set EOP
    if (EOP){
        tx->command |= TX_CMD_EOP | TX_CMD_IFCS; 
    }
    tail = (tail + 1) % NUM_OF_TX_DESCRIPTORS;
    writeMMIO(REG_TDT, tail); // Increment and write the tail

    return;
}

/*
Splits up the data in Sizes of the Buffer and calls sendData to write it into the buffer
*/
size_t i8254xSend(void* data, size_t length){
    size_t sent = 0;
    // split the data into chunks and send them
    for (; sent < length;){
        int to_send = min((int)(length - sent), SIZE_OF_TX_DESCRIPTOR_BUFFER);
        sendData((void*)((uint32_t)data + sent), to_send, (size_t)to_send == (length - sent));
        sent += to_send;
    }
    return sent;
}

/*

*/
static void receivePackets(){
    uint32_t idx = rx_next;

    void* buffer = NULL; // use this to store the buffer.
    uint32_t buffer_len = 0; 

    while (rx_ring[idx].status & RX_STATUS_DD) {
        // This descriptor has been filled
        
        uint8_t eop = rx_ring[idx].status & RX_STATUS_EOP;
        uint32_t len = rx_ring[idx].length;
        void* data = (void*)rx_ring[idx].buffer_address_low;
        
        // Handle multiple-descriptor packets
        if (buffer == NULL){ // This is the first descriptor of the packet
            buffer = kmalloc(len); // use your kernel's heap allocator
            buffer_len = len;
            memcpy(buffer, data, len);
        }
        else{
            // Its the next part of the packet, add it to the packet 
            void* new_buffer = kmalloc(buffer_len + len); // allocate a bigger buffer
            memcpy(new_buffer, buffer, buffer_len); // copy the previous data
            kfree(buffer); // free the old buffer
    
            // copy the new data
            memcpy((void*)((uint32_t)new_buffer + buffer_len), data, len);
            
            // Set the new buffer into the variables
            buffer_len += len;
            buffer = new_buffer;
        }
    
        // Set status to 0 (To give ownership back to the controller)
        rx_ring[idx].status = 0; 

        idx = (idx + 1) % NUM_OF_RX_DESCRIPTORS;

        if (eop) {
            // This is the last descriptor of the packet
            // Forward the packet to your network stack
            netRecievePacket(buffer, buffer_len);
            buffer = NULL;
            buffer_len = 0;
        }
    }

    // Give the controller more free descriptors by updating RDT
    uint32_t tail = (idx == 0) ? NUM_OF_RX_DESCRIPTORS - 1 : idx - 1;
    writeMMIO(REG_RDT, tail);

    rx_next = idx;

    return;
}

/*
Enables Interrupt Requests
*/
static void enableIRQ(){
    uint32_t ims = IMS_RXT0 | IMS_RXO | IMS_LSC;
    writeMMIO(REG_IMS, ims);

    return;
}

/*
Handler funcition for installHandler
*/
void i8254xHandler(intr_regs_t* regs){
    UNUSED(regs);

    #define IMS_LSC (1 << 2)    // Link Status Change
    #define IMS_RXO (1 << 6)    // Receiver FIFO Overrun
    #define STATUS_LU (1 << 1)  // Link Up Indication


    uint32_t cause = readMMIO(REG_ICR); // Cleared uppon read

    if (cause & IMS_RXT0) { // Packets received
        receivePackets();  // Call the function responsible for receiving
                            // packets and sending them to the network stack
    }

    if (cause & IMS_LSC){ // link status change
        // Read the status register and check the LU bit to get the link status
        if (readMMIO(REG_STATUS) & STATUS_LU) {
            biosTermPrintf("DBG: NIC: Link change detected: Link up!\n");
        }else{
            biosTermPrintf("DBG: NIC: Link change detected: Link down!\n");
        }
    }

    return;
}

void initI8254x(pci_dev_t* eth_adapter, int irq_id){
    // PCI Memory Space enable and Bus Master enable (needed for DMA)
    uint32_t cmd = pciReadDWord(eth_adapter->bus, eth_adapter->slot, eth_adapter->func, 0x04) | PCI_MEMORY_SPACE_ENABLE | PCI_BUS_MASTER_ENABLE;
    pciWriteDWord(eth_adapter->bus, eth_adapter->slot, eth_adapter->func, 0x04, cmd);

    // Get BAR0 -> Start of the MMIO Register Space
    uint32_t bar0_phys = pciReadDWord(eth_adapter->bus, eth_adapter->slot, eth_adapter->func, 0x10) & 0xFFFFFFF0;
    // Virtual Address
    mmio_addr = e1000_MMIO_REG_SPACE_ADDR;
    // The e1000 MMIO Register Space is 128KB (so map out 32 pages)
    for (uint32_t i = 0; i < 32; i++) {
        mapAddr(mmio_addr + i * PAGE_SIZE,
                bar0_phys + i * PAGE_SIZE,
                PAGE_FLAG_WRITE);
    }

    resetNIC();
    setupTXRing();
    setupRXRing();
    enableIRQ();
    installIrqHandler(irq_id, i8254xHandler);
    
    biosTermPrintf("DBG: I8254x init success, MAC: %x:%x:%x:%x:%x:%x\n", mac_adr[0], mac_adr[1], mac_adr[2], mac_adr[3], mac_adr[4], mac_adr[5]);

    return;
}
