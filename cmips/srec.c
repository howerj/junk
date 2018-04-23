#include "mips.h"
#include "internal.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct _SrecLoader {
        void *userdata;
        int (*nextChar) (void *);
        int (*isEof) (void *);
} SrecLoader;

int mips_loadSrec(Mips *emu, SrecLoader *s);

static int fnextChar(void *ud)
{
        int c = fgetc((FILE *) ud);
        if (c == EOF)
                return -1;
        return c;
}

static int fisEof(void *ud)
{
        return feof((FILE *) ud);
}

int mips_load_srec_from_file(Mips * emu, char *fname)
{
        SrecLoader loader;
        void *ud = fopen(fname, "r");
        int ret;

        if (!ud) {
                fprintf(stdout, "srec %s failed to open", fname);
                return 1;
        }

        loader.nextChar = &fnextChar;
        loader.isEof = &fisEof;
        loader.userdata = ud;
        ret = mips_loadSrec(emu, &loader);
        fclose((FILE *) ud);
        return ret;
}

static int snextChar(void *ud)
{
        int c = **(char **)ud;
        (*(char **)ud)++;
        if (c == 0)
                return -1;
        return c;
}

static int sisEof(void *ud)
{
        return (**(char **)ud) == 0;
}

int mips_load_srec_from_string(Mips * emu, char *srec)
{
        void *ud = (void *)&srec;
        int ret;

        SrecLoader loader;
        loader.nextChar = &snextChar;
        loader.isEof = &sisEof;
        loader.userdata = ud;
        ret = mips_loadSrec(emu, &loader);
        return ret;
}

/*a minimized version of address translation which checks fixed map ranges*/
static void writeb(Mips * emu, uint32_t addr, uint8_t v)
{
        uint32_t word;
        unsigned offset;
        int shamt;

        if (addr >= 0x80000000 && addr <= 0x9fffffff)
                addr -= 0x80000000;
        else if (addr >= 0xa0000000 && addr <= 0xbfffffff) 
                addr -= 0xa0000000;
        else
                return;

        if (addr > emu->pmemsz)
                return;

        word = emu->mem[addr / 4];
        offset = addr % 4;
        shamt = 8 * (3 - offset);
        word = (word & ~(0xff << shamt)) | (v << shamt);
        emu->mem[addr / 4] = word;
}

static int isHexChar(char c)
{
        if (c >= '0' && c <= '9')
                return 1;
        if (c >= 'a' && c <= 'f')
                return 1;
        if (c >= 'A' && c <= 'F')
                return 1;
        return 0;
}

static int srecReadType(SrecLoader * loader)
{
        int c = loader->nextChar(loader->userdata);
        if (c == -1)
                return -1;  /* 0 type srecord will trigger a skip, which will terminate*/
        if (c != 'S')
                return -2;
        c = loader->nextChar(loader->userdata) - '0';
        if (c >= 0 && c <= 9)
                return c;
        return -2;
}

static int srecReadByte(SrecLoader * loader, uint8_t * count)
{
        char chars[3];
        int i;

        for (i = 0; i < 2; i++) {
                chars[i] = loader->nextChar(loader->userdata);
                if (!isHexChar(chars[i]))
                        return 1;
        }
        chars[2] = 0;
        *count = (uint8_t) strtoul(&chars[0], 0, 16);
        return 0;
}

static int srecReadAddress(SrecLoader * loader, uint32_t * addr)
{
        char chars[9];
        int i;

        for (i = 0; i < 8; i++) {
                chars[i] = loader->nextChar(loader->userdata);
                if (!isHexChar(chars[i]))
                        return 1;
        }
        chars[8] = 0;

        *addr = strtoul(&chars[0], 0, 16);
        return 0;
}

static void srecSkipToNextLine(SrecLoader * loader)
{
        while (1) {
                int c = loader->nextChar(loader->userdata);
                if (c == -1 || c == '\n') {
                        break;
                }
        }
}

static int srecLoadData(SrecLoader * loader, Mips * emu, uint32_t addr, uint32_t count)
{
        uint8_t b;
        while (count--) {
                if (srecReadByte(loader, &b)) {
                        return 1;
                }

                writeb(emu, addr++, b);
        }
        return 0;
}

int mips_loadSrec(Mips * emu, SrecLoader * loader)
{
        uint32_t addr;
        uint8_t count;

        while (!loader->isEof(loader->userdata)) {
                switch (srecReadType(loader)) {
                case -1: break; /*EOF*/
                case 0:  srecSkipToNextLine(loader); break;
                case 3:
                        if (srecReadByte(loader, &count))
                                FATAL("srecLoader: failed to parse bytecount.\n");
                        if (srecReadAddress(loader, &addr))
                                FATAL("srecLoader: failed to parse address.\n");
                        if (srecLoadData(loader, emu, addr, count - 5))
                                FATAL("srecLoader: failed to load data.\n");
                        srecSkipToNextLine(loader);
                        break;
                case 7:
                        if (srecReadByte(loader, &count))
                                FATAL("srecLoader: failed to parse bytecount.\n");
                        if (srecReadAddress(loader, &addr))
                                FATAL("srecLoader: failed to parse address.\n");
                        emu->pc = addr;
                        srecSkipToNextLine(loader);
                        break;
                default:
                        FATAL("Bad/Unsupported srec type\n");
                }
        }

        return 0;
}
