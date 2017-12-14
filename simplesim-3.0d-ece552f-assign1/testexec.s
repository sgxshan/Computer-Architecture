	.file	1 "testexec.c"

 # GNU C 2.7.2.3 [AL 1.1, MM 40, tma 0.1] SimpleScalar running sstrix compiled by GNU C

 # Cc1 defaults:
 # -mgas -mgpOPT

 # Cc1 arguments (-G value = 8, Cpu = default, ISA = 1):
 # -quiet -dumpbase -O1 -o

gcc2_compiled.:
__gnu_compiled_c:
	.rdata
	.align	2
$LC0:
	.ascii	"Sum = %d\n\000"
	.text
	.align	2
	.globl	main

	.extern	stdin, 4
	.extern	stdout, 4

	.text

	.loc	1 4
	.ent	main
main:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	sw	$16,16($sp)
	move	$16,$5
	sw	$31,24($sp)
	sw	$17,20($sp)
	jal	__main
	.set	noreorder
	lw	$4,4($16)
	.set	reorder
	move	$17,$0
	jal	atoi
	blez	$2,$L15
$L17:
	addu	$17,$17,$2
	addu	$17,$17,1
	subu	$2,$2,1
	bgtz	$2,$L17
$L15:
	la	$4,$LC0
	move	$5,$17
	jal	printf
	move	$2,$0
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	addu	$sp,$sp,32
	j	$31
	.end	main
