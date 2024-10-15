savedcmd_crypto/zstd.mod := printf '%s\n'   zstd.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/zstd.mod
