savedcmd_fs/nls/nls_base.mod := printf '%s\n'   nls_base.o | awk '!x[$$0]++ { print("fs/nls/"$$0) }' > fs/nls/nls_base.mod
