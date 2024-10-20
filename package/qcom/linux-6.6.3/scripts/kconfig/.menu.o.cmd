savedcmd_scripts/kconfig/menu.o := clang -Wp,-MMD,scripts/kconfig/.menu.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -c -o scripts/kconfig/menu.o scripts/kconfig/menu.c

source_scripts/kconfig/menu.o := scripts/kconfig/menu.c

deps_scripts/kconfig/menu.o := \
  scripts/kconfig/lkc.h \
    $(wildcard include/config/prefix) \
  scripts/kconfig/expr.h \
  scripts/kconfig/list.h \
  scripts/kconfig/lkc_proto.h \
  scripts/kconfig/internal.h \

scripts/kconfig/menu.o: $(deps_scripts/kconfig/menu.o)

$(deps_scripts/kconfig/menu.o):
