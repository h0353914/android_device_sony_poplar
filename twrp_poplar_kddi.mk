# Inherit from common configuration
$(call inherit-product, device/sony/poplar/twrp_poplar_common.mk)

## Device identifier. This must come after all inclusions
PRODUCT_NAME := twrp_poplar_kddi
PRODUCT_DEVICE := poplar_kddi
PRODUCT_BRAND := Sony
PRODUCT_MODEL := G8341
PRODUCT_MANUFACTURER := Sony
