#ifndef INTERNAL_H
#define INTERNAL_H

typedef struct {
        uint32_t VPN2;
        uint8_t ASID;
        unsigned G:1;
        unsigned C0:3;
        unsigned C1:3;
        unsigned V0:1;
        unsigned V1:1;
        unsigned D0:1;
        unsigned D1:1;
        uint32_t PFN[2];
} TLB_entry;

typedef struct {
        TLB_entry entries[16];
        int exceptionWasNoMatch;
} TLB;

typedef struct {
        uint8_t LCR; /* Line Control, reset, character has 8 bits*/
        uint8_t LSR; /* Line Status register, Transmitter serial register empty and 
                        Transmitter buffer register empty*/
        uint8_t MSR; /* modem status register*/
        uint8_t IIR; /* Interrupt Identification, no interrupt*/
        uint8_t IER; /* Interrupt Enable*/
        uint8_t DLL;
        uint8_t DLH;
        uint8_t FCR; /* FIFO Control;*/
        uint8_t MCR; /* Modem Control*/
        uint8_t SCR; /* Modem Control*/

        /*NOTE do not change this size without changing the corresponding FIFO function*/
        uint8_t fifo[32];       /*ring buffer*/
        uint32_t fifoFirst;
        uint32_t fifoLast;
        uint32_t fifoCount;
} Uart;



typedef struct _Mips {
        uint32_t *mem;
        uint32_t pmemsz;
        uint32_t shutdown;
        uint32_t pc;
        uint32_t regs[32];
        uint32_t hi;
        uint32_t lo;
        uint32_t delaypc;
        uint8_t inDelaySlot;
        uint8_t exceptionOccured;

        int llbit;
        uint32_t CP0_Index;
        uint32_t CP0_EntryHi;
        uint32_t CP0_EntryLo0;
        uint32_t CP0_EntryLo1;
        uint32_t CP0_Context;
        uint32_t CP0_Wired;
        uint32_t CP0_Status;
        uint32_t CP0_Epc;
        uint32_t CP0_BadVAddr;
        uint32_t CP0_ErrorEpc;
        uint32_t CP0_Cause;
        uint32_t CP0_PageMask;

        uint32_t CP0_Count;
        uint32_t CP0_Compare;

        int waiting;

        Uart serial;

        TLB tlb;
} Mips;

#endif
