savedcmd_lib/raid6/mktables := gcc -Wp,-MMD,lib/raid6/.mktables.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -L/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/lib -o lib/raid6/mktables lib/raid6/mktables.c   

source_lib/raid6/mktables := lib/raid6/mktables.c

deps_lib/raid6/mktables := \

lib/raid6/mktables: $(deps_lib/raid6/mktables)

$(deps_lib/raid6/mktables):
