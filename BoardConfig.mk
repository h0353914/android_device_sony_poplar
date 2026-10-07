#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

DEVICE_PATH := device/sony/poplar

# Partitions
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 5242880000
BOARD_VENDORIMAGE_PARTITION_SIZE := 1610612736
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4

# Inherit common board configs
include device/sony/yoshino-common/BoardConfigCommon.mk

# Display
TARGET_SCREEN_DENSITY := 430

# Kernel
TARGET_KERNEL_CONFIG += sony/poplar.config

# Properties
TARGET_SYSTEM_PROP += $(DEVICE_PATH)/system.prop
TARGET_VENDOR_PROP += $(DEVICE_PATH)/vendor.prop

# Recovery
TARGET_RECOVERY_FSTAB := $(DEVICE_PATH)/init/fstab.qcom

# SEPolicy
BOARD_VENDOR_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/vendor

# Init
SOONG_CONFIG_NAMESPACES += libinit
SOONG_CONFIG_libinit := vendor_init_lib
SOONG_CONFIG_libinit_vendor_init_lib := //$(DEVICE_PATH):libinit_poplar

# Inherit vendor board configs
include vendor/sony/poplar/BoardConfigVendor.mk
