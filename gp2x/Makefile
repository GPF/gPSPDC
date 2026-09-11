# gpSP makefile
# Gilead Kutnick - Exophase
# GP2X port(ion) - Z

# Global definitions

CCPREFIX  = arm-linux-
CC        = ${CCPREFIX}gcc
STRIP     = ${CCPREFIX}strip
AS        = ${CCPREFIX}as

PREFIX    = /c/gp2x/devkitGP2X/sysroot/usr
OBJS      = main.o cpu.o memory.o video.o input.o sound.o \
	    arm_stub.o gp2x.o gui.o zip.o
ASMOBJS   = arm_stub.S
BIN       = gpsp.gpe 

# Platform specific definitions 

VPATH      += ..
CFLAGS     += -O3 -DGP2X_BUILD
# NOTE: -funroll-loops will slow down compiling considerably
# 		-fno-common was required to get games to start
CFLAGS     += -funroll-loops -fstrength-reduce -fstrict-aliasing -fexpensive-optimizations -falign-functions -fweb -frename-registers -fomit-frame-pointer -ffast-math -fno-builtin -fno-common -finline -finline-functions
#CFLAGS     += -fsigned-char -funroll-loops -fstrength-reduce -fstrict-aliasing -fexpensive-optimizations -falign-functions -fweb -frename-registers -fomit-frame-pointer -ffast-math -fno-builtin -fno-common -finline -finline-functions
CXXFLAGS   = ${CFLAGS} -fno-exceptions -fno-rtti
ASFLAGS	   = ${CFLAGS}
INCLUDES   = -I${PREFIX}/include -I${PREFIX}/include/SDL
ifeq ($(USE_MINIZ),1)
CFLAGS     += -DGPSP_USE_MINIZ
ZIP_LIBS   =
else
ZIP_LIBS   = -lz
endif
LIBS       = -L${PREFIX}/lib -lSDL -lpthread ${ZIP_LIBS} -static

# Compilation:

.SUFFIXES: .c

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ $<

all:	${OBJS}
	${CC} ${OBJS} ${LIBS} -o ${BIN}  
	${STRIP} ${BIN}

clean:
	rm -f *.o ${BIN} 
