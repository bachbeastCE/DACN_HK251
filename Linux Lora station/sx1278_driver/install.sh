#!/bin/bash

MODULE_NAME="sx1278"
MODULE_FILE="${MODULE_NAME}.ko"
KERNEL_VERSION=$(uname -r)
MODULE_DIR="/lib/modules/${KERNEL_VERSION}/kernel/drivers/lora"

echo "[*] Installing module ${MODULE_NAME}..."

# Tạo thư mục nếu chưa có
sudo mkdir -p ${MODULE_DIR}

# Copy module
sudo cp ${MODULE_FILE} ${MODULE_DIR}/

# Update dependency
sudo depmod

# Add auto-load nếu chưa có
if ! grep -q "^${MODULE_NAME}$" /etc/modules; then
    echo ${MODULE_NAME} | sudo tee -a /etc/modules
fi

# Load module ngay
sudo modprobe ${MODULE_NAME}

echo "[✓] Module installed and loaded successfully!"
