.syntax unified
.global _start
.section .text ,"awx"
_start:
    SUB	R2, R2, #396
    LDR R0, [R2,#1023]
    STR R0, [R2,#-1023]
    LDR R0, [R2,#512]!
    LDR R1, [R2], #512
    LDR R2, [R1], R3
    LDR R2, [R1,R3]!
    MOV R7, #1
    SVC 0