# Inherit from common configuration
$(call inherit-product, device/sony/poplar/twrp_poplar_common.mk)

## Device identifier. This must come after all inclusions
PRODUCT_NAME := twrp_poplar_dsds
PRODUCT_DEVICE := poplar_dsds
PRODUCT_BRAND := Sony
PRODUCT_MODEL := G8342
PRODUCT_MANUFACTURER := Sony
