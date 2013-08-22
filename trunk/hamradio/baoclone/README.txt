
Baoclone is a utility for programming Baofeng radios via a serial or USB
programming cable.  Supported radios:

 * Baofeng UV-5R
 * Baofeng UV-B5 (todo)
 * Baofeng BF-888S (todo)


Usage:
    baoclone [-v] port
                                Save device binary image to file 'device.img',
                                and text configuration to 'device.cfg'.

    baoclone -w [-v] port file.img
                                Write image to device.

    baoclone -c [-v] port file.cfg
                                Configure device from text file.

    baoclone file.img
                                Show configuration from image file.

Options:
    -v                          Verbose mode.
