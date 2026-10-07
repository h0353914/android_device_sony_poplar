/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

// poplar_i2c_probe <bus> <address>
//
// 對 /dev/i2c-<bus> 上的位址做一次 1 byte 的讀取，只看有沒有裝置回應。
// 結束碼: 0 = 有回應, 1 = 沒有回應, 2 = 無法探測 (開不了節點等)
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <bus> <address>\n", argv[0]);
        return 2;
    }

    char path[32];
    snprintf(path, sizeof(path), "/dev/i2c-%ld", strtol(argv[1], nullptr, 0));
    int fd = open(path, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        perror(path);
        return 2;
    }

    // 位址可能已經被驅動佔用，所以要用 FORCE
    if (ioctl(fd, I2C_SLAVE_FORCE, strtol(argv[2], nullptr, 0)) < 0) {
        perror("I2C_SLAVE_FORCE");
        return 2;
    }

    unsigned char data;
    if (read(fd, &data, 1) == 1) return 0;

    // 沒有裝置回應時 msm-i2c 回 ENOTCONN，其他控制器可能回 ENXIO 或 EREMOTEIO
    if (errno == ENOTCONN || errno == ENXIO || errno == EREMOTEIO) return 1;

    perror("read");
    return 2;
}
