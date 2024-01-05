.section .text
str1:
.string "Hello World"
.align 3
.global _start:

print:
	MOV R0, #1
	LDR R1, =str1
	MOV R2, #11
	MOV R7, #4
	SVC 0

exit:
	MOV R7, #1
	MOV R0, #0
	SVC 0