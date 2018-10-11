# Author: Zach Fukuhara
# Email: zfukuhara@sandiego.edu

.text	# indicates that what follows is code, not data
.globl rotate  # declare global name called "rotate"

rotate:
	# The following lines set up the stack
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	(%rdx) # Push z value onto the stack
	pushq	(%rdi) # Push x value onto the stack
	pushq	(%rsi) # Push y value onto the stack
	popq	(%rdi) # Pop y value into x
	popq	(%rdx) # Pop x value into z
	popq	(%rsi) # Pop z value into y
	# Do NOT modify anything below here
	popq	%rbp
	retq
