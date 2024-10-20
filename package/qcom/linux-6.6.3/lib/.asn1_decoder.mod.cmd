savedcmd_lib/asn1_decoder.mod := printf '%s\n'   asn1_decoder.o | awk '!x[$$0]++ { print("lib/"$$0) }' > lib/asn1_decoder.mod
