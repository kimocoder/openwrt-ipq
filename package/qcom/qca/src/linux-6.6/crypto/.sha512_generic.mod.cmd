savedcmd_crypto/sha512_generic.mod := printf '%s\n'   sha512_generic.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/sha512_generic.mod
