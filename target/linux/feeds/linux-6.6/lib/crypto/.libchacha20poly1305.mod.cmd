savedcmd_lib/crypto/libchacha20poly1305.mod := printf '%s\n'   chacha20poly1305.o chacha20poly1305-selftest.o | awk '!x[$$0]++ { print("lib/crypto/"$$0) }' > lib/crypto/libchacha20poly1305.mod
