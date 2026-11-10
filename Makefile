######################################################
#                                                    #
#                     Makefile                       #
# Martin Doff-Sotta (martin.doff-sotta@eng.ox.ac.uk) #
#                                                    #
######################################################

TARGET = main

MCU_SPEC = cortex-m4

# Define the chip architecture.
LD_SCRIPT = STM32F411CCUX_FLASH.ld

TOOLCHAIN =
CC  = arm-none-eabi-gcc
CPP = arm-none-eabi-g++
AS  = arm-none-eabi-as
OC  = arm-none-eabi-objcopy
OS  = arm-none-eabi-size

# Assembly directives.
ASFLAGS += -mcpu=$(MCU_SPEC)
ASFLAGS += -mthumb

# C compilation directives
CFLAGS += -mcpu=$(MCU_SPEC)
CFLAGS += -mthumb
CFLAGS += -mfloat-abi=hard
CFLAGS += -mfpu=fpv4-sp-d16
CFLAGS += -DSTM32F411xE
CFLAGS += -Wall
CFLAGS += -g
CFLAGS += -Os
CFLAGS += -fmessage-length=0 -fno-common
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -std=c99

# C++ compilation directives
CPPFLAGS += -mcpu=$(MCU_SPEC)
CPPFLAGS += -mthumb
CPPFLAGS += -mfloat-abi=hard
CPPFLAGS += -mfpu=fpv4-sp-d16
CPPFLAGS += -DSTM32F411xE
CPPFLAGS += -Wall
CPPFLAGS += -g
CPPFLAGS += -Os
CPPFLAGS += -fmessage-length=0 -fno-common
CPPFLAGS += -ffunction-sections -fdata-sections
CPPFLAGS += -fno-exceptions

# Linker directives.
LSCRIPT = ./$(LD_SCRIPT)
LFLAGS += -mcpu=$(MCU_SPEC)
LFLAGS += -mthumb
LFLAGS += -mfloat-abi=hard
LFLAGS += -mfpu=fpv4-sp-d16
LFLAGS += -Wall
LFLAGS += --specs=nosys.specs
LFLAGS += --static
LFLAGS += -Wl,-Map=$(TARGET).map
LFLAGS += -Wl,--gc-sections
LFLAGS += -lgcc
LFLAGS += -lc
LFLAGS += -T$(LSCRIPT)

# Source files.
AS_SRC    = ./startup/startup_stm32f411xe.s
C_SRC   = ./src/main.c
C_SRC  += ./src/system_stm32f4xx.c

INCLUDE  += -I./include
INCLUDE  += -I./drivers

OBJS  = $(C_SRC:.c=.o)
OBJS += $(CPP_SRC:.cpp=.o)
OBJS += $(AS_SRC:.s=.o)

.PHONY: all
all: $(TARGET).bin

%.o: %.s
	$(AS) $(ASFLAGS) -c $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OC) -S -O binary $< $@
	$(OS) $<

.PHONY: flash 
flash: all
	st-flash erase 
	st-flash write ./$(TARGET).bin 0x08000000 

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f $(TARGET).elf
	rm -f $(TARGET).bin
	rm -f $(TARGET).map