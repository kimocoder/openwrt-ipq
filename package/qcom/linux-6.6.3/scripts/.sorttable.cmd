savedcmd_scripts/sorttable := clang -Wp,-MMD,scripts/.sorttable.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include -I./tools/include -L/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/lib -o scripts/sorttable scripts/sorttable.c   -lpthread

source_scripts/sorttable := scripts/sorttable.c

deps_scripts/sorttable := \
  /home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include/elf.h \
  tools/include/tools/be_byteshift.h \
  tools/include/tools/le_byteshift.h \
  scripts/sorttable.h \

scripts/sorttable: $(deps_scripts/sorttable)

$(deps_scripts/sorttable):
