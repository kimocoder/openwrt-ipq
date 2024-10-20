savedcmd_lib/crypto/libsha256.mod := printf '%s\n'   sha256.o | awk '!x[$$0]++ { print("lib/crypto/"$$0) }' > lib/crypto/libsha256.mod
