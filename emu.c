#include "mips.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>

enum status_registers_bits {
      CP0St_IE   =  0,   CP0St_EXL  =  1,   CP0St_ERL  =  2,   CP0St_KSU  =  3,
      CP0St_UM   =  4,   CP0St_UX   =  5,   CP0St_SX   =  6,   CP0St_KX   =  7,
      CP0St_IM   =  8,   CP0St_NMI  =  19,  CP0St_SR   =  20,  CP0St_TS   =  21,
      CP0St_BEV  =  22,  CP0St_PX   =  23,  CP0St_MX   =  24,  CP0St_RE   =  25,
      CP0St_FR   =  26,  CP0St_RP   =  27,  CP0St_CU0  =  28,  CP0St_CU1  =  29,
      CP0St_CU2  =  30,  CP0St_CU3  =  31
};

static void doop(Mips * emu, uint32_t op);

Mips *new_mips(uint32_t physMemSize)
{       void *mem;
        Mips *ret;
        if (physMemSize % 4 != 0) 
                return 0;

        if (!(mem = calloc(physMemSize, 1)))
                return 0;

        if (!(ret = calloc(1, sizeof(Mips)))) {
                free(mem);
                return 0;
        }

        ret->mem = mem;
        ret->pmemsz = physMemSize;

        /* Init status registers*/

        ret->CP0_Status |= (1 << CP0St_ERL);    /*start in kernel mode with unmapped useg*/

        uart_Reset(ret);

        return ret;
}

void free_mips(Mips * mips)
{
        free(mips->mem);
        free(mips);
}

/* bitwise helpers */
int32_t sext18(uint32_t val)
{
        if ((val & 0x20000) != 0)
                return 0xfffc0000 | val;
        return 0x0003ffff & val;
}

enum exc_field { /*Possible Values for the EXC field in the status reg*/
        EXC_Int   =  0,   EXC_Mod   =  1,   EXC_TLBL   =  2,   EXC_TLBS    = 3,
        EXC_AdEL  =  4,   EXC_AdES  =  5,   EXC_IBE    =  6,   EXC_DBE     = 7,
        EXC_SYS   =  8,   EXC_BP    =  9,   EXC_RI     =  10,  EXC_CpU     = 11,
        EXC_Ov    =  12,  EXC_Tr    =  13,  EXC_Watch  =  23,  EXC_MCheck  = 24
};

void triggerExternalInterrupt(Mips * emu, unsigned intNum) /*not used*/
{
        emu->CP0_Cause |= ((1 << intNum) & 0x3f) << 10;
}

void clearExternalInterrupt(Mips * emu, unsigned intNum) /*not used*/
{
        emu->CP0_Cause &= ~(((1 << intNum) & 0x3f) << 10);
}

static uint32_t getExceptionCode(Mips * emu)
{
        return (emu->CP0_Cause >> 2) & 0x1f;
}

static void setExceptionCode(Mips * emu, int code)
{
        emu->CP0_Cause &= ~(0x1f << 2); /* clear exccode*/
        emu->CP0_Cause |= (code & 0x1f) << 2;   /*set with new code*/
}

static uint32_t isKernelMode(Mips * emu)
{
        if (!(emu->CP0_Status & (1 << CP0St_UM)))
                return 1;
        return emu->CP0_Status & ((1 << CP0St_EXL) | (1 << CP0St_ERL));
}

/*tlb lookup return codes*/

enum tblReturnCodes { TLBRET_MATCH, TLBRET_NOMATCH, TLBRET_DIRTY, TLBRET_INVALID };


uint32_t counter = 0; /*horrible but deterministic fake rand for testing*/

static uint32_t randomInRange(uint32_t a, uint32_t b)
{
        counter++;
        return a + (counter % (1 + b - a));
}

static void writeTlbExceptionExtraData(Mips * emu, uint32_t vaddr)
{
        emu->CP0_BadVAddr = vaddr;
        emu->CP0_Context = (emu->CP0_Context & ~0x007fffff) | ((vaddr >> 9) & 0x007ffff0);
        emu->CP0_EntryHi = (emu->CP0_EntryHi & 0xff) | (vaddr & (0xffffe000));
}

static int tlb_lookup(Mips * emu, uint32_t vaddress, uint32_t * physical, int write)
{ /*XXX currently hardcoded for 4k pages*/
        uint8_t ASID = emu->CP0_EntryHi & 0xFF;
        int i;
        emu->tlb.exceptionWasNoMatch = 0;

        for (i = 0; i < 16; i++) {
                TLB_entry *tlb_e = &emu->tlb.entries[i];
                uint32_t tag = (vaddress & 0xfffff000) >> 13;
                uint32_t VPN2 = tlb_e->VPN2;
                /* Check ASID, virtual page number & size */
                if ((tlb_e->G == 1 || tlb_e->ASID == ASID) && VPN2 == tag) {
                        /* TLB match */
                        int n = (vaddress >> 12) & 1;
                        /* Check access rights */
                        if (!(n ? tlb_e->V1 : tlb_e->V0)) {
                                emu->exceptionOccured = 1;
                                setExceptionCode(emu, write ? EXC_TLBS : EXC_TLBL);
                                writeTlbExceptionExtraData(emu, vaddress);
                                return TLBRET_INVALID;
                        }
                        if (write == 0 || (n ? tlb_e->D1 : tlb_e->D0)) {
                                *physical = tlb_e->PFN[n] | (vaddress & 0xfff);
                                return TLBRET_MATCH;
                        }
                        emu->exceptionOccured = 1;
                        setExceptionCode(emu, EXC_Mod);
                        writeTlbExceptionExtraData(emu, vaddress);
                        return TLBRET_DIRTY;
                }
        }
        emu->tlb.exceptionWasNoMatch = 1;
        emu->exceptionOccured = 1;
        setExceptionCode(emu, write ? EXC_TLBS : EXC_TLBL);
        writeTlbExceptionExtraData(emu, vaddress);
        return TLBRET_NOMATCH;
}

static int translateAddress(Mips * emu, uint32_t vaddr, uint32_t * paddr_out, int write)
{ /*internally triggers exceptions on error returns an error code*/
        if (vaddr <= 0x7FFFFFFF) {
                /* useg */
                if (emu->CP0_Status & (1 << CP0St_ERL)) {
                        *paddr_out = vaddr;
                        return 0;
                } else {
                        return tlb_lookup(emu, vaddr, paddr_out, write);
                }
        } else if (vaddr >= 0x80000000 && vaddr <= 0x9fffffff) {
                *paddr_out = vaddr - 0x80000000;
                return 0;
        } else if (vaddr >= 0xa0000000 && vaddr <= 0xbfffffff) {
                *paddr_out = vaddr - 0xa0000000;
                return 0;
        } else {                /*kseg2 and 3*/
                if (isKernelMode(emu)) {
                        return tlb_lookup(emu, vaddr, paddr_out, write);
                } else {
                        *paddr_out = vaddr;
                        FATAL("translateAddress: unhandled exception");
                }
        }

        FATAL("unreachable.");
        return 1;
}

static uint32_t readVirtWord(Mips * emu, uint32_t addr)
{
        uint32_t paddr;
        if(translateAddress(emu, addr, &paddr, 0))
                return 0;

        if (paddr % 4) {
                printf("Unhandled alignment error reading addr %08x\n", addr);
                exit(1);
        }

        if ((paddr >= UARTBASE) && (paddr <= (UARTBASE + UARTSIZE)))
                return 0; /*XXX: uart read always returns 0*/

        if (paddr >= emu->pmemsz) {
                printf("unhandled bus error at pc: %08x reading paddr: %08x\n", emu->pc, paddr);
                exit(1);
        }

        return emu->mem[paddr / 4];
}

static void writeVirtWord(Mips * emu, uint32_t addr, uint32_t val)
{
        uint32_t paddr;
        if(translateAddress(emu, addr, &paddr, 1))
                return;

        if (paddr % 4) {
                printf("Unhandled alignment error reading addr %08x\n", addr);
                exit(1);
        }

        if (paddr >= UARTBASE && paddr <= UARTBASE + UARTSIZE)
                return; /*XXX:  uart_write does not do anything at the moment*/

        if (paddr >= emu->pmemsz) {
                printf("bus error at pc: %08x writing paddr: %08x\n", emu->pc, paddr);
                setExceptionCode(emu, EXC_DBE);
                emu->exceptionOccured = 1;
                return;
        }

        emu->mem[paddr / 4] = val;
}

static uint8_t readVirtByte(Mips * emu, uint32_t addr)
{
        unsigned offset;
        uint32_t paddr, word, shamt, mask;
        if (translateAddress(emu, addr, &paddr, 0))
                return 0;

        if (paddr >= UARTBASE && paddr <= UARTBASE + UARTSIZE)
                return uart_readb(emu, paddr - UARTBASE);

        offset = paddr & 3;

        if (paddr >= emu->pmemsz) {
                printf("unhandled bus error paddr: %08x\n", paddr);
                exit(1);
        }

        word = emu->mem[(paddr & (~0x3)) / 4];
        shamt = 8 * (3 - offset);
        mask = 0xff << shamt;
        return  (word & mask) >> shamt;
}

static void writeVirtByte(Mips * emu, uint32_t addr, uint8_t val)
{
        unsigned offset, shamt;
        uint32_t paddr, baseaddr, word, clearmask, valmask;

        if(translateAddress(emu, addr, &paddr, 1))
                return;

        if (paddr >= UARTBASE && paddr <= UARTBASE + UARTSIZE) {
                uart_writeb(emu, paddr - UARTBASE, val);
                return;
        }

        if (paddr >= POWERBASE && paddr <= POWERBASE + POWERSIZE) {
                emu->shutdown = 1;
                return;
        }

        if (paddr >= emu->pmemsz) {
                printf("unhandled bus error paddr: %08x\n", paddr);
                exit(1);
        }

        offset = paddr & 3;
        baseaddr = paddr & (~0x3);
        word = emu->mem[baseaddr / 4];
        shamt = 8 * (3 - offset);
        clearmask = ~(0xff << shamt);
        valmask = (val << shamt);
        word = (word & clearmask);
        word = (word | valmask);
        emu->mem[baseaddr / 4] = word;
}

static void handleException(Mips * emu, int inDelaySlot)
{
        uint32_t offset;
        int exccode = getExceptionCode(emu);

        emu->inDelaySlot = 0;

        if ((emu->CP0_Status & (1 << CP0St_EXL)) == 0) {
                if (inDelaySlot) {
                        emu->CP0_Epc = emu->pc - 4;
                        emu->CP0_Cause |= (1 << 31);    /* set BD*/
                } else {
                        emu->CP0_Epc = emu->pc;
                        emu->CP0_Cause &= ~(1 << 31);   /* clear BD*/
                }

                if (exccode == EXC_TLBL || exccode == EXC_TLBS) {
                        /*printf("tlb exception %x\n",emu->CP0_Epc);*/
                        /*XXX this seems inverted? bug in also qemu? test with these cases reversed after booting.*/
                        if (!emu->tlb.exceptionWasNoMatch)
                                offset = 0x180;
                        else 
                                offset = 0;
                } else if ((exccode == EXC_Int) && ((emu->CP0_Cause & (1 << 23)) != 0)) {
                        offset = 0x200;
                } else {
                        offset = 0x180;
                }
        } else {
                offset = 0x180;
        }

        /* Faulting coprocessor number set at fault location*/
        /* exccode set at fault location*/
        emu->CP0_Status |= (1 << CP0St_EXL);

        if (emu->CP0_Status & (1 << CP0St_BEV))
                emu->pc = 0xbfc00200 + offset;
        else
                emu->pc = 0x80000000 + offset;

        emu->exceptionOccured = 0;
}

/* return 1 if interrupt occured else 0*/
static int handleInterrupts(Mips * emu)
{
        /*if interrupts disabled or ERL or EXL set*/
        if ((emu->CP0_Status & 1) == 0 || (emu->CP0_Status & ((1 << 1) | (1 << 2)))) {
                return 0;       /* interrupts disabled*/
        }

        if (!(emu->CP0_Cause & emu->CP0_Status & 0xfc00))
                return 0;       /* no pending interrupts*/

        emu->waiting = 0;
        setExceptionCode(emu, EXC_Int);
        handleException(emu, emu->inDelaySlot);

        return 1;
}

void step_mips(Mips * emu)
{
        int startInDelaySlot;
        uint32_t opcode;
        if (emu->shutdown)
                return;

        emu->CP0_Count++;
        /* timer code */
        if (emu->CP0_Count == emu->CP0_Compare) {
                /* but only do this if interrupts are enabled to save time.*/
                triggerExternalInterrupt(emu, 5);       /* 5 is the timer int :)*/
        }

        if (handleInterrupts(emu))
                return;

        if (emu->waiting)
                return;

        /* end timer code */

        startInDelaySlot = emu->inDelaySlot;
        opcode = readVirtWord(emu, emu->pc);

        if (emu->exceptionOccured) {    /*instruction fetch failed*/
                handleException(emu, startInDelaySlot);
                return;
        }

        doop(emu, opcode);
        emu->regs[0] = 0;

        if (emu->exceptionOccured) {    /*instruction failed*/
                handleException(emu, startInDelaySlot);
                return;
        }

        if (startInDelaySlot) {
                emu->pc = emu->delaypc;
                emu->inDelaySlot = 0;
                return;
        }

        emu->pc += 4;
}

static void setRt(Mips * emu, uint32_t op, uint32_t val)
{
        uint32_t idx = (op & 0x1f0000) >> 16;
        emu->regs[idx] = val;
}

static uint32_t getRt(Mips * emu, uint32_t op)
{
        uint32_t idx = (op & 0x1f0000) >> 16;
        return emu->regs[idx];
}

static void setRd(Mips * emu, uint32_t op, uint32_t val)
{
        uint32_t idx = (op & 0xf800) >> 11;
        emu->regs[idx] = val;
}

static uint32_t getRs(Mips * emu, uint32_t op)
{
        uint32_t idx = (op & 0x3e00000) >> 21;
        return emu->regs[idx];
}

static uint16_t getImm(uint32_t op)
{
        return op & 0xffff;
}

static uint32_t getShamt(uint32_t op)
{
        return (op & 0x7c0) >> 6;
}

/* start opcode implementations */

static void op_wait(Mips * emu, uint32_t op)
{       UNUSED(emu); UNUSED(op);
        emu->waiting = 1;
}

static void op_syscall(Mips * emu, uint32_t op)
{       UNUSED(op);
        emu->exceptionOccured = 1;
        setExceptionCode(emu, EXC_SYS);
}

static void op_sync(Mips * emu, uint32_t op)
{       UNUSED(emu); UNUSED(op);
        /* hopefully nothing needed */
}

static void op_swl(Mips * emu, uint32_t op)
{
        int32_t c = (int16_t) (op & 0x0000ffff);
        uint32_t addr = ((int32_t) getRs(emu, op) + c);
        uint32_t rtVal = getRt(emu, op);
        uint32_t wordVal = readVirtWord(emu, addr & (~3));
        uint32_t offset = addr & 3;
        uint32_t result;
        if (emu->exceptionOccured)
                return;

        switch (offset) {
        case 0: result = rtVal;                                              break;
        case 1: result = (wordVal & 0xff000000) | ((rtVal >> 8) & 0xffffff); break;
        case 2: result = (wordVal & 0xffff0000) | ((rtVal >> 16) & 0xffff);  break;
        case 3: result = (wordVal & 0xffffff00) | ((rtVal >> 24) & 0xff);    break;
        }

        writeVirtWord(emu, addr & (~3), result);
}

static void op_lwl(Mips * emu, uint32_t op)
{
        int32_t c = (int16_t) (op & 0x0000ffff);
        uint32_t addr = (int32_t) getRs(emu, op) + c;
        uint32_t rtVal = getRt(emu, op);
        uint32_t wordVal = readVirtWord(emu, addr & (~3));
        uint32_t offset = addr % 4;
        uint32_t result;
        if (emu->exceptionOccured) { /*from readVirtWord*/
                return;
        }

        switch (offset) {
        case 0: result = wordVal;                                break;
        case 1: result = ((wordVal << 8)  | (rtVal & 0xff));     break;
        case 2: result = ((wordVal << 16) | (rtVal & 0xffff));   break;
        case 3: result = ((wordVal << 24) | (rtVal & 0xffffff)); break;
        }

        setRt(emu, op, result);
}

static void op_bne(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (getRs(emu, op) != getRt(emu, op))
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
        else 
                emu->delaypc = emu->pc + 8;
        emu->inDelaySlot = 1;
}

static void op_bnel(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (getRs(emu, op) != getRt(emu, op)) {
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
                emu->inDelaySlot = 1;
        } else {
                emu->pc += 4;
        }
}

static void op_tne(Mips * emu, uint32_t op)
{
        if (getRs(emu, op) != getRt(emu, op))
                FATAL("unhandled trap!");
}

static void op_andi(Mips * emu, uint32_t op)
{
        uint32_t v = getRs(emu, op) & getImm(op);
        setRt(emu, op, v);
}

static void op_jal(Mips * emu, uint32_t op)
{
        uint32_t pc = emu->pc;
        uint32_t top = pc & 0xf0000000;
        uint32_t addr = (top | ((op & 0x3ffffff) << 2));
        emu->delaypc = addr;
        emu->regs[31] = pc + 8;
        emu->inDelaySlot = 1;
}

static void op_sb(Mips * emu, uint32_t op)
{
        uint32_t addr = (int32_t) getRs(emu, op) + (int16_t) getImm(op);
        writeVirtByte(emu, addr, getRt(emu, op) & 0xff);
}

static void op_lb(Mips * emu, uint32_t op)
{
        uint32_t addr = ((int32_t) getRs(emu, op) + (int16_t) getImm(op));
        int8_t v = (int8_t) readVirtByte(emu, addr);
        if (emu->exceptionOccured) {
                return;
        }
        setRt(emu, op, (int32_t) v);
}

static void op_beq(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (getRs(emu, op) == getRt(emu, op)) {
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
        } else {
                emu->delaypc = emu->pc + 8;
        }
        emu->inDelaySlot = 1;
}

static void op_beql(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (getRs(emu, op) == getRt(emu, op)) {
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
                emu->inDelaySlot = 1;
        } else {
                emu->pc += 4;
        }
}

static void op_lbu(Mips * emu, uint32_t op)
{
        uint32_t addr = ((int32_t) getRs(emu, op) + (int16_t) getImm(op));
        uint32_t v = readVirtByte(emu, addr);
        if (emu->exceptionOccured)
                return;
        setRt(emu, op, v);
}

static void op_lw(Mips * emu, uint32_t op)
{
        int16_t offset = getImm(op);
        uint32_t addr = ((int32_t) getRs(emu, op) + offset);
        uint32_t v = readVirtWord(emu, addr);
        if (emu->exceptionOccured)
                return;
        setRt(emu, op, v);
}

static void op_j(Mips * emu, uint32_t op)
{
        uint32_t top = emu->pc & 0xf0000000;
        uint32_t addr = top | ((op & 0x3ffffff) * 4);
        emu->delaypc = addr;
        emu->inDelaySlot = 1;
}

static void op_sh(Mips * emu, uint32_t op)
{
        int16_t offset = getImm(op);
        uint32_t addr = (int32_t) getRs(emu, op) + offset;
        uint8_t vlo = getRt(emu, op) & 0xff;
        uint8_t vhi = (getRt(emu, op) & 0xff00) >> 8;
        writeVirtByte(emu, addr, vhi);
        if (emu->exceptionOccured)
                return;
        writeVirtByte(emu, addr + 1, vlo);
}

static void op_slti(Mips * emu, uint32_t op)
{
        int32_t rs = getRs(emu, op);
        int32_t c = (int16_t) getImm(op);
        if (rs < c)
                setRt(emu, op, 1);
        else
                setRt(emu, op, 0);
}

static void op_sltiu(Mips * emu, uint32_t op)
{
        uint32_t rs = getRs(emu, op);
        uint32_t c = (uint32_t) (int32_t) (int16_t) getImm(op);
        if (rs < c)
                setRt(emu, op, 1);
        else
                setRt(emu, op, 0);
}

static void op_addiu(Mips * emu, uint32_t op)
{
        int32_t v = (int32_t) getRs(emu, op) + (int16_t) (getImm(op));
        setRt(emu, op, (uint32_t) v);
}

static void op_xori(Mips * emu, uint32_t op)
{
        uint32_t v = getRs(emu, op) ^ getImm(op);
        setRt(emu, op, v);
}

static void op_ori(Mips * emu, uint32_t op)
{
        uint32_t v = getRs(emu, op) | getImm(op);
        setRt(emu, op, v);
}

static void op_swr(Mips * emu, uint32_t op)
{
        int32_t c = (int16_t) (op & 0x0000ffff);
        uint32_t addr = (int32_t) getRs(emu, op) + c;
        uint32_t rtVal = getRt(emu, op);
        uint32_t wordVal = readVirtWord(emu, addr & (~3));
        uint32_t offset = addr & 3;
        uint32_t result;
        if (emu->exceptionOccured) { /*from readVirtWord*/
                return;
        }

        switch (offset) {
        case 3: result = rtVal;                                                 break;
        case 2: result = ((rtVal << 8) & 0xffffff00) | (wordVal & 0xff);        break;
        case 1: result = ((rtVal << 16) & 0xffff0000) | (wordVal & 0xffff);     break;
        case 0: result = ((rtVal << 24) & 0xff000000) | (wordVal & 0xffffff);   break;
        }
        writeVirtWord(emu, addr & (~3), result);
}

static void op_sw(Mips * emu, uint32_t op)
{
        int16_t offset = getImm(op);
        uint32_t addr = ((int32_t) getRs(emu, op) + offset);
        writeVirtWord(emu, addr, getRt(emu, op));
}

static void op_lh(Mips * emu, uint32_t op)
{
        uint32_t v, addr = (int32_t) getRs(emu, op) + (int16_t) getImm(op);
        uint8_t vlo, vhi;
        vlo = readVirtByte(emu, addr + 1);
        if (emu->exceptionOccured)
                return;
        vhi = readVirtByte(emu, addr);
        if (emu->exceptionOccured)
                return;
        v = (int32_t) (int16_t) ((vhi << 8) | vlo);
        setRt(emu, op, v);
}

static void op_lui(Mips * emu, uint32_t op)
{
        uint32_t v = getImm(op) << 16;
        setRt(emu, op, v);
}

static void op_addi(Mips * emu, uint32_t op)
{
        uint32_t v = (int32_t) getRs(emu, op) + (int16_t) getImm(op);
        setRt(emu, op, v);
}

static void op_lhu(Mips * emu, uint32_t op)
{
        uint32_t v, addr = (int32_t) getRs(emu, op) + (int16_t) getImm(op);
        uint8_t vlo = readVirtByte(emu, addr + 1), vhi;
        if (emu->exceptionOccured)
                return;
        vhi = readVirtByte(emu, addr);
        if (emu->exceptionOccured)
                return;
        v = (vhi << 8) | vlo;
        setRt(emu, op, v);
}

static void op_bgtz(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) > 0)
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
        else
                emu->delaypc = emu->pc + 8;
        emu->inDelaySlot = 1;
}

static void op_bgtzl(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) > 0) {
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
                emu->inDelaySlot = 1;
        } else {
                emu->pc += 4;
        }
}

static void op_blez(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) <= 0)
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
        else
                emu->delaypc = emu->pc + 8;
        emu->inDelaySlot = 1;
}

static void op_blezl(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) <= 0) {
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
                emu->inDelaySlot = 1;
        } else {
                emu->pc += 4;
        }
}

static void op_lwr(Mips * emu, uint32_t op)
{
        int32_t c = (int16_t) (op & 0x0000ffff);
        uint32_t addr = ((int32_t) getRs(emu, op)) + c;
        uint32_t rtVal = getRt(emu, op);
        uint32_t wordVal = readVirtWord(emu, addr & (~3));
        uint32_t offset = addr & 3;
        uint32_t result;
        if (emu->exceptionOccured) /*from readVirtWord*/
                return;

        switch (offset) {
        case 3: result = wordVal;                                       break;
        case 2: result = ((rtVal & 0xff000000) | (wordVal >> 8));       break;
        case 1: result = ((rtVal & 0xffff0000) | (wordVal >> 16));      break;
        case 0: result = ((rtVal & 0xffffff00) | (wordVal >> 24));      break;
        }
        setRt(emu, op, result);
}

static void op_mfc0(Mips * emu, uint32_t op)
{
        uint32_t regNum = (op & 0xf800) >> 11;
        uint32_t sel = op & 7;
        uint32_t retval;
        switch (regNum) {

        case 0:                /* Index*/
                if (sel != 0) 
                        goto unhandled;
                retval = emu->CP0_Index;
                break;
        case 2:                /*EntryLo0*/
                retval = emu->CP0_EntryLo0;
                break;
        case 3:                /*EntryLo1*/
                retval = emu->CP0_EntryLo1;
                break;
        case 4:                /* Context*/
                if (sel != 0) 
                        goto unhandled;
                retval = emu->CP0_Context;
                break;
        case 5:                /* Page Mask*/
                if (sel != 0) 
                        goto unhandled;
                retval = emu->CP0_PageMask;
                break;
        case 6:                /* Wired*/
                if (sel != 0)
                        goto unhandled;
                retval = emu->CP0_Wired;
                break;
        case 8:                /* BadVAddr*/
                if (sel != 0) 
                        goto unhandled;
                retval = emu->CP0_BadVAddr;
                break;
        case 9:                /* Count*/
                if (sel != 0) 
                        goto unhandled;
                retval = emu->CP0_Count;
                break;
        case 10:               /* EntryHi*/
                if (sel != 0) 
                        goto unhandled;
                retval = emu->CP0_EntryHi;
                break;
        case 11:               /* Compare*/
                if (sel != 0) 
                        goto unhandled;
                retval = emu->CP0_Compare;
                break;
        case 12:               /* Status*/
                if (sel != 0) 
                        goto unhandled;
                retval = emu->CP0_Status;
                break;
        case 13:
                if (sel != 0) 
                        goto unhandled;
                retval = emu->CP0_Cause;
                break;
        case 14:               /* EPC*/
                if (sel != 0)
                        goto unhandled;
                retval = emu->CP0_Epc;
                break;
        case 15:
                retval = 0x00018000;    /*processor id, just copied qemu 4kc*/
                break;
        case 16:
                if (sel == 0) {
                        retval = 0x80008082;    /* XXX cacheability fields shouldnt be hardcoded as writeable*/
                        break;
                }
                if (sel == 1) {
                        retval = 0x1e190c8a;
                        break;
                }
                goto unhandled;
        case 18: /*fallthrough ?*/
        case 19: retval = 0; break;
        default:
 unhandled:
                printf("unhandled cp0 reg selector in mfc0 %d %d\n", regNum, sel);
                exit(1);
        }

        setRt(emu, op, retval);
}

static void op_mtc0(Mips * emu, uint32_t op) /*move to co-processor 0*/
{
        uint32_t rt = getRt(emu, op);
        uint32_t regNum = (op & 0xf800) >> 11;
        uint32_t sel = op & 7;

        switch (regNum) {
        case 0:                /* Index*/
                if (sel)
                        goto unhandled;
                emu->CP0_Index = (emu->CP0_Index & 0x80000000) | (rt & 0xf);
                break;
        case 2:                /* EntryLo0*/
                emu->CP0_EntryLo0 = (rt & 0x3ffffff);
                break;
        case 3:                /* EntryLo1*/
                emu->CP0_EntryLo1 = (rt & 0x3ffffff);
                break;
        case 4:                /* Context*/
                emu->CP0_Context &= (emu->CP0_Context & ~0xff800000) | (rt & 0xff800000);
                break;
        case 5:                /* Page Mask*/
                if (sel)
                        goto unhandled;
                if (rt)
                        FATAL("XXX untested page mask, unhandled page mask");
                emu->CP0_PageMask = rt & 0x1ffe000;
                break;
        case 6:                /* Wired*/
                if (sel)
                        goto unhandled;
                emu->CP0_Wired = rt & 0xf;
                break;
        case 9:                /* Count*/
                if (sel)
                        goto unhandled;
                emu->CP0_Count = rt;
                break;
        case 10:               /* EntryHi*/
                if (sel)
                        goto unhandled;
                emu->CP0_EntryHi = rt & (~0x1f00);
                break;
        case 11:               /* Compare*/
                if (sel)
                        goto unhandled;
                clearExternalInterrupt(emu, 5);
                emu->CP0_Compare = rt;
                break;
        case 12:               /* Status*/
        {
                uint32_t status_mask = 0x7d7cff17; /*XXX: magic number*/
                if (sel)
                        goto unhandled;
                emu->CP0_Status = (emu->CP0_Status & ~status_mask) | (rt & status_mask);
        } break; /*XXX NMI is one way write*/
        case 13:               /*cause*/
        {       
                uint32_t cause_mask = ((1 << 23) | (1 << 22) | (3 << 8));
                if (sel != 0)
                        goto unhandled;
                emu->CP0_Cause = (emu->CP0_Cause & ~cause_mask) | (rt & cause_mask);
        } break;
        case 14:               /*epc*/
                if (sel != 0)
                        goto unhandled;
                emu->CP0_Epc = rt;
                break;
        case 16:
                break;
        case 18: /*fall through*/
        case 19:
                break;

        default:
 unhandled:
                printf("unhandled cp0 reg selector in mtc0 %d %d\n", regNum, sel);
                exit(1);
        }
}

static void op_cache(Mips * emu, uint32_t op)
{       UNUSED(emu); UNUSED(op);
        /*XXX: noop? */
}

static void op_pref(Mips * emu, uint32_t op)
{       UNUSED(emu); UNUSED(op);
        /*XXX: noop? */
}

static void op_eret(Mips * emu, uint32_t op)
{       UNUSED(op);
        if (emu->inDelaySlot)
                return;

        emu->llbit = 0;

        if (emu->CP0_Status & (1 << 2)) {       /*if ERL is set*/
                emu->CP0_Status &= ~(1 << 2);   /*clear ERL;*/
                emu->pc = emu->CP0_ErrorEpc;
        } else {
                emu->pc = emu->CP0_Epc;
                emu->CP0_Status &= ~(1 << 1);   /*clear EXL;*/
        }
        emu->pc -= 4;           /*counteract typical pc += 4*/
}

static void helper_writeTlbEntry(Mips * emu, uint32_t idx)
{
        /*printf("tlb write idx %d\n",idx);*/
        TLB_entry *tlbent;
        idx &= 0xf;             /*only 16 entries must mask it off*/
        tlbent  = &emu->tlb.entries[idx];
        tlbent->VPN2 = emu->CP0_EntryHi >> 13;
        tlbent->ASID = emu->CP0_EntryHi & 0xff;
        tlbent->G = (emu->CP0_EntryLo0 & emu->CP0_EntryLo1) & 1;
        tlbent->V0 = (emu->CP0_EntryLo0 & 2) > 0;
        tlbent->V1 = (emu->CP0_EntryLo1 & 2) > 0;
        tlbent->D0 = (emu->CP0_EntryLo0 & 4) > 0;
        tlbent->D1 = (emu->CP0_EntryLo1 & 4) > 0;
        tlbent->C0 = (emu->CP0_EntryLo0 >> 3) & 7;
        tlbent->C1 = (emu->CP0_EntryLo1 >> 3) & 7;
        tlbent->PFN[0] = ((emu->CP0_EntryLo0 >> 6) & 0xfffff) << 12;
        tlbent->PFN[1] = ((emu->CP0_EntryLo1 >> 6) & 0xfffff) << 12;
}

static void op_tlbwi(Mips * emu, uint32_t op)
{       
        uint32_t idx = emu->CP0_Index;
        UNUSED(op);
        helper_writeTlbEntry(emu, idx);
}

static void op_tlbwr(Mips * emu, uint32_t op)
{       
        uint32_t idx = randomInRange(emu->CP0_Wired, 15);
        UNUSED(op);       
        helper_writeTlbEntry(emu, idx);
}

/*XXX hardcoded for 4k pages*/
static void op_tlbp(Mips * emu, uint32_t op)
{       
        uint8_t ASID = emu->CP0_EntryHi & 0xFF;
        int i;
        UNUSED(op);

        emu->CP0_Index = 0x80000000;

        for (i = 0; i < 16; i++) {
                TLB_entry *tlb_e = &emu->tlb.entries[i];
                uint32_t tag = (emu->CP0_EntryHi & 0xfffff000) >> 13;
                uint32_t VPN2 = tlb_e->VPN2;
                /* Check ASID, virtual page number & size */
                if ((tlb_e->G == 1 || tlb_e->ASID == ASID) && VPN2 == tag) {
                        /* TLB match */
                        emu->CP0_Index = i;
                        return;
                }
        }
}

static void op_sltu(Mips * emu, uint32_t op)
{
        uint32_t rs = getRs(emu, op);
        uint32_t rt = getRt(emu, op);
        if (rs < rt)
                setRd(emu, op, 1);
        else 
                setRd(emu, op, 0);
}

static void op_slt(Mips * emu, uint32_t op)
{
        int32_t rs = getRs(emu, op);
        int32_t rt = getRt(emu, op);
        if (rs < rt)
                setRd(emu, op, 1);
        else
                setRd(emu, op, 0);
}

static void op_nor(Mips * emu, uint32_t op)
{
        setRd(emu, op, ~(getRs(emu, op) | getRt(emu, op)));
}

static void op_xor(Mips * emu, uint32_t op)
{
        setRd(emu, op, getRs(emu, op) ^ getRt(emu, op));
}

static void op_or(Mips * emu, uint32_t op)
{
        setRd(emu, op, getRs(emu, op) | getRt(emu, op));
}

static void op_and(Mips * emu, uint32_t op)
{
        setRd(emu, op, getRs(emu, op) & getRt(emu, op));
}

static void op_subu(Mips * emu, uint32_t op)
{
        setRd(emu, op, getRs(emu, op) - getRt(emu, op));
}

static void op_sub(Mips * emu, uint32_t op)
{
        setRd(emu, op, getRs(emu, op) - getRt(emu, op));
}

static void op_addu(Mips * emu, uint32_t op)
{
        setRd(emu, op, getRs(emu, op) + getRt(emu, op));
}

static void op_add(Mips * emu, uint32_t op)
{
        setRd(emu, op, getRs(emu, op) + getRt(emu, op));
}

static void op_ll(Mips * emu, uint32_t op)
{
        int16_t offset = getImm(op);
        uint32_t addr = ((int32_t) getRs(emu, op) + offset);
        uint32_t v = readVirtWord(emu, addr);
        if (emu->exceptionOccured)
                return;
        emu->llbit = 1;
        setRt(emu, op, v);
}

static void op_sc(Mips * emu, uint32_t op)
{
        int16_t offset = getImm(op);
        uint32_t addr = ((int32_t) getRs(emu, op) + offset);
        if (emu->llbit)
                writeVirtWord(emu, addr, getRt(emu, op));
        setRt(emu, op, emu->llbit);

}

static void op_divu(Mips * emu, uint32_t op)
{
        uint32_t rs = getRs(emu, op);
        uint32_t rt = getRt(emu, op);

        if (!rt)
                return;
        emu->lo = rs / rt;
        emu->hi = rs % rt;
}

static void op_div(Mips * emu, uint32_t op)
{
        int32_t rs = getRs(emu, op);
        int32_t rt = getRt(emu, op);

        if (!rt)
                return;
        emu->lo = rs / rt;
        emu->hi = rs % rt;
}

static void op_multu(Mips * emu, uint32_t op)
{
        uint64_t result = (uint64_t) getRs(emu, op) * (uint64_t) getRt(emu, op);
        emu->hi = result >> 32;
        emu->lo = result & 0xffffffff;
}

static void op_mult(Mips * emu, uint32_t op)
{
        int64_t result = (int64_t) (int32_t) getRs(emu, op) * (int64_t) (int32_t) getRt(emu, op);
        emu->hi = result >> 32;
        emu->lo = result & 0xffffffff;
}

static void op_movz(Mips * emu, uint32_t op)
{
        if (!getRt(emu, op))
                setRd(emu, op, getRs(emu, op));
}

static void op_movn(Mips * emu, uint32_t op)
{
        if (getRt(emu, op) != 0) {
                setRd(emu, op, getRs(emu, op));
        }
}

static void op_mul(Mips * emu, uint32_t op)
{
        int32_t result = (int32_t) getRs(emu, op) * (int32_t) getRt(emu, op);
        setRd(emu, op, result);
}

static void op_mflo(Mips * emu, uint32_t op)
{
        setRd(emu, op, emu->lo);
}

static void op_mfhi(Mips * emu, uint32_t op)
{
        setRd(emu, op, emu->hi);
}

static void op_mtlo(Mips * emu, uint32_t op)
{
        emu->lo = getRs(emu, op);
}

static void op_mthi(Mips * emu, uint32_t op)
{
        emu->hi = getRs(emu, op);
}

static void op_jalr(Mips * emu, uint32_t op)
{
        emu->delaypc = getRs(emu, op);
        emu->regs[31] = emu->pc + 8;
        emu->inDelaySlot = 1;
}

static void op_jr(Mips * emu, uint32_t op)
{
        emu->delaypc = getRs(emu, op);
        emu->inDelaySlot = 1;
}

static void op_srav(Mips * emu, uint32_t op)
{
        setRd(emu, op, ((int32_t) getRt(emu, op) >> (getRs(emu, op) & 0x1f)));
}

static void op_srlv(Mips * emu, uint32_t op)
{
        setRd(emu, op, (getRt(emu, op) >> (getRs(emu, op) & 0x1f)));
}

static void op_sllv(Mips * emu, uint32_t op)
{
        setRd(emu, op, (getRt(emu, op) << (getRs(emu, op) & 0x1f)));
}

static void op_sra(Mips * emu, uint32_t op)
{
        int32_t rt = getRt(emu, op);
        int32_t sa = getShamt(op);
        int32_t v = rt >> sa;
        setRd(emu, op, v);
}

static void op_srl(Mips * emu, uint32_t op)
{
        uint32_t v = getRt(emu, op) >> getShamt(op);
        setRd(emu, op, v);
}

static void op_sll(Mips * emu, uint32_t op)
{
        uint32_t v = getRt(emu, op) << getShamt(op);
        setRd(emu, op, v);
}

static void op_bgezal(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) >= 0)
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
        else
                emu->delaypc = emu->pc + 8;
        emu->regs[31] = emu->pc + 8;
        emu->inDelaySlot = 1;
}

static void op_bgez(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) >= 0)
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
        else
                emu->delaypc = emu->pc + 8;
        emu->inDelaySlot = 1;
}

static void op_bgezl(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) >= 0) {
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
                emu->inDelaySlot = 1;
        } else {
                emu->pc += 4; 
        }
}

static void op_bltzal(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) < 0)
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
        else
                emu->delaypc = emu->pc + 8;
        emu->regs[31] = emu->pc + 8;
        emu->inDelaySlot = 1;
}

static void op_bltz(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) < 0)
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
        else
                emu->delaypc = emu->pc + 8;
        emu->inDelaySlot = 1;
}

static void op_bltzl(Mips * emu, uint32_t op)
{
        int32_t offset = sext18(getImm(op) * 4);
        if (((int32_t) getRs(emu, op)) < 0) {
                emu->delaypc = (int32_t) (emu->pc + 4) + offset;
                emu->inDelaySlot = 1;
        } else {
                emu->pc += 4;
        }
}

static void doop(Mips * emu, uint32_t op)
{
        switch (op & 0xfc000000) {
        case 0x8000000:         op_j(emu, op);          return;
        case 0xc000000:         op_jal(emu, op);        return;
        case 0x10000000:        op_beq(emu, op);        return;
        case 0x14000000:        op_bne(emu, op);        return;
        case 0x18000000:        op_blez(emu, op);       return;
        case 0x1c000000:        op_bgtz(emu, op);       return;
        case 0x20000000:        op_addi(emu, op);       return;
        case 0x24000000:        op_addiu(emu, op);      return;
        case 0x28000000:        op_slti(emu, op);       return;        
        case 0x2c000000:        op_sltiu(emu, op);      return;        
        case 0x30000000:        op_andi(emu, op);       return;        
        case 0x34000000:        op_ori(emu, op);        return;        
        case 0x38000000:        op_xori(emu, op);       return;        
        case 0x3c000000:        op_lui(emu, op);        return;        
        case 0x50000000:        op_beql(emu, op);       return;        
        case 0x54000000:        op_bnel(emu, op);       return;        
        case 0x58000000:        op_blezl(emu, op);      return;        
        case 0x80000000:        op_lb(emu, op);         return;
        case 0x84000000:        op_lh(emu, op);         return;
        case 0x88000000:        op_lwl(emu, op);        return;
        case 0x8c000000:        op_lw(emu, op);         return;
        case 0x90000000:        op_lbu(emu, op);        return;
        case 0x94000000:        op_lhu(emu, op);        return;
        case 0x98000000:        op_lwr(emu, op);        return;
        case 0xa0000000:        op_sb(emu, op);         return;
        case 0xa4000000:        op_sh(emu, op);         return;
        case 0xa8000000:        op_swl(emu, op);        return;
        case 0xac000000:        op_sw(emu, op);         return;
        case 0xb8000000:        op_swr(emu, op);        return;
        case 0xbc000000:        op_cache(emu, op);      return;
        case 0xc0000000:        op_ll(emu, op);         return;
        case 0xcc000000:        op_pref(emu, op);       return;
        case 0xe0000000:        op_sc(emu, op);         return;
        }

        switch (op & 0xfc00003f) {
        case 0x0:               op_sll(emu, op);        return;
        case 0x2:               op_srl(emu, op);        return;
        case 0x3:               op_sra(emu, op);        return;
        case 0x4:               op_sllv(emu, op);       return;
        case 0x6:               op_srlv(emu, op);       return;
        case 0x7:               op_srav(emu, op);       return;
        case 0x8:               op_jr(emu, op);         return;
        case 0x9:               op_jalr(emu, op);       return;
        case 0xc:               op_syscall(emu, op);    return;
        case 0xf:               op_sync(emu, op);       return;
        case 0x10:              op_mfhi(emu, op);       return;
        case 0x11:              op_mthi(emu, op);       return;
        case 0x12:              op_mflo(emu, op);       return;
        case 0x13:              op_mtlo(emu, op);       return;
        case 0x18:              op_mult(emu, op);       return;
        case 0x19:              op_multu(emu, op);      return;
        case 0x1a:              op_div(emu, op);        return;
        case 0x1b:              op_divu(emu, op);       return;
        case 0x20:              op_add(emu, op);        return;
        case 0x21:              op_addu(emu, op);       return;
        case 0x22:              op_sub(emu, op);        return;
        case 0x23:              op_subu(emu, op);       return;
        case 0x24:              op_and(emu, op);        return;
        case 0x25:              op_or(emu, op);         return;
        case 0x26:              op_xor(emu, op);        return;
        case 0x27:              op_nor(emu, op);        return;
        case 0x2a:              op_slt(emu, op);        return;
        case 0x2b:              op_sltu(emu, op);       return;
        case 0x36:              op_tne(emu, op);        return;
        }

        switch (op & 0xfc1f0000) {
        case 0x4000000:         op_bltz(emu, op);       return;
        case 0x4010000:         op_bgez(emu, op);       return;
        case 0x4020000:         op_bltzl(emu, op);      return;
        case 0x4030000:         op_bgezl(emu, op);      return;
        case 0x4100000:         op_bltzal(emu, op);     return;
        case 0x4110000:         op_bgezal(emu, op);     return;
        case 0x5c000000:        op_bgtzl(emu, op);      return;
        }

        switch (op & 0xffffffff) {
        case 0x42000002:        op_tlbwi(emu, op);      return;
        case 0x42000006:        op_tlbwr(emu, op);      return;
        case 0x42000008:        op_tlbp(emu, op);       return;
        case 0x42000018:        op_eret(emu, op);       return;
        }

        switch (op & 0xfc0007ff) {
        case 0xa:               op_movz(emu, op);       return;
        case 0xb:               op_movn(emu, op);       return;
        case 0x70000002:        op_mul(emu, op);        return;
        }

        switch (op & 0xffe00000) {
        case 0x40000000: op_mfc0(emu, op); return;
        case 0x40800000: op_mtc0(emu, op); return;
        }
        switch (op & 0xfe00003f) {
        case 0x42000020:op_wait(emu, op); return;
        }
        /*printf("unhandled opcode at %x -> %x\n",emu->pc,op);*/
        setExceptionCode(emu, EXC_RI);
        emu->exceptionOccured = 1;
        return;
}

