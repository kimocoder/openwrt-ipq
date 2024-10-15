savedcmd_crypto/arc4.mod := printf '%s\n'   arc4.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/arc4.mod
