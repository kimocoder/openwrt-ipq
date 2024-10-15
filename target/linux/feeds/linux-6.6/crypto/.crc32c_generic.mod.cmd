savedcmd_crypto/crc32c_generic.mod := printf '%s\n'   crc32c_generic.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/crc32c_generic.mod
