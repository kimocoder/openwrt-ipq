savedcmd_security/keys/trusted-keys/trusted.mod := printf '%s\n'   trusted_core.o | awk '!x[$$0]++ { print("security/keys/trusted-keys/"$$0) }' > security/keys/trusted-keys/trusted.mod
