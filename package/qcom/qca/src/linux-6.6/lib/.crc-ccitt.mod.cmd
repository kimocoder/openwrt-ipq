savedcmd_lib/crc-ccitt.mod := printf '%s\n'   crc-ccitt.o | awk '!x[$$0]++ { print("lib/"$$0) }' > lib/crc-ccitt.mod
