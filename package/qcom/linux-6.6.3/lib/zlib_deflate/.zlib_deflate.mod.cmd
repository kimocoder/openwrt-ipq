savedcmd_lib/zlib_deflate/zlib_deflate.mod := printf '%s\n'   deflate.o deftree.o deflate_syms.o | awk '!x[$$0]++ { print("lib/zlib_deflate/"$$0) }' > lib/zlib_deflate/zlib_deflate.mod
