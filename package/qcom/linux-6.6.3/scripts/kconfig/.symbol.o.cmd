savedcmd_scripts/kconfig/symbol.o := clang -Wp,-MMD,scripts/kconfig/.symbol.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -c -o scripts/kconfig/symbol.o scripts/kconfig/symbol.c

source_scripts/kconfig/symbol.o := scripts/kconfig/symbol.c

deps_scripts/kconfig/symbol.o := \
  scripts/kconfig/lkc.h \
    $(wildcard include/config/prefix) \
  scripts/kconfig/expr.h \
  scripts/kconfig/list.h \
  scripts/kconfig/lkc_proto.h \

scripts/kconfig/symbol.o: $(deps_scripts/kconfig/symbol.o)

$(deps_scripts/kconfig/symbol.o):
