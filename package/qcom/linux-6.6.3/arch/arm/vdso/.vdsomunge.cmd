savedcmd_arch/arm64/kernel/vdso32/../../../arm/vdso/vdsomunge := clang -Wp,-MMD,arch/arm64/kernel/vdso32/../../../arm/vdso/.vdsomunge.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -L/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/lib -o arch/arm64/kernel/vdso32/../../../arm/vdso/vdsomunge arch/arm64/kernel/vdso32/../../../arm/vdso/vdsomunge.c   

source_arch/arm64/kernel/vdso32/../../../arm/vdso/vdsomunge := arch/arm64/kernel/vdso32/../../../arm/vdso/vdsomunge.c

deps_arch/arm64/kernel/vdso32/../../../arm/vdso/vdsomunge := \
  /home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include/elf.h \

arch/arm64/kernel/vdso32/../../../arm/vdso/vdsomunge: $(deps_arch/arm64/kernel/vdso32/../../../arm/vdso/vdsomunge)

$(deps_arch/arm64/kernel/vdso32/../../../arm/vdso/vdsomunge):
