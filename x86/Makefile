# gpSP makefile (MinGW-w64 cross-compilation on Debian)
# Gilead Kutnick - Exophase

# Global definitions (MinGW-w64 cross-compilation)
CC        = i686-w64-mingw32-gcc  # Using 32-bit target for SDL 1.2 compatibility
STRIP     = i686-w64-mingw32-strip
AS        = i686-w64-mingw32-as
SRC_DIR   = ..
OBJ_DIR   = ..
VPATH    += ..
PREFIX    = /usr
OBJS      = $(OBJ_DIR)/main.o $(OBJ_DIR)/cpu.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/video.o \
            $(OBJ_DIR)/input.o $(OBJ_DIR)/sound.o $(OBJ_DIR)/cpu_threaded.o \
            $(OBJ_DIR)/gui.o x86_stub.o $(OBJ_DIR)/cheats.o $(OBJ_DIR)/zip.o
BIN       = gpsp.exe 

# Platform specific definitions (Windows target)
VPATH    += ..
CFLAGS   += -DPC_BUILD -Wall -m32 -fno-stack-protector
INCLUDES  =  -I/usr/local/cross-tools/i686-w64-mingw32/include/SDL -I/usr/local/cross-tools/i686-w64-mingw32/include/
LIBS = -L/usr/local/cross-tools/i686-w64-mingw32/lib -lmingw32 -lSDLmain -lSDL -lz -m32 -mconsole

# Compilation rules
.SUFFIXES: .c .S

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S
	$(AS) -o $@ $<

all: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $(BIN)
	$(STRIP) $(BIN)

clean:
	rm -f $(OBJS) $(BIN)