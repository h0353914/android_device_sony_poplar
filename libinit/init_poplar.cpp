/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fcntl.h>
#include <strings.h>
#include <sys/_system_properties.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <cctype>
#include <cstring>
#include <string>
#include <string_view>

#include <android-base/logging.h>
#include <android-base/unique_fd.h>

#include "vendor_init.h"

namespace {

constexpr const char* kLtaLabel = "/dev/block/bootdevice/by-name/LTALabel";

// 此時 ueventd 還沒建立 by-name 連結，LTALabel 固定是 sda2 (8:2)，自己建暫時的節點來讀
// (init 不能開 sysfs 內沒有明確標籤的檔案，所以沒辦法掃 /sys/class/block 找 PARTNAME)
constexpr const char* kLtaLabelTmpNode = "/dev/.poplar_ltalabel";
constexpr unsigned kLtaLabelMajor = 8;
constexpr unsigned kLtaLabelMinor = 2;

constexpr std::string_view kModels[] = {"SO-01K", "701SO", "SOV36", "G8341", "G8342", "G8343"};
constexpr std::string_view kNbsp = "&nbsp;";

// 型號在 LTALabel 內的位置固定（已驗證的日本機型都在 4514481），只讀這一小段 (4 KiB 對齊)，
// 規則比對不到就保留預設值，其他機型的位置之後再調整
constexpr off_t kWindowOffset = 4513792;
constexpr size_t kWindowSize = 8192;

// LTALabel 的型號有兩種寫法（規則同 init.qcom.msim.sh）：
//   全球版 "model: G8342"；日本版 "SO-01K&nbsp;"，701SO 在型號與 &nbsp; 之間多一個空白
bool IsModelAt(std::string_view buf, size_t pos, size_t len) {
    size_t end = pos + len;
    if (end < buf.size() && (isalnum(static_cast<unsigned char>(buf[end])) || buf[end] == '-')) {
        return false;
    }

    size_t before = pos;
    while (before > 0 && buf[before - 1] == ' ') before--;
    constexpr std::string_view kKey = "model:";
    if (before >= kKey.size() &&
        strncasecmp(buf.data() + before - kKey.size(), kKey.data(), kKey.size()) == 0) {
        return true;
    }

    while (end < buf.size() && buf[end] == ' ') end++;
    return buf.substr(end, kNbsp.size()) == kNbsp;
}

std::string_view FindModel(std::string_view buf) {
    for (std::string_view model : kModels) {
        for (size_t pos = buf.find(model); pos != std::string_view::npos;
             pos = buf.find(model, pos + 1)) {
            if (IsModelAt(buf, pos, model.size())) return model;
        }
    }
    return {};
}

android::base::unique_fd OpenLtaLabel() {
    android::base::unique_fd fd(open(kLtaLabel, O_RDONLY | O_CLOEXEC));
    if (fd >= 0) return fd;

    unlink(kLtaLabelTmpNode);
    if (mknod(kLtaLabelTmpNode, S_IFBLK | 0600, makedev(kLtaLabelMajor, kLtaLabelMinor)) != 0) {
        PLOG(WARNING) << "Cannot create " << kLtaLabelTmpNode;
        return fd;
    }
    fd.reset(open(kLtaLabelTmpNode, O_RDONLY | O_CLOEXEC));
    if (fd < 0) PLOG(WARNING) << "Cannot open " << kLtaLabelTmpNode;
    unlink(kLtaLabelTmpNode);
    return fd;
}

std::string ReadModel() {
    android::base::unique_fd fd = OpenLtaLabel();
    if (fd < 0) return "";

    char buf[kWindowSize];
    ssize_t n = pread(fd, buf, sizeof(buf), kWindowOffset);
    if (n <= 0) {
        PLOG(WARNING) << "Cannot read LTALabel";
        return "";
    }
    return std::string(FindModel(std::string_view(buf, n)));
}

void property_override(const char* name, const char* value) {
    auto* pi = const_cast<prop_info*>(__system_property_find(name));
    if (pi != nullptr) {
        __system_property_update(pi, value, strlen(value));
    } else {
        __system_property_add(name, strlen(name), value, strlen(value));
    }
}

}  // namespace

// 關於手機顯示的型號改用機身 LTALabel 內的原廠型號，讀不到就保留編譯時的預設值
void vendor_load_properties() {
    std::string model = ReadModel();
    if (model.empty()) {
        LOG(WARNING) << "Model not found in LTALabel, keeping the default";
        return;
    }
    property_override("ro.product.model", model.c_str());
}
