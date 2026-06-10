# gpSP makefile
# Gilead Kutnick - Exophase

# Global definitions

PSPSDK          = ${shell psp-config --pspsdk-path}
PREFIX          = ${shell psp-config --psp-prefix}

OBJS            = main.o cpu.o video.o memory.o sound.o input.o \
 	          cpu_threaded.o gui.o zip.o mips_stub.o 

TARGET          = gpSP

VPATH           += ..
CFLAGS          += -O3 -DPSP_BUILD -G0
CFLAGS          += ${shell ${PREFIX}/bin/sdl-config --cflags}
CXXFLAGS        = ${CFLAGS} -fno-exceptions -fno-rtti
ASFLAGS         = ${CFLAGS}
PSP_EBOOT_TITLE = gpSP
EXTRA_TARGETS   = EBOOT.PBP

ifeq ($(USE_MINIZ),1)
CFLAGS          += -DGPSP_USE_MINIZ
ZIP_LIBS        =
else
ZIP_LIBS        = -lz
endif

LIBS            += ${shell ${PREFIX}/bin/sdl-config --libs} -lpsppower \
		  ${ZIP_LIBS}

include ${PSPSDK}/lib/build.mak

