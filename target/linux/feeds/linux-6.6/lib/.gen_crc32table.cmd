savedcmd_lib/gen_crc32table := gcc -Wp,-MMD,lib/.gen_crc32table.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -L/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/lib -o lib/gen_crc32table lib/gen_crc32table.c   

source_lib/gen_crc32table := lib/gen_crc32table.c

deps_lib/gen_crc32table := \
  lib/../include/linux/crc32poly.h \
  lib/crc32defs.h \
    $(wildcard include/config/CRC32_SLICEBY8) \
    $(wildcard include/config/CRC32_SLICEBY4) \
    $(wildcard include/config/CRC32_SARWATE) \
    $(wildcard include/config/CRC32_BIT) \
    $(wildcard include/config/64BIT) \

lib/gen_crc32table: $(deps_lib/gen_crc32table)

$(deps_lib/gen_crc32table):
