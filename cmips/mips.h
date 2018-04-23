#ifndef MIPS_H
#define MIPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#define UARTBASE (0x140003f8)
#define UARTSIZE (20)
#define POWERBASE (0x1fbf0004)
#define POWERSIZE (4)

typedef struct _Mips Mips;

int     mips_is_vm_shutdown             (Mips *emu);
Mips   *mips_new                        (uint32_t physMemSize);
void    mips_clear_external_interrupt   (Mips *emu, unsigned intNum);
void    mips_free                       (Mips * mips);
void    mips_step                       (Mips * emu);
void    mips_trigger_external_interrupt (Mips *emu, unsigned intNum);

int     mips_load_srec_from_file        (Mips *emu, char *fname);
int     mips_load_srec_from_string      (Mips *emu, char *srec);

uint8_t mips_uart_readb        (Mips * emu, uint32_t offset);
void    mips_uart_receive_char (Mips * emu, uint8_t c);
void    mips_uart_reset        (Mips * emu);
void    mips_uart_writeb       (Mips * emu, uint32_t offset, uint8_t v);

#ifdef __cplusplus
}
#endif
#endif
