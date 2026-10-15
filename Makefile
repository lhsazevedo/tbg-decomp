MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

ASMSH_FLAGS=-debug -cpu=sh4 -endian=little -sjis
BUILD_DIR=build
OUTPUT_DIR=$(BUILD_DIR)/output

SHELL := /bin/bash

# Enable serial logging
SERIAL_DEBUG ?= 0

# Overrides the default log level (INFO) for serial logging. See LOG_LEVEL constants
LOG_LEVEL ?=

# Boot straight into the unused debug menu
DEBUG_MENU ?=

# Select the language for the game.
# Defaults to the original Japanese release.
GAME_LANG ?= ja

SHC_DEFINES := __SHC__
ifeq ($(SERIAL_DEBUG),1)
SHC_DEFINES += SERIAL_DEBUG
endif

ifeq ($(DEBUG_MENU),1)
SHC_DEFINES += DEBUG_MENU
endif

ifeq ($(GAME_LANG),en)
SHC_DEFINES += GAME_LANG_EN
endif

ifneq ($(LOG_LEVEL),)
SHC_DEFINES += DEBUG_LEVEL=LOG_LEVEL_$(LOG_LEVEL)
endif

empty :=
space := $(empty) $(empty)
comma := ,
SHC_DEFINE_ARG = -define=$(subst $(space),$(comma),$(SHC_DEFINES))

SRCS = \
	src/010080_main.c \
	src/0100bc_sound.c \
	src/010e90_vibration.c \
	src/010fe8_heap.c \
	src/011120_asset_queues.c \
	src/012324_input.c \
	src/0129cc_game.c \
	src/013ae8_route.c \
	src/014934_unused_load.c \
	src/0149b0_sbinit.c \
	src/1ba1c8_globals.c \
	src/014a9c_tasks.c \
	src/014b8c_backup.c \
	src/014f54_sprite.c \
	src/015034_text.c \
	src/015ab8_title.c \
	src/016108_resgrp_free.c \
	src/01614c_replay_menu.c \
	src/016c58_prompt.c \
	src/016d2c_course_menu.c \
	src/018644_file_select.c \
	src/0193c8_vm_select.c \
	src/019e98_main_menu.c \
	src/01a148_option.c \
	src/01b19c_system_menu.c \
	src/01bb48_vm_game.c \
	src/01c980_profile_file.c \
	src/01d290_album.c \
	src/01d7fc_results.c \
	src/01e27c_practice_menu.c \
	src/01f3c0_ending.c \
	src/01fa78_hud.c \
	src/020214_drive_cue.c \
	src/020594_vehicle_model.c \
	src/0206f0_intersect.c \
	src/0207d4_vec_xz.c \
	src/02081c_geom.c \
	src/020914_ground_query.c \
	src/020b6c_ground_probe.c \
	src/02171c_tile_stream.c \
	src/021b9c_tile_draw.c \
	src/022464_render.c \
	src/022bdc_bus.c \
	src/023310_bus_init.c \
	src/023938_bus_drive.c \
	src/02412c_bus_line.c \
	src/024280_bus_input.c \
	src/024b4c_bus_camera.c \
	src/025870_cinematic.c \
	src/025b98_traffic_drive.c \
	src/026710_traffic.c \
	src/02786c_vehicle_parts.c \
	src/027958_bus_draw.c \
	src/028258_signal.c \
	src/0289ac_objects.c \
	src/02a9fc_message_box.c \
	src/02af78_event.c \
	src/02b2f0_drive_msg.c \
	src/02b464_grading.c \
	src/02c884_stop.c \
	src/02d06c_stop_draw.c \
	src/02d19c_passenger.c \
	src/02d968_stop_spawn.c \
	src/02df3c_traffic_lookahead.c \
	src/02e2dc_bus_collision.c \
	src/02e400_collision.c \
	src/02e51c_attr_query.c \
	src/02f0c8_traffic_path_scan.c \
	src/02f320_replay_codec.c \
	src/scif.c \
	src/serial_debug.c \
	src/04ce10_line_nodes.c \
	src/235ca0_nj_buffers.c \

C_SRCS = $(filter %.c,$(SRCS))

OBJS = $(patsubst src/%.c,$(OUTPUT_DIR)/src/%.obj,$(SRCS))
OBJS := $(patsubst src/asm/%.src,$(OUTPUT_DIR)/src/asm/%.obj,$(OBJS))
LINKER_OBJS = $(subst /,\\, $(OBJS))

# A full build is a few seconds; make a stale-object mistake impossible by
# default. `make all` opts into the incremental build.
.DEFAULT_GOAL := rebuild

all: $(OUTPUT_DIR)/tbg.bin

# Track changes to the compiler defines
$(OUTPUT_DIR)/defines: FORCE
	@mkdir -p $(@D)
	@printf '%s\n' '$(SHC_DEFINES)' > $@.tmp
	@cmp -s $@.tmp $@ || cp $@.tmp $@
	@rm -f $@.tmp

FORCE:

# Compile ASM files
$(OUTPUT_DIR)/src/asm/%.obj: src/asm/%.src $(OUTPUT_DIR)/defines
	@mkdir -p $(@D)
	wibo "$(SHC_BIN)/asmsh.exe" "$(subst /,\\,$<)" -object="$(subst /,\\,$@)" $(ASMSH_FLAGS)

# Compile C files
$(OUTPUT_DIR)/src/%.obj: src/%.c $(OUTPUT_DIR)/defines
	@mkdir -p $(@D)
	wibo "$(SHC_BIN)/shc.exe" "$(subst /,\\,$<)" -object="$(subst /,\\,$@)" -sub=$(BUILD_DIR)/shc.sub $(SHC_DEFINE_ARG)
	wibo "$(SHC_BIN)/shc.exe" "$(subst /,\\,$<)" -code=asm -object="$(subst /,\\,$@).src" -sub=$(BUILD_DIR)/shc.sub

$(OUTPUT_DIR)/tbg.elf: $(OBJS) $(BUILD_DIR)/lnk.sub
	wibo "$(SHC_BIN)/lnk.exe" -sub=build\\lnk.sub

$(BUILD_DIR)/lnk.sub: $(BUILD_DIR)/lnk_template.sub Makefile
	sed "s|@DC_SDK@|$$(printf %q "$(KATANA_SDK_DIR)")|g" $(BUILD_DIR)/lnk_template.sub > $(BUILD_DIR)/lnk.sub
	sed -i 's|@INPUTS@|$(foreach obj,$(LINKER_OBJS),input $(obj)\n)|g' $(BUILD_DIR)/lnk.sub

$(OUTPUT_DIR)/tbg.bin: $(OUTPUT_DIR)/tbg.elf
	wibo "$(KATANA_SDK_DIR)/bin/elf2bin.exe" -s 8c010000 "$(subst /,\\,$<)"
	@echo "================"
	@echo "Project built :)"
	@echo "================"

graph: all
	python3 scripts/generate_graph.py

clean:
	rm -rf $(OUTPUT_DIR) $(BUILD_DIR)/lnk.sub

rebuild: clean all

depend:
	makedepend -Y -o .obj -f- $(C_SRCS) 2>/dev/null > Makefile.d
	sed -i 's|^src/|$$(OUTPUT_DIR)/src/|' Makefile.d

.PHONY: all graph clean rebuild FORCE

include Makefile.d
