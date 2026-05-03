MODULE_NAME := sx1278
DTBO_NAME := $(MODULE_NAME).dtbo
DTS_NAME := $(MODULE_NAME)_overlays.dts

obj-m += $(MODULE_NAME).o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

OVERLAY_DIR := /boot/firmware/overlays

all: module dt

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

dt: $(DTS_NAME)
	dtc -@ -I dts -O dtb -o $(DTBO_NAME) $(DTS_NAME)

install: all
	@echo "--- [1/3] Copying DTBO to system overlays ---"
	sudo cp $(DTBO_NAME) $(OVERLAY_DIR)/
	
# 	@echo "--- [2/3] Reloading Device Tree Overlay ---"
# 	-sudo dtoverlay -r $(MODULE_NAME)
# 	sudo dtoverlay $(MODULE_NAME)
	
# 	@echo "--- [3/3] Inserting Kernel Module ---"
# 	-sudo rmmod $(MODULE_NAME)
# 	sudo insmod $(MODULE_NAME).ko
	
# 	@echo "--- Done! Checking dmesg for Probe result ---"
# 	sudo dmesg | tail -n 15

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(DTBO_NAME)

.PHONY: all module dt clean install