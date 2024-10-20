savedcmd_scripts/dtc/checks.o := clang -Wp,-MMD,scripts/dtc/.checks.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include -I ./scripts/dtc/libfdt -DNO_YAML  -c -o scripts/dtc/checks.o scripts/dtc/checks.c

source_scripts/dtc/checks.o := scripts/dtc/checks.c

deps_scripts/dtc/checks.o := \
  scripts/dtc/dtc.h \
  scripts/dtc/libfdt/libfdt_env.h \
  scripts/dtc/libfdt/fdt.h \
  scripts/dtc/util.h \
  scripts/dtc/srcpos.h \

scripts/dtc/checks.o: $(deps_scripts/dtc/checks.o)

$(deps_scripts/dtc/checks.o):
