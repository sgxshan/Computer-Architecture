	.file	1 "mbq1.c"

 # GNU C 2.7.2.3 [AL 1.1, MM 40, tma 0.1] SimpleScalar running sstrix compiled by GNU C

 # Cc1 defaults:
 # -mgas -mgpOPT

 # Cc1 arguments (-G value = 8, Cpu = default, ISA = 1):
 # -quiet -dumpbase -O2 -o

gcc2_compiled.:
__gnu_compiled_c:
	.sdata
	.align	2
$LC0:
	.ascii	"%d\000"
	.text
	.align	2
	.globl	main

	.extern	stdin, 4
	.extern	stdout, 4

	.text

	.loc	1 4
	.ent	main
main:
	.frame	$sp,25600024,$31		# vars= 25600000, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	li	$8,0x0186a018		# 25600024
	subu	$sp,$sp,$8
	addu	$9,$8,$sp
	sw	$31,-8($9)
	jal	__main
	move	$6,$0
	move	$4,$0
	li	$5,0x00610000		# 6356992
	ori	$5,$5,0xa7ff
	addu	$3,$sp,16
$L17:
	.set	noreorder
	lw	$2,0($3)
	.set	reorder
	addu	$4,$4,50
	addu	$6,$6,$2
	addu	$3,$3,200
	slt	$2,$5,$4
	beq	$2,$0,$L17
	la	$4,$LC0
	addu	$5,$sp,16
	jal	printf
	move	$2,$0
	li	$8,0x0186a018	# 25600024
	addu	$9,$8,$sp
	lw	$31,-8($9)
	addu	$sp,$sp,$8
	j	$31
	.end	main
