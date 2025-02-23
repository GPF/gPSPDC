# gpSP
# DC porting by Troy Davis (GPF)

TARGET = gdC.elf
OBJS = ../main.o ../cpu.o ../memory.o ../video.o ../input.o ../sound.o \
       ../gui.o ../zip.o ../cheats.o romdisk.o

KOS_ROMDISK_DIR = romdisk

include $(KOS_BASE)/Makefile.rules

KOS_CFLAGS += -D_arch_dreamcast=1 -Wno-implicit-function-declaration -Wno-implicit-int -Wno-return-mismatch \
    -I$(KOS_BASE)/addons/include \
    -I$(KOS_BASE)/addons/include/SDL \
    -I$(KOS_BASE)/../kos-ports/include/zlib \
    -I$(KOS_BASE)/../kos-ports/include/png

all: rm-elf $(TARGET)

clean: rm-elf
	-rm -f $(OBJS)

rm-elf:
	-rm -f $(TARGET) romdisk.*

$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS) -L$(KOS_BASE)/lib/dreamcast -L$(KOS_BASE)/addons/lib/dreamcast -L$(KOS_BASE)/../kos-ports/lib \
	-lz -lm -lSDL $(KOS_LIBS)

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist: $(TARGET)
	-rm -f $(OBJS) romdisk.img
	$(KOS_STRIP) $(TARGET)

# AR = ar
# AR_FLAGS = cru
# RANLIB = ranlib

# INC=-I. 

# OPT= -O3 -Wall  -ffast-math -funroll-loops -fstrict-aliasing -fexpensive-optimizations -falign-functions -fweb -frename-registers -fomit-frame-pointer -finline -finline-functions -fno-builtin -fno-common 


# OPT2 = -O9 -fomit-frame-pointer -fno-ident -freorder-blocks -funsigned-char -funsigned-char -funroll-all-loops -frerun-loop-opt  -falign-loops -fstrict-aliasing

# OPT3 = -O2 -Wall -ffast-math -fno-optimize-sibling-calls -fno-builtin -fno-strict-aliasing
# OPT4 = -DFRAME_POINTER -g3 -O0 -fno-eliminate-unused-debug-types 
# OPT5 = -Wno-implicit-function-declaration -Wno-implicit-int -Wno-return-mismatch
# #KOS_CFLAGS += $(INC) $(OPT2)
# KOS_CFLAGS = $(INC) -ml -m4-single-only -D_arch_dreamcast -D_arch_sub_pristine  \
# $(OPT5) \
# -I$(KOS_BASE)/../../sh-elf/sh-elf/include -I$(KOS_BASE)/include -I$(KOS_BASE)/kernel/arch/dreamcast/include -I$(KOS_BASE)/addons/include  -I$(KOS_BASE)/addons/include/SDL  -I$(KOS_BASE)/../kos-ports/include/zlib -I$(KOS_BASE)/../kos-ports/include/png


# KOS_CPPFLAGS+= ${KOS_CFLAGS} 




# TARGET = gdC.elf

# .PHONY: all all-before all-after clean clean-custom

# all: all-before $(TARGET) all-after

# include dreamcast.mak
 
# OBJS      = ../main.o ../cpu.o ../memory.o ../video.o ../input.o ../sound.o \
# 	     ../gui.o ../zip.o ../cheats.o #x86_stub.o ../cpu_threaded.o

# include $(KOS_BASE)/Makefile.rules

# clean:
# 	-rm -f $(TARGET) $(OBJS) romdisk.*

# rm-elf:
# 	-rm -f $(TARGET) romdisk.*

# $(TARGET): $(OBJS) $(PROBJS) romdisk.o
# 	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) \
# 		$(OBJS)  $(PROBJS)  romdisk.o $(OBJEXTRA) -L$(KOS_BASE)/lib -lgcc -lz -lm -lSDL -lstdc++ $(KOS_LIBS)



          
