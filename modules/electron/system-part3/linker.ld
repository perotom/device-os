INCLUDE platform_ram.ld

reset_handler = main;
ENTRY( reset_handler );


MEMORY
{
    DCT1_FLASH                 (rx)  : ORIGIN = 0x08004000,     LENGTH = 16K
    DCT2_FLASH                 (rx)  : ORIGIN = 0x08008000,     LENGTH = 16K
    APP_FLASH                  (rx)  : ORIGIN = 0x08040000,     LENGTH = 128K

    /* The SRAM area is offset to include the stack and static ram allocated at the end of the SRAM area.
       This is done so that the build outputs meaningful values for BSS and Data section sizes.
    */
    SRAM      (rwx) : ORIGIN = _system_part2_static_ram_start, LENGTH = _system_part2_static_ram_size + _stack_size

    INCLUDE backup_ram_memory.ld
}


/* Define the static locations of the part1 module so the dynamic invoke functions have the base address of each library table. */
INCLUDE module_system_part1_export.ld
INCLUDE module_system_part3_export.ld

/* Definitions for the stack size */
INCLUDE module_system_part2_export.ld

/* Definitions for the static parts of the user module */
INCLUDE module_user_export.ld

INCLUDE ../../shared/stm32f2xx/part2.ld
