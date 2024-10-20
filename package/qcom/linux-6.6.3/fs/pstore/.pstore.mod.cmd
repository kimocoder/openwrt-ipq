savedcmd_fs/pstore/pstore.mod := printf '%s\n'   inode.o platform.o | awk '!x[$$0]++ { print("fs/pstore/"$$0) }' > fs/pstore/pstore.mod
