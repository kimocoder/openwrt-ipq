	.arch armv8-a
	.file	"asm-offsets.c"
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
// arch/arm64/kernel/asm-offsets.c:31:   DEFINE(TSK_ACTIVE_MM,		offsetof(struct task_struct, active_mm));
#APP
// 31 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_ACTIVE_MM 976 offsetof(struct task_struct, active_mm)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:32:   BLANK();
// 32 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:33:   DEFINE(TSK_TI_CPU,		offsetof(struct task_struct, thread_info.cpu));
// 33 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_TI_CPU 24 offsetof(struct task_struct, thread_info.cpu)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:34:   DEFINE(TSK_TI_FLAGS,		offsetof(struct task_struct, thread_info.flags));
// 34 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_TI_FLAGS 0 offsetof(struct task_struct, thread_info.flags)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:35:   DEFINE(TSK_TI_PREEMPT,	offsetof(struct task_struct, thread_info.preempt_count));
// 35 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_TI_PREEMPT 16 offsetof(struct task_struct, thread_info.preempt_count)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:37:   DEFINE(TSK_TI_TTBR0,		offsetof(struct task_struct, thread_info.ttbr0));
// 37 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_TI_TTBR0 8 offsetof(struct task_struct, thread_info.ttbr0)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:43:   DEFINE(TSK_STACK,		offsetof(struct task_struct, stack));
// 43 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_STACK 40 offsetof(struct task_struct, stack)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:45:   DEFINE(TSK_STACK_CANARY,	offsetof(struct task_struct, stack_canary));
// 45 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_STACK_CANARY 1096 offsetof(struct task_struct, stack_canary)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:47:   BLANK();
// 47 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:48:   DEFINE(THREAD_CPU_CONTEXT,	offsetof(struct task_struct, thread.cpu_context));
// 48 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->THREAD_CPU_CONTEXT 2240 offsetof(struct task_struct, thread.cpu_context)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:49:   DEFINE(THREAD_SCTLR_USER,	offsetof(struct task_struct, thread.sctlr_user));
// 49 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->THREAD_SCTLR_USER 2952 offsetof(struct task_struct, thread.sctlr_user)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:59:   BLANK();
// 59 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:60:   DEFINE(S_X0,			offsetof(struct pt_regs, regs[0]));
// 60 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X0 0 offsetof(struct pt_regs, regs[0])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:61:   DEFINE(S_X2,			offsetof(struct pt_regs, regs[2]));
// 61 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X2 16 offsetof(struct pt_regs, regs[2])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:62:   DEFINE(S_X4,			offsetof(struct pt_regs, regs[4]));
// 62 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X4 32 offsetof(struct pt_regs, regs[4])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:63:   DEFINE(S_X6,			offsetof(struct pt_regs, regs[6]));
// 63 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X6 48 offsetof(struct pt_regs, regs[6])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:64:   DEFINE(S_X8,			offsetof(struct pt_regs, regs[8]));
// 64 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X8 64 offsetof(struct pt_regs, regs[8])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:65:   DEFINE(S_X10,			offsetof(struct pt_regs, regs[10]));
// 65 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X10 80 offsetof(struct pt_regs, regs[10])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:66:   DEFINE(S_X12,			offsetof(struct pt_regs, regs[12]));
// 66 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X12 96 offsetof(struct pt_regs, regs[12])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:67:   DEFINE(S_X14,			offsetof(struct pt_regs, regs[14]));
// 67 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X14 112 offsetof(struct pt_regs, regs[14])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:68:   DEFINE(S_X16,			offsetof(struct pt_regs, regs[16]));
// 68 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X16 128 offsetof(struct pt_regs, regs[16])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:69:   DEFINE(S_X18,			offsetof(struct pt_regs, regs[18]));
// 69 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X18 144 offsetof(struct pt_regs, regs[18])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:70:   DEFINE(S_X20,			offsetof(struct pt_regs, regs[20]));
// 70 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X20 160 offsetof(struct pt_regs, regs[20])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:71:   DEFINE(S_X22,			offsetof(struct pt_regs, regs[22]));
// 71 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X22 176 offsetof(struct pt_regs, regs[22])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:72:   DEFINE(S_X24,			offsetof(struct pt_regs, regs[24]));
// 72 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X24 192 offsetof(struct pt_regs, regs[24])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:73:   DEFINE(S_X26,			offsetof(struct pt_regs, regs[26]));
// 73 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X26 208 offsetof(struct pt_regs, regs[26])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:74:   DEFINE(S_X28,			offsetof(struct pt_regs, regs[28]));
// 74 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X28 224 offsetof(struct pt_regs, regs[28])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:75:   DEFINE(S_FP,			offsetof(struct pt_regs, regs[29]));
// 75 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_FP 232 offsetof(struct pt_regs, regs[29])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:76:   DEFINE(S_LR,			offsetof(struct pt_regs, regs[30]));
// 76 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_LR 240 offsetof(struct pt_regs, regs[30])"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:77:   DEFINE(S_SP,			offsetof(struct pt_regs, sp));
// 77 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_SP 248 offsetof(struct pt_regs, sp)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:78:   DEFINE(S_PSTATE,		offsetof(struct pt_regs, pstate));
// 78 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_PSTATE 264 offsetof(struct pt_regs, pstate)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:79:   DEFINE(S_PC,			offsetof(struct pt_regs, pc));
// 79 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_PC 256 offsetof(struct pt_regs, pc)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:80:   DEFINE(S_SYSCALLNO,		offsetof(struct pt_regs, syscallno));
// 80 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_SYSCALLNO 280 offsetof(struct pt_regs, syscallno)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:81:   DEFINE(S_SDEI_TTBR1,		offsetof(struct pt_regs, sdei_ttbr1));
// 81 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_SDEI_TTBR1 288 offsetof(struct pt_regs, sdei_ttbr1)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:82:   DEFINE(S_PMR_SAVE,		offsetof(struct pt_regs, pmr_save));
// 82 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_PMR_SAVE 296 offsetof(struct pt_regs, pmr_save)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:83:   DEFINE(S_STACKFRAME,		offsetof(struct pt_regs, stackframe));
// 83 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_STACKFRAME 304 offsetof(struct pt_regs, stackframe)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:84:   DEFINE(PT_REGS_SIZE,		sizeof(struct pt_regs));
// 84 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->PT_REGS_SIZE 336 sizeof(struct pt_regs)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:85:   BLANK();
// 85 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:103:   DEFINE(COMPAT_SIGFRAME_REGS_OFFSET,		offsetof(struct compat_sigframe, uc.uc_mcontext.arm_r0));
// 103 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->COMPAT_SIGFRAME_REGS_OFFSET 32 offsetof(struct compat_sigframe, uc.uc_mcontext.arm_r0)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:104:   DEFINE(COMPAT_RT_SIGFRAME_REGS_OFFSET,	offsetof(struct compat_rt_sigframe, sig.uc.uc_mcontext.arm_r0));
// 104 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->COMPAT_RT_SIGFRAME_REGS_OFFSET 160 offsetof(struct compat_rt_sigframe, sig.uc.uc_mcontext.arm_r0)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:105:   BLANK();
// 105 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:107:   DEFINE(MM_CONTEXT_ID,		offsetof(struct mm_struct, context.id.counter));
// 107 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->MM_CONTEXT_ID 880 offsetof(struct mm_struct, context.id.counter)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:108:   BLANK();
// 108 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:109:   DEFINE(VMA_VM_MM,		offsetof(struct vm_area_struct, vm_mm));
// 109 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->VMA_VM_MM 16 offsetof(struct vm_area_struct, vm_mm)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:110:   DEFINE(VMA_VM_FLAGS,		offsetof(struct vm_area_struct, vm_flags));
// 110 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->VMA_VM_FLAGS 32 offsetof(struct vm_area_struct, vm_flags)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:111:   BLANK();
// 111 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:112:   DEFINE(VM_EXEC,	       	VM_EXEC);
// 112 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->VM_EXEC 4 VM_EXEC"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:113:   BLANK();
// 113 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:114:   DEFINE(PAGE_SZ,	       	PAGE_SIZE);
// 114 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->PAGE_SZ 4096 PAGE_SIZE"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:115:   BLANK();
// 115 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:116:   DEFINE(DMA_TO_DEVICE,		DMA_TO_DEVICE);
// 116 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->DMA_TO_DEVICE 1 DMA_TO_DEVICE"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:117:   DEFINE(DMA_FROM_DEVICE,	DMA_FROM_DEVICE);
// 117 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->DMA_FROM_DEVICE 2 DMA_FROM_DEVICE"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:118:   BLANK();
// 118 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:119:   DEFINE(PREEMPT_DISABLE_OFFSET, PREEMPT_DISABLE_OFFSET);
// 119 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->PREEMPT_DISABLE_OFFSET 0 PREEMPT_DISABLE_OFFSET"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:120:   DEFINE(SOFTIRQ_SHIFT, SOFTIRQ_SHIFT);
// 120 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->SOFTIRQ_SHIFT 8 SOFTIRQ_SHIFT"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:121:   DEFINE(IRQ_CPUSTAT_SOFTIRQ_PENDING, offsetof(irq_cpustat_t, __softirq_pending));
// 121 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->IRQ_CPUSTAT_SOFTIRQ_PENDING 0 offsetof(irq_cpustat_t, __softirq_pending)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:122:   BLANK();
// 122 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:123:   DEFINE(CPU_BOOT_TASK,		offsetof(struct secondary_data, task));
// 123 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->CPU_BOOT_TASK 0 offsetof(struct secondary_data, task)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:124:   BLANK();
// 124 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:125:   DEFINE(FTR_OVR_VAL_OFFSET,	offsetof(struct arm64_ftr_override, val));
// 125 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->FTR_OVR_VAL_OFFSET 0 offsetof(struct arm64_ftr_override, val)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:126:   DEFINE(FTR_OVR_MASK_OFFSET,	offsetof(struct arm64_ftr_override, mask));
// 126 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->FTR_OVR_MASK_OFFSET 8 offsetof(struct arm64_ftr_override, mask)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:127:   BLANK();
// 127 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:158:   DEFINE(ARM_SMCCC_RES_X0_OFFS,		offsetof(struct arm_smccc_res, a0));
// 158 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_RES_X0_OFFS 0 offsetof(struct arm_smccc_res, a0)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:159:   DEFINE(ARM_SMCCC_RES_X2_OFFS,		offsetof(struct arm_smccc_res, a2));
// 159 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_RES_X2_OFFS 16 offsetof(struct arm_smccc_res, a2)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:160:   DEFINE(ARM_SMCCC_QUIRK_ID_OFFS,	offsetof(struct arm_smccc_quirk, id));
// 160 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_QUIRK_ID_OFFS 0 offsetof(struct arm_smccc_quirk, id)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:161:   DEFINE(ARM_SMCCC_QUIRK_STATE_OFFS,	offsetof(struct arm_smccc_quirk, state));
// 161 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_QUIRK_STATE_OFFS 8 offsetof(struct arm_smccc_quirk, state)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:162:   DEFINE(ARM_SMCCC_1_2_REGS_X0_OFFS,	offsetof(struct arm_smccc_1_2_regs, a0));
// 162 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X0_OFFS 0 offsetof(struct arm_smccc_1_2_regs, a0)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:163:   DEFINE(ARM_SMCCC_1_2_REGS_X2_OFFS,	offsetof(struct arm_smccc_1_2_regs, a2));
// 163 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X2_OFFS 16 offsetof(struct arm_smccc_1_2_regs, a2)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:164:   DEFINE(ARM_SMCCC_1_2_REGS_X4_OFFS,	offsetof(struct arm_smccc_1_2_regs, a4));
// 164 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X4_OFFS 32 offsetof(struct arm_smccc_1_2_regs, a4)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:165:   DEFINE(ARM_SMCCC_1_2_REGS_X6_OFFS,	offsetof(struct arm_smccc_1_2_regs, a6));
// 165 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X6_OFFS 48 offsetof(struct arm_smccc_1_2_regs, a6)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:166:   DEFINE(ARM_SMCCC_1_2_REGS_X8_OFFS,	offsetof(struct arm_smccc_1_2_regs, a8));
// 166 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X8_OFFS 64 offsetof(struct arm_smccc_1_2_regs, a8)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:167:   DEFINE(ARM_SMCCC_1_2_REGS_X10_OFFS,	offsetof(struct arm_smccc_1_2_regs, a10));
// 167 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X10_OFFS 80 offsetof(struct arm_smccc_1_2_regs, a10)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:168:   DEFINE(ARM_SMCCC_1_2_REGS_X12_OFFS,	offsetof(struct arm_smccc_1_2_regs, a12));
// 168 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X12_OFFS 96 offsetof(struct arm_smccc_1_2_regs, a12)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:169:   DEFINE(ARM_SMCCC_1_2_REGS_X14_OFFS,	offsetof(struct arm_smccc_1_2_regs, a14));
// 169 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X14_OFFS 112 offsetof(struct arm_smccc_1_2_regs, a14)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:170:   DEFINE(ARM_SMCCC_1_2_REGS_X16_OFFS,	offsetof(struct arm_smccc_1_2_regs, a16));
// 170 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X16_OFFS 128 offsetof(struct arm_smccc_1_2_regs, a16)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:171:   BLANK();
// 171 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:172:   DEFINE(HIBERN_PBE_ORIG,	offsetof(struct pbe, orig_address));
// 172 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->HIBERN_PBE_ORIG 8 offsetof(struct pbe, orig_address)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:173:   DEFINE(HIBERN_PBE_ADDR,	offsetof(struct pbe, address));
// 173 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->HIBERN_PBE_ADDR 0 offsetof(struct pbe, address)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:174:   DEFINE(HIBERN_PBE_NEXT,	offsetof(struct pbe, next));
// 174 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->HIBERN_PBE_NEXT 16 offsetof(struct pbe, next)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:175:   DEFINE(ARM64_FTR_SYSVAL,	offsetof(struct arm64_ftr_reg, sys_val));
// 175 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM64_FTR_SYSVAL 24 offsetof(struct arm64_ftr_reg, sys_val)"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:176:   BLANK();
// 176 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:178:   DEFINE(TRAMP_VALIAS,		TRAMP_VALIAS);
// 178 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TRAMP_VALIAS -8625614848 TRAMP_VALIAS"	//
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:204:   BLANK();
// 204 "arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// arch/arm64/kernel/asm-offsets.c:221: }
#NO_APP
	mov	w0, 0	//,
	ret	
	.size	main, .-main
	.ident	"GCC: (OpenWrt GCC 13.3.0 r0+27426-6b5867093ab8) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
