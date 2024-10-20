savedcmd_crypto/hmac.mod := printf '%s\n'   hmac.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/hmac.mod
