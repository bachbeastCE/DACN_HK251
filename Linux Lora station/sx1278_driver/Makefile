# Tên driver và overlay
MODULE_NAME := sx1278
DTBO_NAME := $(MODULE_NAME).dtbo
DTS_NAME := $(MODULE_NAME).dts

# Ð?i tu?ng build kernel module
obj-m += $(MODULE_NAME).o

# Ðu?ng d?n kernel build
KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

# Ðu?ng d?n cài d?t trên Pi 5 (Bookworm)
OVERLAY_DIR := /boot/firmware/overlays

all: module dt

# Biên d?ch Kernel Module
module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Biên d?ch Device Tree Overlay
dt: $(DTS_NAME)
	dtc -@ -I dts -O dtb -o $(DTBO_NAME) $(DTS_NAME)

# L?nh cài d?t, n?p overlay và insmod t? d?ng
install: all
	@echo "--- [1/3] Copying DTBO to system overlays ---"
	sudo cp $(DTBO_NAME) $(OVERLAY_DIR)/
	
	@echo "--- [2/3] Reloading Device Tree Overlay ---"
	@# D?u '-' d? b? qua l?i n?u overlay chua du?c n?p tru?c dó
	-sudo dtoverlay -r $(MODULE_NAME)
	sudo dtoverlay $(MODULE_NAME)
	
	@echo "--- [3/3] Inserting Kernel Module ---"
	@# G? module cu n?u có và n?p module m?i
	-sudo rmmod $(MODULE_NAME)
	sudo insmod $(MODULE_NAME).ko
	
	@echo "--- Done! Checking dmesg for Probe result ---"
	sudo dmesg | tail -n 15

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(DTBO_NAME)

.PHONY: all module dt clean install