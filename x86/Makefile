# gpSP makefile
# Gilead Kutnick - Exophase

# Global definitions

CC        = gcc
STRIP     = strip
AS        = as

PREFIX    = /usr
OBJS      = main.o cpu.o memory.o video.o input.o sound.o \
	    cpu_threaded.o gui.o x86_stub.o cheats.o zip.o
BIN       = gpsp.exe 

# Platform specific definitions 

VPATH      += ..
CFLAGS     += 
INCLUDES   = -I${PREFIX}/include `sdl-config --cflags`
ifeq ($(USE_MINIZ),1)
CFLAGS     += -DGPSP_USE_MINIZ
ZIP_LIBS   =
else
ZIP_LIBS   = -lz
endif
LIBS       = -L${PREFIX}/lib `sdl-config --libs` -mconsole ${ZIP_LIBS}

# Compilation:

.SUFFIXES: .c .S

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.o: %.S
	${AS} -o $@ $<

all:	${OBJS}
	${CC} ${OBJS} ${LIBS} -o ${BIN}  
	${STRIP} ${BIN}

clean:
	rm -f *.o ${BIN} 

