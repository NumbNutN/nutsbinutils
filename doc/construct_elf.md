#### 环境

```
gcc version 10.2.1
binutils version 2.41
ld version 2.35.2
```



##### 目前最小的汇编源文件

```assembly
;main_nocrt.s
.global _start
.section .text
str1:
.string "Hello \n"
_start:
    MOV R0, #1
    LDR R1, =str1
    MOV R2, #8
    MOV R7, #4
    SVC 0

    MOV R7, #1
    SVC 0
```



##### 去掉装载器不必要的节的链接脚本

```
/* link.ld */
ENTRY(_start)

SECTIONS
{
    .text : {
        *(.text)
    }
    /DISCARD/ : { *(*) }
}
```



##### 完整汇编链接流程

```bash
gcc -c -o main.o main_nocrt.s
ld -marmelf_linux_eabi -o a.out main.o
strip a.out

//or

gcc -nostartfiles -o a.out main_nocrt.s -T link.ld
strip a.out
```



##### 节信息

```
//readelf -S a.out
  
  There are 3 section headers, starting at offset 0x103c:

Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .text             PROGBITS        00000000 001000 000028 00  AX  0   0  4
  [ 2] .shstrtab         STRTAB          00000000 001028 000011 00      0   0  1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  y (purecode), p (processor specific)
```





**生成链接信息文件**

```
-Wl,-Map,xxx.map
```



