savedcmd_lib/crc16.mod := printf '%s\n'   crc16.o | awk '!x[$$0]++ { print("lib/"$$0) }' > lib/crc16.mod
