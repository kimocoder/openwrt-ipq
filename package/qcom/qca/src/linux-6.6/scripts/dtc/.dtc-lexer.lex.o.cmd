savedcmd_scripts/dtc/dtc-lexer.lex.o := clang -Wp,-MMD,scripts/dtc/.dtc-lexer.lex.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11  -O2 -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include  -Wall -Wmissing-prototypes -Wstrict-prototypes -I/home/kimocoder/Desktop/QSDK/qsdk/staging_dir/host/include -I ./scripts/dtc/libfdt -DNO_YAML -I ./scripts/dtc -c -o scripts/dtc/dtc-lexer.lex.o scripts/dtc/dtc-lexer.lex.c

source_scripts/dtc/dtc-lexer.lex.o := scripts/dtc/dtc-lexer.lex.c

deps_scripts/dtc/dtc-lexer.lex.o := \
  scripts/dtc/dtc.h \
  scripts/dtc/libfdt/libfdt_env.h \
  scripts/dtc/libfdt/fdt.h \
  scripts/dtc/util.h \
  scripts/dtc/srcpos.h \
  scripts/dtc/dtc-parser.tab.h \

scripts/dtc/dtc-lexer.lex.o: $(deps_scripts/dtc/dtc-lexer.lex.o)

$(deps_scripts/dtc/dtc-lexer.lex.o):
