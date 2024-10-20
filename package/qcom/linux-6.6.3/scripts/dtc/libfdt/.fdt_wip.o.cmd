savedcmd_scripts/dtc/libfdt/fdt_wip.o := clang -Wp,-MMD,scripts/dtc/libfdt/.fdt_wip.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include -I ./scripts/dtc/libfdt -DNO_YAML  -c -o scripts/dtc/libfdt/fdt_wip.o scripts/dtc/libfdt/fdt_wip.c

source_scripts/dtc/libfdt/fdt_wip.o := scripts/dtc/libfdt/fdt_wip.c

deps_scripts/dtc/libfdt/fdt_wip.o := \
  scripts/dtc/libfdt/libfdt_env.h \
  scripts/dtc/libfdt/fdt.h \
  scripts/dtc/libfdt/libfdt.h \
  scripts/dtc/libfdt/libfdt_internal.h \

scripts/dtc/libfdt/fdt_wip.o: $(deps_scripts/dtc/libfdt/fdt_wip.o)

$(deps_scripts/dtc/libfdt/fdt_wip.o):
