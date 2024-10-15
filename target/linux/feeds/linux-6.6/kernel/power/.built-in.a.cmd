savedcmd_kernel/power/built-in.a := rm -f kernel/power/built-in.a;  printf "kernel/power/%s " qos.o poweroff.o | xargs aarch64-openwrt-linux-musl-ar cDPrST kernel/power/built-in.a
