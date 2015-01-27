OUTDIR = build

TARGET = meanshift

# Source files:
SRC += main.c 
SRC += mean_shift.c 

CFLAGS += -I.
CFLAGS += -std=gnu99 
CFLAGS += -Wall -g -O2
LDFLAGS += -lm

TCHAIN_PREFIX = 
CC      = $(TCHAIN_PREFIX)gcc
AR      = $(TCHAIN_PREFIX)ar
OBJCOPY = $(TCHAIN_PREFIX)objcopy
OBJDUMP = $(TCHAIN_PREFIX)objdump
SIZE    = $(TCHAIN_PREFIX)size
NM      = $(TCHAIN_PREFIX)nm
REMOVE  = rm -rf

# List of all source files without directory and file-extension.
ALLSRCBASE = $(notdir $(basename $(SRC)))

# Define all object files.
ALLOBJ     = $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

default: all

# Default target.
all: $(OUTDIR)/$(TARGET).elf

# Link: create ELF output file from object files.
.SECONDARY : $(OUTDIR)/$(TARGET).elf
.PRECIOUS : $(ALLOBJ)
$(OUTDIR)/$(TARGET).elf: $(ALLOBJ)
	@echo
	@echo "**** Linking :" $@
	@$(CC) $(ALLOBJ) -o $(OUTDIR)/$(TARGET).elf $(CFLAGS) $(LDFLAGS)

# Compile: create object files from C source files.
define COMPILE_C_SOURCE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo "**** Compiling :" $$< "->" $$@
	@$(CC) -c $$(CFLAGS) $$< -o $$@
endef
$(foreach src, $(SRC), $(eval $(call COMPILE_C_SOURCE, $(src))))

%.bin:  %.asm
	$(OUTDIR)/$(TARGET).elf $<

clean:
	@echo "cleaned build"
	@$(REMOVE) $(OUTDIR)
	$(foreach test,$(ALL_TESTS), rm tests/$(test)/*.bin -f;)

$(shell mkdir -p $(OUTDIR))
