savedcmd_scripts/unifdef := gcc -Wp,-MMD,scripts/.unifdef.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -L/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/lib -o scripts/unifdef scripts/unifdef.c   

source_scripts/unifdef := scripts/unifdef.c

deps_scripts/unifdef := \

scripts/unifdef: $(deps_scripts/unifdef)

$(deps_scripts/unifdef):
