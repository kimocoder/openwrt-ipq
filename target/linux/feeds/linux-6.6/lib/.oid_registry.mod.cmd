savedcmd_lib/oid_registry.mod := printf '%s\n'   oid_registry.o | awk '!x[$$0]++ { print("lib/"$$0) }' > lib/oid_registry.mod
