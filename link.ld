ENTRY(Reset_Handler)

MEMORY {
    FLASH (rx) : ORIGIN = 0x10000000, LENGTH = 2048K
    RAM   (rwx): ORIGIN = 0x20000000, LENGTH = 512K
}

SECTIONS {
    .vectors ORIGIN(FLASH) : {
        KEEP(*(.vectors))
    } > FLASH

    .text : {
        _text_start = .;
        *(.text*)
        *(.rodata*)
        _text_end = .;
    } > FLASH

    .data : AT(_text_end) {
        __data_start = .;
        __data_load_ptr = LOADADDR(.data);
        *(.data*)
        __data_end = .;
    } > RAM

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > RAM

    .stack (NOLOAD): {
        . = ALIGN(8);
        _stack_top = . + 0x8000; /* 32KB stack */
    } > RAM
}