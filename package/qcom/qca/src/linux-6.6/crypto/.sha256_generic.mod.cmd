savedcmd_crypto/sha256_generic.mod := printf '%s\n'   sha256_generic.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/sha256_generic.mod
