savedcmd_crypto/cbc.mod := printf '%s\n'   cbc.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/cbc.mod
