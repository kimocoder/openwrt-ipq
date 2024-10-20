	.arch armv8-a
	.file	"bounds.c"
// GNU C11 (OpenWrt GCC 13.3.0 r0+27426-6b5867093ab8) version 13.3.0 (aarch64-openwrt-linux-musl)
//	compiled by GNU C version 14.2.0, GMP version 6.3.0, MPFR version 4.2.1, MPC version 1.3.1, isl version none
// GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
// options passed: -mlittle-endian -mgeneral-regs-only -mabi=lp64 -mbranch-protection=none -O2 -std=gnu11 -fshort-wchar -funsigned-char -fno-common -fno-PIE -fno-strict-aliasing -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-delete-null-pointer-checks -fno-allow-store-data-races -fstack-protector -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-stack-clash-protection -falign-functions=4 -fstrict-flex-arrays=3 -fno-strict-overflow -fstack-check=no -fconserve-stack -fno-caller-saves
	.text
	.section	.text.startup,"ax",@progbits
	.align	2
	.global	main
	.type	main, %function
main:
// kernel/bounds.c:19: 	DEFINE(NR_PAGEFLAGS, __NR_PAGEFLAGS);
#APP
// 19 "kernel/bounds.c" 1
	
.ascii "->NR_PAGEFLAGS 22 __NR_PAGEFLAGS"	//
// 0 "" 2
// kernel/bounds.c:20: 	DEFINE(MAX_NR_ZONES, __MAX_NR_ZONES);
// 20 "kernel/bounds.c" 1
	
.ascii "->MAX_NR_ZONES 4 __MAX_NR_ZONES"	//
// 0 "" 2
// kernel/bounds.c:22: 	DEFINE(NR_CPUS_BITS, ilog2(CONFIG_NR_CPUS));
// 22 "kernel/bounds.c" 1
	
.ascii "->NR_CPUS_BITS 8 ilog2(CONFIG_NR_CPUS)"	//
// 0 "" 2
// kernel/bounds.c:24: 	DEFINE(SPINLOCK_SIZE, sizeof(spinlock_t));
// 24 "kernel/bounds.c" 1
	
.ascii "->SPINLOCK_SIZE 4 sizeof(spinlock_t)"	//
// 0 "" 2
// kernel/bounds.c:26: 	DEFINE(LRU_GEN_WIDTH, order_base_2(MAX_NR_GENS + 1));
// 26 "kernel/bounds.c" 1
	
.ascii "->LRU_GEN_WIDTH 3 order_base_2(MAX_NR_GENS + 1)"	//
// 0 "" 2
// kernel/bounds.c:27: 	DEFINE(__LRU_REFS_WIDTH, MAX_NR_TIERS - 2);
// 27 "kernel/bounds.c" 1
	
.ascii "->__LRU_REFS_WIDTH 2 MAX_NR_TIERS - 2"	//
// 0 "" 2
// kernel/bounds.c:35: }
#NO_APP
	mov	w0, 0	//,
	ret	
	.size	main, .-main
	.ident	"GCC: (OpenWrt GCC 13.3.0 r0+27426-6b5867093ab8) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
