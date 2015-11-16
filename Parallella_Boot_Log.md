
```
U-Boot 2012.10-00003-g792c31c (Jan 03 2014 - 12:24:08)

I2C:   ready
DRAM:  992 MiB
WARNING: Caches not enabled
MMC:   SDHCI: 0
SF: Detected N25Q128 with page size 64 KiB, total 16 MiB
In:    serial
Out:   serial
Err:   serial
Net:   zynq_gem
Hit any key to stop autoboot:  0 
Configuring PL and Booting Linux...
Device: SDHCI
Manufacturer ID: 9
OEM: 4150
Name:       
Tran Speed: 50000000
Rd Block Len: 512
SD version 2.0
High Capacity: Yes
Capacity: 7.5 GiB
Bus Width: 4-bit
reading parallella.bit.bin

4045568 bytes read
reading uImage

3391040 bytes read
reading devicetree.dtb

7471 bytes read
## Booting kernel from Legacy Image at 03000000 ...
   Image Name:   Linux-3.14.12-parallella-xilinx-
   Image Type:   ARM Linux Kernel Image (uncompressed)
   Data Size:    3390976 Bytes = 3.2 MiB
   Load Address: 00008000
   Entry Point:  00008000
   Verifying Checksum ... OK
## Flattened Device Tree blob at 02a00000
   Booting using the fdt blob at 0x02a00000
   Loading Kernel Image ... OK
OK
   Loading Device Tree to 1fffb000, end 1ffffd2e ... OK

Starting kernel ...

Uncompressing Linux... done, booting the kernel.
Booting Linux on physical CPU 0x0
Linux version 3.14.12-parallella-xilinx-g40a90c3 (esim@adapteva-dev) (gcc version 4.8.2 (Ubuntu/Linaro 4.8.2-16ubuntu4) ) #1 SMP PREEMPT Fri Jan 23 22:01:51 CET 2015
CPU: ARMv7 Processor [413fc090] revision 0 (ARMv7), cr=18c5387d
CPU: PIPT / VIPT nonaliasing data cache, VIPT aliasing instruction cache
Machine model: Parallella Gen1
bootconsole [earlycon0] enabled
cma: CMA: reserved 128 MiB at 27800000
Memory policy: Data cache writealloc
PERCPU: Embedded 8 pages/cpu @e6fe0000 s9024 r8192 d15552 u32768
Built 1 zonelists in Zone order, mobility grouping on.  Total pages: 252432
Kernel command line: console=ttyPS0,115200 root=/dev/mmcblk0p2 rw earlyprintk rootfstype=ext4 rootwait
PID hash table entries: 4096 (order: 2, 16384 bytes)
Dentry cache hash table entries: 131072 (order: 7, 524288 bytes)
Inode-cache hash table entries: 65536 (order: 6, 262144 bytes)
Memory: 868820K/1015808K available (4609K kernel code, 217K rwdata, 1544K rodata, 176K init, 209K bss, 146988K reserved, 237568K highmem)
Virtual kernel memory layout:
    vector  : 0xffff0000 - 0xffff1000   (   4 kB)
    fixmap  : 0xfff00000 - 0xfffe0000   ( 896 kB)
    vmalloc : 0xf0000000 - 0xff000000   ( 240 MB)
    lowmem  : 0xc0000000 - 0xef800000   ( 760 MB)
    pkmap   : 0xbfe00000 - 0xc0000000   (   2 MB)
    modules : 0xbf000000 - 0xbfe00000   (  14 MB)
      .text : 0xc0008000 - 0xc060a7f8   (6154 kB)
      .init : 0xc060b000 - 0xc0637340   ( 177 kB)
      .data : 0xc0638000 - 0xc066e5e0   ( 218 kB)
       .bss : 0xc066e5ec - 0xc06a29f4   ( 210 kB)
Preemptible hierarchical RCU implementation.
	RCU restricting CPUs from NR_CPUS=4 to nr_cpu_ids=2.
RCU: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=2
NR_IRQS:16 nr_irqs:16 16
slcr mapped to f0004000
zynq_clock_init: clkc starts at f0004100
Zynq clock init
sched_clock: 16 bits at 54kHz, resolution 18432ns, wraps every 1207951633ns
timer #0 at f0006000, irq=43
Console: colour dummy device 80x30
Calibrating delay loop... 1332.01 BogoMIPS (lpj=6660096)
pid_max: default: 32768 minimum: 301
Mount-cache hash table entries: 2048 (order: 1, 8192 bytes)
Mountpoint-cache hash table entries: 2048 (order: 1, 8192 bytes)
CPU: Testing write buffer coherency: ok
CPU0: thread -1, cpu 0, socket 0, mpidr 80000000
Setting up static identity map for 0x45e140 - 0x45e198
L310 cache controller enabled
l2x0: 8 ways, CACHE_ID 0x410000c8, AUX_CTRL 0x72760000, Cache size: 512 kB
CPU1: Booted secondary processor
CPU1: thread -1, cpu 1, socket 0, mpidr 80000001
Brought up 2 CPUs
SMP: Total of 2 processors activated.
CPU: All CPU(s) started in SVC mode.
devtmpfs: initialized
VFP support v0.3: implementor 41 architecture 3 part 30 variant 9 rev 4
regulator-dummy: no parameters
NET: Registered protocol family 16
DMA: preallocated 256 KiB pool for atomic coherent allocations
cpuidle: using governor ladder
cpuidle: using governor menu
syscon f8000000.slcr: regmap [mem 0xf8000000-0xf8000fff] registered
hw-breakpoint: found 5 (+1 reserved) breakpoint and 1 watchpoint registers.
hw-breakpoint: maximum watchpoint size is 4 bytes.
bio: create slab <bio-0> at 0
usbcore: registered new interface driver usbfs
usbcore: registered new interface driver hub
usbcore: registered new device driver usb
media: Linux media interface: v0.10
Linux video capture interface: v2.00
EDAC MC: Ver: 3.0.0
Advanced Linux Sound Architecture Driver Initialized.
Bluetooth: Core ver 2.18
NET: Registered protocol family 31
Bluetooth: HCI device and connection manager initialized
Bluetooth: HCI socket layer initialized
Bluetooth: L2CAP socket layer initialized
Bluetooth: SCO socket layer initialized
Switched to clocksource ttc_clocksource
NET: Registered protocol family 2
TCP established hash table entries: 8192 (order: 3, 32768 bytes)
TCP bind hash table entries: 8192 (order: 4, 65536 bytes)
TCP: Hash tables configured (established 8192 bind 8192)
TCP: reno registered
UDP hash table entries: 512 (order: 2, 16384 bytes)
UDP-Lite hash table entries: 512 (order: 2, 16384 bytes)
NET: Registered protocol family 1
RPC: Registered named UNIX socket transport module.
RPC: Registered udp transport module.
RPC: Registered tcp transport module.
RPC: Registered tcp NFSv4.1 backchannel transport module.
hw perfevents: enabled with ARMv7 Cortex-A9 PMU driver, 7 counters available
futex hash table entries: 512 (order: 3, 32768 bytes)
bounce pool size: 64 pages
msgmni has been set to 1488
Block layer SCSI generic (bsg) driver version 0.4 loaded (major 251)
io scheduler noop registered
io scheduler deadline registered
io scheduler cfq registered (default)
dma-pl330 f8003000.ps7-dma: Loaded driver for PL330 DMAC-2364208
dma-pl330 f8003000.ps7-dma: 	DBUFF-128x8bytes Num_Chans-8 Num_Peri-4 Num_Events-16
xuartps e0001000.uart: clock name 'aper_clk' is deprecated.
xuartps e0001000.uart: clock name 'ref_clk' is deprecated.
console [ttyPS0] enabledMMIO 0xe0001000 (irq = 82, base_baud = 3124999) is a xuartps
console [ttyPS0] enabled
bootconsole [earlycon0] disabled
bootconsole [earlycon0] disabled
xdevcfg f8007000.devcfg: ioremap 0xf8007000 to f0016000
epiphany_init() - shared memory: bus 0x8f000000, phy 0x3f000000, kvirt 0xf1000000
[drm] Initialized drm 1.1.0 20060810
brd: module loaded
loop: module loaded
zram: Created 1 device(s) ...
libphy: XEMACPS mii bus: probed
xemacps e000b000.eth: pdev->id -1, baseaddr 0xe000b000, irq 54
usbcore: registered new interface driver asix
usbcore: registered new interface driver ax88179_178a
usbcore: registered new interface driver cdc_ether
usbcore: registered new interface driver net1080
usbcore: registered new interface driver cdc_subset
usbcore: registered new interface driver zaurus
usbcore: registered new interface driver cdc_ncm
aoe: cannot create debugfs directory
aoe: AoE v85 initialised.
ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
zynq-dr e0002000.usb: Unable to init USB phy, missing?
usbcore: registered new interface driver usbserial
mousedev: PS/2 mouse device common for all mice
i2c /dev entries driver
cdns-i2c e0004000.i2c: 100 kHz mmio e0004000 irq 57
cpufreq_cpu0: failed to get cpu0 regulator: -19
cpufreq_cpu0: failed to get cpu0 clock: -2
cpufreq-cpu0: probe of cpufreq-cpu0.0 failed with error -2
Xilinx Zynq CpuIdle Driver started
sdhci: Secure Digital Host Controller Interface driver
sdhci: Copyright(c) Pierre Ossman
sdhci-pltfm: SDHCI platform and OF driver helper
mmc0: no vqmmc regulator found
mmc0: no vmmc regulator found
mmc0: Invalid maximum block size, assuming 512 bytes
mmc0: SDHCI controller on e0101000.sdhci [e0101000.sdhci] using ADMA
usbcore: registered new interface driver usbhid
usbhid: USB HID core driver
usbcore: registered new interface driver snd-usb-audio
TCP: cubic registered
NET: Registered protocol family 10
sit: IPv6 over IPv4 tunneling driver
NET: Registered protocol family 17
Bluetooth: RFCOMM TTY layer initialized
Bluetooth: RFCOMM socket layer initialized
Bluetooth: RFCOMM ver 1.11
Bluetooth: BNEP (Ethernet Emulation) ver 1.3
Bluetooth: BNEP filters: protocol multicast
mmc0: new high speed SDHC card at address b368
Bluetooth: BNEP socket layer initialized
Bluetooth: HIDP (Human Interface Emulation) ver 1.2
mmcblk0: mmc0:b368       7.45 GiB 
Bluetooth: HIDP socket layer initialized
zynq_pm_ioremap: no compatible node found for 'xlnx,zynq-ddrc-1.0'
 mmcblk0: p1 p2
zynq_pm_late_init: Unable to map DDRC IO memory.
zynq_pm_remap_ocm: no compatible node found for 'xlnx,zynq-ocmc-1.0'
zynq_pm_late_init: Unable to map OCM.
Registering SWP/SWPB emulation handler
regulator-dummy: disabling
ALSA device list:
  No soundcards found.
EXT4-fs (mmcblk0p2): mounted filesystem with ordered data mode. Opts: (null)
VFS: Mounted root (ext4 filesystem) on device 179:2.
devtmpfs: mounted
Freeing unused kernel memory: 176K (c060b000 - c0637000)
Mount failed for selinuxfs on /sys/fs/selinux:  No such file or directory
 * Starting Mount filesystems on bootocess ended, respawning with status [ OK ]
 * Starting Signal sysvinit that the rootfs is mounted                   [ OK ]
 * Starting Populate /dev filesystem                                     [ OK ]
 * Stopping Populate /dev filesystem                                     [ OK ]
 * Starting Clean /tmp directory                                         [ OK ]
 * Starting Populate and link to /run filesystem                         [ OK ]
 * Stopping Populate and link to /run filesystem                         [ OK ]
 * Stopping Clean /tmp directory                                         [ OK ]
 * Stopping Track if upstart is running in a container                   [ OK ]
 * Starting Signal sysvinit that virtual filesystems are mounted         [ OK ]
 * Starting Signal sysvinit that virtual filesystems are mounted         [ OK ]
 * Starting Bridge udev events into upstart                              [ OK ]
 * Starting Signal sysvinit that remote filesystems are mounted          [ OK ]
 * Starting device node and kernel event manager                         [ OK ]
 * Starting Signal sysvinit that local filesystems are mounted           [ OK ]
 * Stopping Mount filesystems on boot                                    [ OK ]
 * Starting flush early job output to logs                               [ OK ]
 * Starting load modules from /etc/modules                               [ OK ]
 * Starting cold plug devices                                            [ OK ]
 * Starting log initial device creation                                  [ OK ]
 * Stopping flush early job output to logs                               [ OK ]
 * Stopping load modules from /etc/modules                               [ OK ]
 * Starting D-Bus system message bus                                     [ OK ]
 * Starting SystemD login management service                             [ OK ]
 * Starting system logging daemon                                        [ OK ]
 * Starting configure network device security                            [ OK ]
 * Starting mDNS/DNS-SD daemon                                           [ OK ]
 * Starting Reload cups, upon starting avahi-daemon to make sure remote q[ OK ]are populated
 * Starting Reload cups, upon starting avahi-daemon to make sure remote q[fail]are populated
 * Starting configure network device security                            [ OK ]
 * Starting configure network device security                            [ OK ]
 * Stopping cold plug devices                                            [ OK ]
 * Stopping log initial device creation                                  [ OK ]
 * Starting configure network device                                     [ OK ]
 * Starting configure network device security                            [ OK ]
 * Starting save udev log and update rules                               [ OK ]
 * Starting set console font                                             [ OK ]
 * Stopping set console font                                             [ OK ]
 * Starting userspace bootsplash                                         [ OK ]
 * Starting Mount network filesystems                                    [ OK ]
 * Starting Failsafe Boot Delay                                          [ OK ]
 * Stopping save udev log and update rules                               [ OK ]
 * Stopping userspace bootsplash                                         [ OK ]
 * Starting Send an event to indicate plymouth is up                     [ OK ]
 * Stopping Send an event to indicate plymouth is up                     [ OK ]
 * Stopping Mount network filesystems                                    [ OK ]
 * Starting Bridge socket events into upstart                            [ OK ]
 * Starting Bridge file events into upstart                              [ OK ]
 * Starting configure virtual network devices                            [ OK ]
 * Starting configure network device                                     [ OK ]
 * Starting Mount network filesystems                                    [ OK ]
 * Stopping Mount network filesystems                                    [ OK ]
 * Starting configure network device                                     [ OK ]
 * Stopping Failsafe Boot Delay                                          [ OK ]
 * Starting System V initialisation compatibility                        [ OK ]
Starting fake hwclock: loading system time.
Sat Jun  6 03:30:29 UTC 2015
 * Setting up X socket directories...                                    [ OK ] 
 * Stopping System V initialisation compatibility                        [ OK ]
 * Starting System V runlevel compatibility                              [ OK ]
 * Starting Parallella Thermal Watchdog                                  [ OK ]
 * Starting save kernel messages                                         [ OK ]
 * Starting OpenSSH server                                               [ OK ]
 * Starting regular background program processing daemon                 [ OK ]
 * Starting deferred execution scheduler                                 [ OK ]
 * Stopping save kernel messages                                         [ OK ]
 * Starting NTP server ntpd                                              [ OK ] 
 * Stopping System V runlevel compatibility                              [ OK ]

Last login: Fri Jan 30 01:01:08 UTC 2015 on ttyPS0
Welcome to Ubuntu 14.04.2 LTS (GNU/Linux 3.14.12-parallella-xilinx-g40a90c3 armv7l)

 * Documentation:  https://help.ubuntu.com/
root@parallella:~# _
```