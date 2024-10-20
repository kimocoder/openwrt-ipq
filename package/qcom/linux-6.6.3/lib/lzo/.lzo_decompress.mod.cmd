savedcmd_lib/lzo/lzo_decompress.mod := printf '%s\n'   lzo1x_decompress_safe.o | awk '!x[$$0]++ { print("lib/lzo/"$$0) }' > lib/lzo/lzo_decompress.mod
