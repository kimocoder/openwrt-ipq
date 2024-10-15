savedcmd_scripts/kallsyms := clang -Wp,-MMD,scripts/.kallsyms.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -L/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/lib -o scripts/kallsyms scripts/kallsyms.c   

source_scripts/kallsyms := scripts/kallsyms.c

deps_scripts/kallsyms := \

scripts/kallsyms: $(deps_scripts/kallsyms)

$(deps_scripts/kallsyms):
