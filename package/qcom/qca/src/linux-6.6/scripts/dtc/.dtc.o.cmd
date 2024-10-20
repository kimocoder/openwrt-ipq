savedcmd_scripts/dtc/dtc.o := clang -Wp,-MMD,scripts/dtc/.dtc.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include -I ./scripts/dtc/libfdt -DNO_YAML  -c -o scripts/dtc/dtc.o scripts/dtc/dtc.c

source_scripts/dtc/dtc.o := scripts/dtc/dtc.c

deps_scripts/dtc/dtc.o := \
  scripts/dtc/dtc.h \
  scripts/dtc/libfdt/libfdt_env.h \
  scripts/dtc/libfdt/fdt.h \
  scripts/dtc/util.h \
  scripts/dtc/srcpos.h \

scripts/dtc/dtc.o: $(deps_scripts/dtc/dtc.o)

$(deps_scripts/dtc/dtc.o):
