/* link.ld */
ENTRY(_start)

SECTIONS
{
    .text : {
        *(.text)
    }
    /DISCARD/ : { *(*) }
}