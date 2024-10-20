savedcmd_scripts/mod/file2alias.o := clang -Wp,-MMD,scripts/mod/.file2alias.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -c -o scripts/mod/file2alias.o scripts/mod/file2alias.c

source_scripts/mod/file2alias.o := scripts/mod/file2alias.c

deps_scripts/mod/file2alias.o := \
  scripts/mod/modpost.h \
  /home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include/elf.h \
  scripts/mod/list.h \
  scripts/mod/elfconfig.h \
  scripts/mod/devicetable-offsets.h \
  scripts/mod/../../include/linux/mod_devicetable.h \

scripts/mod/file2alias.o: $(deps_scripts/mod/file2alias.o)

$(deps_scripts/mod/file2alias.o):
