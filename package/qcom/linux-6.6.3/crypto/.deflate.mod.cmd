savedcmd_crypto/deflate.mod := printf '%s\n'   deflate.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/deflate.mod
