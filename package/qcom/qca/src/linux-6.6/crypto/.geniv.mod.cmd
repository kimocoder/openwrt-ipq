savedcmd_crypto/geniv.mod := printf '%s\n'   geniv.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/geniv.mod
