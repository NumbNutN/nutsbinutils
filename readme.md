#### The remote repository: [NumbNutN/nutsbinutils: self implementation binutils (github.com)](https://github.com/NumbNutN/nutsbinutils)



### How to build

#### build the assembler and linker

```
cd ./myas/build
make

//and you get ./myas/build/myas

cd ./myld/build
make

//and you get ./myld/build/myld

cd ./myloader
make

//and you get ./myloader/build/myloader
```





### How to use

#### write a assemble text file

```assembly
;hello.s
;remove the comment before give the file to assembler, the syntax analysis currently doesn't support comments
.section
str1:
.string "Hello World"
.align 3
.global _start:
	MOV R0, #1
	LDR R1, =str1
	MOV R2, #11
	MOV R7, #4
	SVC 0

	MOV R7, #1
	MOV R0, #0
	SVC 0
```



```
myas {OUTPUT_OBJECT_FILE} {INPUT_ASSEMBLE_FILE}
```



```
myld {OUTPUT_EXEC_FILE} {INPUT_OBJECT_FILE}
```



#### Assembler Syntax

```
/* Supportive Mnemonic */

MOV Rd, OPERAND2
ADD Rd, Rn, OPERAND2
SUB Rd, Rn, OPERAND2
LDR|STR Rd, [Rn, OFFSET]{!}
LDR|STR Rd, [Rn], OFFSET{!}
LDR Rd, =LABEL

/* Supportive Directive */
.section {STRING}
	create a new section. (now the default section permission is alloctable & executable)
	
.zero {DIGIT}
	fill the sequential memory with DIGIT bytes
	
.align {DIGIT}
	fill the sequential memory until the top reach an address that is a multiple of 2<<DIGIT
	
.word {DIGIT}
	a memory space with a size of 4 bytes will store the DIGIT with a format of complementary code will be inserted next to the previous address
	
.string {"LITERAL STRING"}
	the following space will store the string with a format of ASCII with a size of the number of characters plus 1. The last byte of the memory will store a '\0' in ASCII

{LABEL}:
	to bind a relative position from the current section to the symbol LABEL, which has a LOCAL attribute
	
.global {LABEL}:
	to bind a relative position from the current section to the symbol LABEL, which has a GLOBAL attribute (different from the syntax of GNU/AS)
```



### Design Concept

