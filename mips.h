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

Mips *mips_new(uint32_t physMemSize);
void mips_free(Mips * mips);
void mips_step(Mips * emu);
int  mips_isMipsShutdown(Mips *emu);
void mips_triggerExternalInterrupt(Mips *emu, unsigned intNum);
void mips_clearExternalInterrupt(Mips *emu, unsigned intNum);

int mips_loadSrecFromFile(Mips *emu, char *fname);
int mips_loadSrecFromString(Mips *emu, char *srec);

void uart_Reset(Mips * emu);

uint32_t uart_read(Mips * emu, uint32_t offset);
void uart_write(Mips * emu, uint32_t offset, uint32_t v);
uint8_t uart_readb(Mips * emu, uint32_t offset);
void uart_writeb(Mips * emu, uint32_t offset, uint8_t v);
void uart_RecieveChar(Mips * emu, uint8_t c);

#ifdef __cplusplus
}
#endif
#endif
