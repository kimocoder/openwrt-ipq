savedcmd_usr/built-in.a := rm -f usr/built-in.a;  printf "usr/%s " initramfs_data.o | xargs aarch64-openwrt-linux-musl-ar cDPrST usr/built-in.a
