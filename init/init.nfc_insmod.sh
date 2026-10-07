#!/vendor/bin/sh
#
# 依實際裝上的 NFC 控制器載入驅動模組。
# 日本機是 Sony CXD224x，國際版是 NXP PN553，兩者接在同一條 I2C 上，驅動 probe 時都不會檢查晶片在不在。
# PN553 沒開機時不會回應，所以改看只有日本機才有的 FeliCa 供電 LDO (BD7602, 位址 0x1e)，
# 有回應就是日本機，不依賴任何驅動。重試 3 次仍探測不了就當作日本機。
# 清單檔 init.insmod.nfc_<chip>.cfg 的格式比照 Google 的 init.insmod.sh，每行 動作|參數：
#   modprobe|<模組>   載入模組
#   wait|<路徑>       等該路徑出現（最多 3 秒）
#   setprop|<屬性>    把屬性設成 1
# 任何一步失敗就停下來，不設定完成旗標，避免 HAL 在沒有裝置節點時被啟動。

log() {
    echo "init.nfc_insmod: $*" > /dev/kmsg
}

chip=
tries=0
while [ -z "$chip" ] && [ "$tries" -lt 3 ]; do
    /vendor/bin/poplar_i2c_probe 7 0x1e
    case $? in
        0) chip=cxd;;
        1) chip=nxp;;
    esac
    tries=$((tries + 1))
    [ -z "$chip" ] && sleep 0.1
done

if [ -z "$chip" ]; then
    log "cannot probe the I2C bus, assuming cxd"
    chip=cxd
fi

cfg_file=/vendor/etc/init.insmod.nfc_${chip}.cfg

[ -f "$cfg_file" ] || { log "$cfg_file not found"; exit 1; }

while IFS="|" read -r action arg; do
    case "$action" in
        "modprobe")
            modprobe -a -d /vendor/lib/modules "$arg" || { log "modprobe $arg failed"; exit 1; };;
        "wait")
            i=0
            while [ ! -e "$arg" ] && [ "$i" -lt 30 ]; do
                sleep 0.1
                i=$((i + 1))
            done
            [ -e "$arg" ] || { log "$arg did not appear"; exit 1; };;
        "setprop") setprop "$arg" 1;;
    esac
done < "$cfg_file"
