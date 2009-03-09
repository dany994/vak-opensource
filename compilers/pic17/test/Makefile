CC      = cc17
AS      = as17
CFLAGS  = -DMHZ=16384000
OBJ     = startup.s
ALL     = chardivs.hex chardivc.hex intshift.hex intarith.hex shortari.hex charcmp.hex shortcmp.hex #longcmp.hex longarit.hex

all:    $(ALL)

clean:
	rm -f *.hex *.[bis] *.core *.lst

charcmp.hex: charcmp.s $(OBJ)
	$(AS) $(OBJ) charcmp.s -o charcmp.hex

chardivs.hex: chardivs.s $(OBJ)
	$(AS) $(OBJ) chardivs.s -o chardivs.hex

chardivc.hex: chardivc.s $(OBJ)
	$(AS) $(OBJ) chardivc.s -o chardivc.hex

longcmp.hex: longcmp.s $(OBJ)
	$(AS) $(OBJ) longcmp.s -o longcmp.hex

shortcmp.hex: shortcmp.s $(OBJ)
	$(AS) $(OBJ) shortcmp.s -o shortcmp.hex

intarith.hex: intarith.s $(OBJ)
	$(AS) $(OBJ) intarith.s -o intarith.hex

shortari.hex: shortari.s $(OBJ)
	$(AS) $(OBJ) shortari.s -o shortari.hex

longarit.hex: longarit.s $(OBJ)
	$(AS) $(OBJ) longarit.s -o longarit.hex

intshift.hex: intshift.s $(OBJ)
	$(AS) $(OBJ) intshift.s -o intshift.hex

.SUFFIXES:: .i

.c.s:
	$(CC) $(CFLAGS) -c $<

.c.i:
	$(CPP) -I/usr/local/lib/pic17/include $(CFLAGS) $< > $*.i
