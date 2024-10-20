savedcmd_crypto/sha1_generic.mod := printf '%s\n'   sha1_generic.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/sha1_generic.mod
