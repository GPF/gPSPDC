# gpSP
# DC porting by Troy Davis (GPF)

TARGET = gdC.elf
VPATH += .. .

OBJS = sh4_stub.o sh4_helpers.o main.o cpu_threaded.o cpu.o memory.o video.o \
       input.o sound.o gui.o zip.o cheats.o romdisk.o

KOS_ROMDISK_DIR = romdisk

include $(KOS_BASE)/Makefile.rules

KOS_CFLAGS += -D_arch_dreamcast=1 -DGPSP_DC_BLIT_MEMCPY  \
    -I. \
	-I.. \
    -I$(KOS_BASE)/addons/include \
    -I$(KOS_BASE)/addons/include/SDL \
    -I$(KOS_BASE)/../kos-ports/include/zlib

all: $(TARGET)

clean:
	-rm -f $(TARGET) $(OBJS) romdisk.img romdisk.o

$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS) -L$(KOS_BASE)/lib/dreamcast -L$(KOS_BASE)/addons/lib/dreamcast -L$(KOS_BASE)/../kos-ports/lib \
	-lz -lm -lSDL $(KOS_LIBS)

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist: $(TARGET)
	$(KOS_STRIP) $(TARGET)
