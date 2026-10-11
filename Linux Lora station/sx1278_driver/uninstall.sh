#!/bin/bash

MODULE_NAME="sx1278"
KERNEL_VERSION=$(uname -r)
MODULE_DIR="/lib/modules/${KERNEL_VERSION}/kernel/drivers/lora"

echo "[*] Removing module ${MODULE_NAME}..."

# Unload nếu đang chạy
sudo rmmod ${MODULE_NAME} 2>/dev/null

# Xoá file .ko
sudo rm -f ${MODULE_DIR}/${MODULE_NAME}.ko

# Update dependency
sudo depmod

# Xoá khỏi auto-load
sudo sed -i "/^${MODULE_NAME}$/d" /etc/modules

echo "[✓] Module removed successfully!"
