savedcmd_arch/arm64/lib/built-in.a := rm -f arch/arm64/lib/built-in.a;  printf "arch/arm64/lib/%s " crc32.o | xargs aarch64-openwrt-linux-musl-ar cDPrST arch/arm64/lib/built-in.a
