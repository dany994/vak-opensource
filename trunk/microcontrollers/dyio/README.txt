Namespace 0: bcs.core
~~~~~~~~~~~~~~~~~~~~~

    _png GET() -> POST()

        Ping the device

    _nms GET(byte) -> POST(asciiz, byte)

        Get the number of namespaces.
        Get the description of namespace.


Namespace 1: bcs.rpc
~~~~~~~~~~~~~~~~~~~~
    _rpc GET(byte, byte) -> POST(byte, byte, byte, asciiz)

        Get number of methods (RPCs) and RPC value
        by namespace index and method index.

    args GET(byte, byte) -> POST(byte, byte, byte, byte[], byte, byte[])

        Get number and types of arguments for a method and it's response
        by namespace index and method index.


Namespace 2: bcs.io
~~~~~~~~~~~~~~~~~~~
    gchm GET(byte) -> POST(byte, byte)

    gacm GET() -> POST(byte[])

    gchv GET(byte) -> POST(byte, int)

    gacv GET() -> POST(int[])

    asyn GET(byte) -> POST(byte, byte)

    gchc GET() -> POST(int)

    gcml GET(byte) -> POST(byte[])

    strm GET(byte) -> POST(byte, byte[])

    strm POST(byte, byte[]) -> POST(byte, byte)

    schv POST(byte, int, int) -> POST(byte, byte)

    sacv POST(int, int[]) -> POST(int[])

    cchn CRITICAL(byte, bool, int[]) -> POST(int[])

    schv CRITICAL(byte, int[]) -> POST()

    asyn CRITICAL(byte, byte, int, int, byte) -> POST()


Namespace 3: bcs.io.setmode
~~~~~~~~~~~~~~~~~~~~~~~~~~~
    schm POST(byte, byte, byte) -> POST(byte[])

    sacm POST(byte[]) -> POST(byte[])


Namespace 4: neuronrobotics.dyio
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    _rev GET() -> POST(byte, byte, byte, byte, byte, byte)

        Get firmware revision numbers.

    _pwr GET() -> POST(byte, byte, int16, bool)

        Get external power input voltage and
        power source for right and left rails.

    _mac CRITICAL(byte, byte, byte, byte, byte, byte) -> POST(byte, byte)

    _pwr CRITICAL(byte) -> POST(byte, byte)


Namespace 5: bcs.pid
~~~~~~~~~~~~~~~~~~~~
    apid GET() -> POST(int[])

    _pid GET(byte) -> POST(byte, int)

    cpid GET(byte) -> POST(byte, byte, byte, byte, f100, f100, f100,
                           int, byte, byte, fixed, fixed, fixed)

    cpdv GET(byte) -> POST(byte, f100, f100)

    gpdc GET() -> POST(int)

    apid POST(int, int[]) -> STATUS(byte, byte)

    _pid POST(byte, int, int) -> STATUS(byte, byte)

    _vpd POST(byte, int, int) -> STATUS(byte, byte)

    rpid POST(byte, int) -> STATUS(byte, byte)

    kpid CRITICAL() -> STATUS(byte, byte)

    cpid CRITICAL(byte, byte, byte, byte, f100, f100, f100, int,
                  byte, byte, fixed, fixed, fixed) -> STATUS(byte, byte)

    cpdv CRITICAL(byte, f100, f100) -> STATUS(byte, byte)

    acal CRITICAL(byte) -> STATUS(byte, byte)


Namespace 6: bcs.pid.dypid
~~~~~~~~~~~~~~~~~~~~~~~~~~
    dpid GET(byte) -> POST(byte, byte, byte, byte, byte)

    dpid CRITICAL(byte, byte, byte, byte, byte) -> POST()


Namespace 7: bcs.safe
~~~~~~~~~~~~~~~~~~~~~
    safe GET() -> POST(byte, int16)

    safe POST(byte, int16) -> POST(byte, byte)
