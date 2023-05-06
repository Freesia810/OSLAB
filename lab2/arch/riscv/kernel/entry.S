.section .text.entry

.global trap_s
.extern handler_s
.equ reg_size, 0x8
.align 2

trap_s:
	# save caller-saved registers and sepc
	csrr x5, sepc
	addi sp, sp, -128
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

	# call handler_s(scause)
	csrr x10, scause
	call x1, handler_s
	# load sepc and caller-saved registers
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
	addi sp, sp, 128
	csrw sepc, x5
	sret
