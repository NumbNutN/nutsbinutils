.syntax unified

.section .text
str:
.string "Hello World\n"
.align 3
.global _start
_start:
	MOV R0, #1
	LDR R1, =str
	MOV R2, 13
	MOV R7, #4
	SVC 0	

	MOV R7, #1
	MOV R0, #0
	SVC 0
