;memory leaking, now cannot pass
.section .text
num:
.word 2
str1:
.string "the result of 1 plus 2 is "
.align 3
.global _start:

calculate:
	MOV R0, #1
	LDR R1, =num
	LDR R2,[R1]
	ADD R4, R0, R2
    ADD R4, R4, #48
    STR R4, [R1]

print:
    MOV R0, #1
	LDR R1, =str1
	MOV R2, #25
	MOV R7, #4
	SVC 0

    MOV R0, #1
    LDR R1, =num
    MOV R2, #1
    MOV R7, #4
    SVC 0

exit:
    MOV R0, #0
    MOV R7, #1
	SVC 0
