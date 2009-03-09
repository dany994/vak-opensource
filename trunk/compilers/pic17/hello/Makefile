CC      = cc17
AS      = as17 -O
CFLAGS  = -DMHZ=16384000
OBJ     = hello.s

all:    hello.hex

hello.hex: $(OBJ)
	$(AS) $(OBJ) -o hello.hex

clean:
	rm -f $(OBJ) *.[bi] *.core

.c.s:
	$(CC) $(CFLAGS) -S $<
