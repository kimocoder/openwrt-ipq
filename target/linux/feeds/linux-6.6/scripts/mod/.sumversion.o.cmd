savedcmd_scripts/mod/sumversion.o := clang -Wp,-MMD,scripts/mod/.sumversion.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -c -o scripts/mod/sumversion.o scripts/mod/sumversion.c

source_scripts/mod/sumversion.o := scripts/mod/sumversion.c

deps_scripts/mod/sumversion.o := \
  scripts/mod/modpost.h \
  /home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include/elf.h \
  scripts/mod/list.h \
  scripts/mod/elfconfig.h \

scripts/mod/sumversion.o: $(deps_scripts/mod/sumversion.o)

$(deps_scripts/mod/sumversion.o):
