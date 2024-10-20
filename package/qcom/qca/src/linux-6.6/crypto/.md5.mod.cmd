savedcmd_crypto/md5.mod := printf '%s\n'   md5.o | awk '!x[$$0]++ { print("crypto/"$$0) }' > crypto/md5.mod
