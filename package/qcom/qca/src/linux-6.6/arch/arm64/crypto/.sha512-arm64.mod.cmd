savedcmd_arch/arm64/crypto/sha512-arm64.mod := printf '%s\n'   sha512-glue.o sha512-core.o | awk '!x[$$0]++ { print("arch/arm64/crypto/"$$0) }' > arch/arm64/crypto/sha512-arm64.mod
