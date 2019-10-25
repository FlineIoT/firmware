# Get version from version.ini
APP_MAJOR = $(shell sed -n -e 's/^\s*major\s*=\s*//p' version.ini)
APP_MINOR = $(shell sed -n -e 's/^\s*minor\s*=\s*//p' version.ini)
APP_PATCH = $(shell sed -n -e 's/^\s*patch\s*=\s*//p' version.ini)
BL_VERSION = $(shell sed -n -e 's/^\s*bl\s*=\s*//p' version.ini)
APP_VERSION = $(shell expr $(APP_MAJOR) \* 65536 + $(APP_MINOR) \* 256 + $(APP_PATCH))

compile_app:
	make -C fline/app/armgcc/ app_debug

compile_bootloader:
	make -C fline/bootloader/armgcc/ bootloader_debug

compile_all: compile_app compile_bootloader

# Erase chip
# Flash softdevice & bootloader
# Flash app
flash_all: 
	make -C fline/bootloader/armgcc/ flash bootloader_debug
	make -C fline/app/armgcc/ flash app_debug

clean: 
	@echo Cleaning built files
	make -C fline/bootloader/armgcc/ clean
	make -C fline/app/armgcc/ clean

# To make a new package, files are all cleaned before compilation from scratch
package: clean compile_all
	@echo "\033[92mPackage v$(APP_MAJOR).$(APP_MINOR).$(APP_PATCH), bl version:$(BL_VERSION)\033[0m"
	nrfutil pkg generate --application fline/app/armgcc/_build/app_debug.hex --application-version $(APP_VERSION) --sd-req 0xB7 --key-file fline/bootloader/dfu_req_handling/dfu_private_key.pem --hw-version 52 hex/dfu_debug_fline_app_v$(APP_MAJOR).$(APP_MINOR).$(APP_PATCH)-$(BL_VERSION).zip

# 	increment version rules
increment_minor:
	python utils/code/gen_version.py -m

increment_patch:
	python utils/code/gen_version.py -p

new_version: increment_patch
	git pull
	git commit -am "Fline - Release DFU package $(APP_MAJOR).$(APP_MINOR).$(APP_PATCH)-$(BL_VERSION)" --allow-empty
	git tag -a v/$(APP_MAJOR).$(APP_MINOR).$(APP_PATCH)-$(BL_VERSION) -m "Release Fline DFU package $(APP_MAJOR).$(APP_MINOR).$(APP_PATCH)-$(BL_VERSION)"
	@echo "\nRemaining TODOs:\n"
	grep -rnw -e TODO fline/

