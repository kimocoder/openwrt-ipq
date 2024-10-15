savedcmd_lib/xxhash.mod := printf '%s\n'   xxhash.o | awk '!x[$$0]++ { print("lib/"$$0) }' > lib/xxhash.mod
