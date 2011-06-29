#
# DESCRIPTION: Verilator Example: Makefile for inside source directory
#
# This calls the object directory makefile.  That allows the objects to
# be placed in the "current directory" which simplifies the Makefile.
#
# Copyright 2003-2011 by Wilson Snyder. This program is free software; you can
# redistribute it and/or modify it under the terms of either the GNU
# Lesser General Public License Version 3 or the Perl Artistic License
# Version 2.0.
#
ALL             = obj-alu/simx obj-regfile/simx

all compile:    $(ALL)

run:            $(ALL)
		obj-alu/simx
		obj-regfile/simx

#
# alu
#
obj-alu/simx:   obj-alu/Valu.h *.h *.cpp
		[ -d obj-alu ] && $(MAKE) -C obj-alu -f ../Makefile-alu

obj-alu/Valu.h: alu.v
		verilator --cc -f verilator.options -Mdir obj-alu alu.v

#
# regfile
#
obj-regfile/simx:   obj-regfile/Vregfile.h *.h *.cpp
		[ -d obj-regfile ] && $(MAKE) -C obj-regfile -f ../Makefile-regfile

obj-regfile/Vregfile.h: regfile.v
		verilator --cc -f verilator.options -Mdir obj-regfile regfile.v

clean:
		-rm -rf obj-* *.log *.dmp *.vpd core *~
