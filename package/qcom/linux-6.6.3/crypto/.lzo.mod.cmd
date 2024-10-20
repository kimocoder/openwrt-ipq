savedcmd_crypto/lzo.mod := printf '%s\n'   lzo.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/lzo.mod
