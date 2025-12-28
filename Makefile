MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

ASMSH_FLAGS=-debug=d -cpu=sh4 -endian=little -sjis
BUILD_DIR=build
OUTPUT_DIR=$(BUILD_DIR)/output
SHA1_CHECKSUM=a6df9e0de39b2d11e9339aef915d20e35763ec81
SHELL := /bin/bash

SRCS = \
	src/010080_main.c \
	src/0100bc_sound.c \
	src/010e90.c \
	src/asm/010fe8_unused.src \
	src/011120_asset_queues.c \
	src/012324_peripheral_support.c \
	src/asm/012504.src \
	src/asm/0129cc.src \
	src/012f44.c \
	src/asm/013ae8_pre_data.src \
	src/asm/013ae8.src \
	src/014934.c \
	src/0149b0_sbinit.c \
	src/014a9c_tasks.c \
	src/014b8c_backup.c \
	src/asm/014f54_text_pre_data.src \
	src/014f54_text.c \
	src/015ab8_title.c \
	src/016108.c \
	src/asm/01614c.src \
	src/asm/016bf4.src \
	src/016c58_prompt.c \
	src/016d2c_course_menu.c \
	src/asm/018644.src \
	src/asm/018784.src \
	src/asm/0193c8_pre_data.src \
	src/0193c8_vm_menu.c \
	src/019e98_main_menu.c \
	src/asm/01a148.src \
	src/asm/01b19c.src \
	src/asm/01bb48.src \
	src/asm/01c980.src \
	src/asm/01d290.src \
	src/asm/01d7fc.src \
	src/asm/01e27c.src \
	src/asm/01f3c0.src \
	src/asm/01fa78.src \
	src/asm/020214.src \
	src/020528.c \
	src/asm/020594.src \
	src/asm/0206f0.src \
	src/0207d4.c \
	src/asm/02081c.src \
	src/asm/020914.src \
	src/asm/020b6c.src \
	src/asm/02171c.src \
	src/asm/021b9c.src \
	src/asm/0222dc.src \
	src/asm/022464.src \
	src/asm/022bdc.src \
	src/asm/023310.src \
	src/asm/023938.src \
	src/asm/02412c.src \
	src/asm/024280.src \
	src/asm/024b4c.src \
	src/asm/025870.src \
	src/asm/025b98.src \
	src/asm/026710.src \
	src/asm/02786c.src \
	src/asm/027958.src \
	src/asm/028258.src \
	src/asm/02af78.src \
	src/asm/02b2f0.src \
	src/asm/02b464.src \
	src/asm/02c884.src \
	src/asm/02d06c.src \
	src/asm/02d19c.src \
	src/asm/02d968.src \
	src/asm/02df3c.src \
	src/asm/02e2dc.src \
	src/asm/02e400.src \
	src/asm/02e51c.src \
	src/asm/02f0c8.src \
	src/asm/02f320.src \
	src/asm/0332a4_sectionC.src \
	src/asm/03bd80_sectionD.src \
	src/asm/0fcd20_sectionB.src \
	src/02fb50_sh4nlfzn_post_data.c \
	src/scif.c \
	src/serial_debug.c

OBJS = $(patsubst src/%.c,$(OUTPUT_DIR)/src/%.obj,$(SRCS))
OBJS := $(patsubst src/asm/%.src,$(OUTPUT_DIR)/src/asm/%.obj,$(OBJS))
LINKER_OBJS = $(subst /,\\, $(OBJS))

all: create_dirs $(OUTPUT_DIR)/tbg.bin

create_dirs:
	@mkdir -p $(OUTPUT_DIR)/src/asm

$(OUTPUT_DIR)/src/asm/%.obj: src/asm/%.src
	wibo "$(SHC_BIN)/asmsh.exe" "$(subst /,\\,$<)" -object="$(subst /,\\,$@)" $(ASMSH_FLAGS)

$(OUTPUT_DIR)/src/%.obj: src/%.c
	wibo "$(SHC_BIN)/shc.exe" "$(subst /,\\,$<)" -object="$(subst /,\\,$@)" -sub=$(BUILD_DIR)/shc.sub

$(OUTPUT_DIR)/tbg.elf: $(OBJS) $(BUILD_DIR)/lnk.sub
	wibo "$(SHC_BIN)/lnk.exe" -sub=build\\lnk.sub

$(BUILD_DIR)/lnk.sub: $(BUILD_DIR)/lnk_template.sub
	sed "s|@DC_SDK@|$$(printf %q "$(KATANA_SDK_DIR)")|g" $(BUILD_DIR)/lnk_template.sub > $(BUILD_DIR)/lnk.sub
	sed -i 's|@INPUTS@|$(foreach obj,$(LINKER_OBJS),input $(obj)\n)|g' $(BUILD_DIR)/lnk.sub

$(OUTPUT_DIR)/tbg.bin: $(OUTPUT_DIR)/tbg.elf
	wibo "$(KATANA_SDK_DIR)/bin/elf2bin.exe" -s 8c010000 "$(subst /,\\,$<)"
	@if ! echo "$(SHA1_CHECKSUM) *$(OUTPUT_DIR)/tbg.bin" | sha1sum --status -c -; then \
		echo "================" ;\
		echo "Project built :)" ;\
		echo "================" ;\
	else \
		echo "===========================" ;\
		echo "Matching project built! \o/" ;\
		echo "===========================" ;\
	fi

clean:
	rm -rf $(OUTPUT_DIR) $(OUTPUT_DIR)/lnk.sub

depend:
	makedepend -Y -o .obj -f- $(C_SRCS) 2>/dev/null > Makefile.d
	sed -i 's/^src/$$(OUTPUT_DIR)/' Makefile.d

.PHONY: all clean create_dirs $(OUTPUT_DIR)/tbg.bin

include Makefile.d
