savedcmd_lib/lzo/lzo_compress.mod := printf '%s\n'   lzo1x_compress.o | awk '!x[$$0]++ { print("lib/lzo/"$$0) }' > lib/lzo/lzo_compress.mod
