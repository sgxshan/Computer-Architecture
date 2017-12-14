	.file	1 "mbq1.c"

 # GNU C 2.7.2.3 [AL 1.1, MM 40, tma 0.1] SimpleScalar running sstrix compiled by GNU C

 # Cc1 defaults:
 # -mgas -mgpOPT

 # Cc1 arguments (-G value = 8, Cpu = default, ISA = 1):
 # -quiet -dumpbase -O1 -o

gcc2_compiled.:
__gnu_compiled_c:
	.sdata
	.align	2
d:
	.word	0
	.rdata
	.align	2
$LC0:
	.ascii	"Usage: %s <count>\n\000"
	.align	2
$LC1:
	.ascii	"%d, %d, %d, %d\000"
	.text
	.align	2
	.globl	main

	.extern	stdin, 4
	.extern	stdout, 4

	.text

	.loc	1 3
	.ent	main
main:
	.frame	$sp,48,$31		# vars= 0, regs= 6/0, args= 24, extra= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,48
	sw	$16,24($sp)
	move	$16,$4
	sw	$20,40($sp)
	move	$20,$5
	sw	$31,44($sp)
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	jal	__main
	move	$17,$0
	move	$18,$0
	move	$19,$0
	li	$2,0x00000002		# 2
	beq	$16,$2,$L14
	.set	noreorder
	lw	$5,0($20)
	.set	reorder
	la	$4,$LC0
	jal	printf
	li	$4,0x00000005		# 5
	jal	exit
$L14:
	.set	noreorder
	lw	$4,4($20)
	.set	reorder
	jal	atoi
	bltz	$2,$L16
$L18:
	addu	$17,$17,$2
	addu	$18,$18,2
	addu	$17,$17,$18
	addu	$19,$19,1
	addu	$18,$18,$17
	subu	$2,$2,1
	bgez	$2,$L18
$L16:
	sw	$2,16($sp)
	la	$4,$LC1
	move	$5,$17
	move	$6,$18
	move	$7,$19
	jal	printf
	move	$2,$0
	lw	$31,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	addu	$sp,$sp,48
	j	$31
	.end	main
