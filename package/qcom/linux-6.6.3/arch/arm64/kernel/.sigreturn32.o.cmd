savedcmd_arch/arm64/kernel/sigreturn32.o := aarch64-openwrt-linux-musl-gcc -Wp,-MMD,arch/arm64/kernel/.sigreturn32.o.d -nostdinc -I./arch/arm64/include -I./arch/arm64/include/generated  -I./include -I./arch/arm64/include/uapi -I./arch/arm64/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/compiler-version.h -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -DKASAN_SHADOW_SCALE_SHIFT= -fmacro-prefix-map=./= -Werror -D__ASSEMBLY__ -fno-PIE -mabi=lp64 -fno-asynchronous-unwind-tables -fno-unwind-tables -DKASAN_SHADOW_SCALE_SHIFT= -g    -c -o arch/arm64/kernel/sigreturn32.o arch/arm64/kernel/sigreturn32.S 

source_arch/arm64/kernel/sigreturn32.o := arch/arm64/kernel/sigreturn32.S

deps_arch/arm64/kernel/sigreturn32.o := \
  include/linux/compiler-version.h \
    $(wildcard include/config/CC_VERSION_TEXT) \
  include/linux/kconfig.h \
    $(wildcard include/config/CPU_BIG_ENDIAN) \
    $(wildcard include/config/BOOGER) \
    $(wildcard include/config/FOO) \
  arch/arm64/include/asm/unistd.h \
    $(wildcard include/config/COMPAT) \
  arch/arm64/include/uapi/asm/unistd.h \
  include/uapi/asm-generic/unistd.h \
    $(wildcard include/config/MMU) \
  arch/arm64/include/uapi/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
    $(wildcard include/config/64BIT) \
  include/uapi/asm-generic/bitsperlong.h \

arch/arm64/kernel/sigreturn32.o: $(deps_arch/arm64/kernel/sigreturn32.o)

$(deps_arch/arm64/kernel/sigreturn32.o):
