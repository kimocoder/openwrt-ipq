savedcmd_crypto/ecb.mod := printf '%s\n'   ecb.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/ecb.mod
