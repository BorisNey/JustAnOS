from scapy.all import *
import time

# VMWare
dst_mac = "00:0C:29:14:4B:B4"

# Qemu
#dst_mac = "52:54:00:12:34:56"

for x in range(6):
    string = b"Hello from Host " + bytes(str(x), 'ascii') + b" !\0"
    frame = Ether(dst=dst_mac, src=get_if_hwaddr(conf.iface), type=0x1234) / string # type: ignore
    sendp(frame, iface="VMware Network Adapter VMnet1")
    time.sleep(1)
