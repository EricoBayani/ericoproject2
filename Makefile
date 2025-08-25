ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else
		UNAME := Windows
	endif
else
	UNAME := $(shell uname)
endif

TARGET = ericoproject2clean.elf

.DEFAULT: all

# Directories

src_dir = .
ben_library_dir = ./BenDemoLibrary
corelib_dir = ../../../SDK

libxmploaders = ../../../3rdparty/libxmp/loaders
libxmploadersextra = ../../../3rdparty/libxmp/loaders/prowizard
libxmpdepackers = ../../../3rdparty/libxmp/depackers
libxmp = ../../../3rdparty/libxmp

# Rules

ifeq ($(UNAME), Windows)
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 -Wno-narrowing --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Warray-bounds=0 -Wstringop-overflow=0 -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -Wl,--strip-all -lgcc -lm

else
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -g -std=c++20 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -O0 -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -lgcc -lm
endif

RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump


incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(libxmp)/ -I$(libxmploaders)/ -I$(libxmploadersextra)/ -I$(libxmpdepackers)/ -I$(src_dir)/.
 

benincs := $(incs)
benincs += -I$(ben_library_dir)

libs += $(wildcard $(corelib_dir)/*.c) $(wildcard $(libxmp)/*.c) $(wildcard $(libxmploaders)/*.c) $(wildcard $(libxmploadersextra)/*.c) $(wildcard $(libxmpdepackers)/*.c)

benlibs := $(libs)
benlibs += $(wildcard $(ben_library_dir)/*.cpp)

$(TARGET): $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -o $@ $(wildcard $(src_dir)/*.cpp) -D LIBXMP_NO_DEPACKERS $(libs) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: $(TARGET)

$(TARGET):
	$(RISCV_GCC) $(benincs) -o $@ $(wildcard $(src_dir)/*.cpp) -D LIBXMP_NO_DEPACKERS $(benlibs) $(RISCV_GCC_OPTS)

dump: $(TARGET)
	$(RISCV_OBJDUMP) $(TARGET) -x -D -S >> $(TARGET).txt

.PHONY: clean
clean:
ifeq ($(UNAME), Windows)
	del *.elf
else
	rm -rf *.elf
endif

setup:
ifeq ($(UNAME), Windows)
	.\setup.bat
else
	./setup.sh
endif
	

again: clean $(TARGET) setup