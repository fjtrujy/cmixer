  
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = demo.elf

EE_OBJS = src/main.o ../../src/cmixer.o
EE_OBJS += libsd_irx.o audsrv_irx.o
EE_CFLAGS = -I../../src/
 
EE_LDFLAGS = -lpatches -laudsrv

all: $(EE_BIN)

clean:
	rm -rf $(EE_OBJS) $(EE_BIN)

libsd_irx.c:
	$(PS2SDK)/bin/bin2c $(PS2SDK)/iop/irx/libsd.irx $@ libsd_irx

audsrv_irx.c:
	$(PS2SDK)/bin/bin2c $(PS2SDK)/iop/irx/audsrv.irx $@ audsrv_irx

# Include makefiles
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal