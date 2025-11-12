# gpSP
# DC porting by Troy Davis (GPF)

TARGET = gdC.elf
OBJS =  sh4_stub.o  ../main.o ../cpu_threaded.o ../cpu.o ../memory.o ../video.o ../input.o ../sound.o \
       ../gui.o ../zip.o ../cheats.o 

KOS_ROMDISK_DIR = romdisk

include $(KOS_BASE)/Makefile.rules

KOS_CFLAGS += -D_arch_dreamcast=1 -save-temps -fno-lto -DDEBUG_DYNAREC=1 -Wno-comment -Wno-unused-variable -Wno-switch -Wno-parentheses -Wno-int-conversion -Wno-implicit-function-declaration -ffixed-r12 -ffixed-r13 \
    -I.. \
    -I$(KOS_BASE)/addons/include \
    -I$(KOS_BASE)/addons/include/dreamcast/SDL \
    -I$(KOS_BASE)/../kos-ports/include/zlib \
    -I$(KOS_BASE)/../kos-ports/include/png

all: rm-elf $(TARGET)

clean: rm-elf
	-rm -f $(OBJS) $(OBJS:.o=.i) $(OBJS:.o=.s) romdisk.*

rm-elf:
	-rm -f $(TARGET) romdisk.*

$(TARGET): $(OBJS)
	kos-cc  -v -o $(TARGET) $(OBJS) -L$(KOS_BASE)/lib/dreamcast -L$(KOS_BASE)/addons/lib/dreamcast -L$(KOS_BASE)/../kos-ports/lib \
	-lz -lm -lSDL $(KOS_LIBS) -Wl,-Map=prog.map

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist: $(TARGET)
	-rm -f $(OBJS) romdisk.img
	$(KOS_STRIP) $(TARGET)