.section .text.entry

.extern test
.global trap_s
.extern handler_s
.equ reg_size, 0x8
.align 2

trap_s:
	# save caller-saved registers and sepc
	addi sp, sp, -136
	sd ra, 120(sp)
	sd x5, 112(sp)
	sd x6, 104(sp)
	sd x7, 96(sp)
	sd x10, 88(sp)
	sd x11, 80(sp)
	sd x12, 72(sp)
	sd x13, 64(sp)
	sd x14, 56(sp)
	sd x15, 48(sp)
	sd x16, 40(sp)
	sd x17, 32(sp)
	sd x28, 24(sp)
	sd x29, 16(sp)
	sd x30, 8(sp)
	sd x31, 0(sp)
	
	csrr x5, sepc
	sd x5, 128(sp)

	# call handler_s(scause)
	csrr x10, scause
	call x1, handler_s

	# load sepc and caller-saved registers
	ld x5, 128(sp)
	csrw sepc, x5
	ld x31, 0(sp)
	ld x30, 8(sp)
	ld x29, 16(sp)
	ld x28, 24(sp)
	ld x17, 32(sp)
	ld x16, 40(sp)
	ld x15, 48(sp)
	ld x14, 56(sp)
	ld x13, 64(sp)
	ld x12, 72(sp)
	ld x11, 80(sp)
	ld x10, 88(sp)
	ld x7, 96(sp)
	ld x6, 104(sp)
	ld x5, 112(sp)
	ld ra, 120(sp)
	addi sp, sp, 136

	sret


.globl __switch_to
__switch_to:
	li    a4,  40 
	add   a3, a0, a4
	add   a4, a1, a4
	# Save context into prev->thread
	# your code
	sd ra, 0(a3)
	sd sp, 8(a3)
	sd s0, 16(a3)
	sd s1, 24(a3)
	sd s2, 32(a3)
	sd s3, 40(a3)
	sd s4, 48(a3)
	sd s5, 56(a3)
	sd s6, 64(a3)
	sd s7, 72(a3)
	sd s8, 80(a3)
	sd s9, 88(a3)
	sd s10, 96(a3)
	sd s11, 104(a3)

	# Restore context from next->thread
	# your code
	ld ra, 0(a4)
	ld sp, 8(a4)
	ld s0, 16(a4)
	ld s1, 24(a4)
	ld s2, 32(a4)
	ld s3, 40(a4)
	ld s4, 48(a4)
	ld s5, 56(a4)
	ld s6, 64(a4)
	ld s7, 72(a4)
	ld s8, 80(a4)
	ld s9, 88(a4)
	ld s10, 96(a4)
	ld s11, 104(a4)

	# return to ra
	ret


.globl __init_sepc
__init_sepc:
	la  t0, test
	csrw sepc, t0
	sret
