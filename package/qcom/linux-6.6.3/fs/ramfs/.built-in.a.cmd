savedcmd_fs/ramfs/built-in.a := rm -f fs/ramfs/built-in.a;  printf "fs/ramfs/%s " inode.o file-mmu.o | xargs aarch64-openwrt-linux-musl-ar cDPrST fs/ramfs/built-in.a
