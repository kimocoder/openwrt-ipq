savedcmd_crypto/lzo-rle.mod := printf '%s\n'   lzo-rle.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/lzo-rle.mod
