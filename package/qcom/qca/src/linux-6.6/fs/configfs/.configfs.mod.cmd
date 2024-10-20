savedcmd_fs/configfs/configfs.mod := printf '%s\n'   inode.o file.o dir.o symlink.o mount.o item.o | awk '!x[$$0]++ { print("fs/configfs/"$$0) }' > fs/configfs/configfs.mod
