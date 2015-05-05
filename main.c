#include "mips.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

mutex_type emu_mutex;

void *runEmulator(void *p)
{
        Mips *emu = (Mips *) p;

        while (!mips_isMipsShutdown(emu)) {
                unsigned i;

                if (util_mutex_lock(emu_mutex))
                        FATAL("mutex failed lock, exiting");

                for (i = 0; i < 1024; i++)
                        mips_step(emu);

                if (util_mutex_unlock(emu_mutex))
                        FATAL("mutex failed unlock, exiting");
        }
        mips_free(emu);
        exit(0);
}

int main(int argc, char *argv[])
{
        int c;
        thread_type emu_thread;
        Mips *emu;

        if (argc < 2) {
                printf("usage: %s image.srec\n", argv[0]);
                return 1;
        }

        if (!(emu_mutex = util_mutex_create()))
                FATAL("failed to create mutex");

        if (!(emu = mips_new(64 * 1024 * 1024))) 
                FATAL("allocating emu failed.");

        if (mips_loadSrecFromFile(emu, argv[1]) != 0)
                FATAL("failed loading srec");

        if (util_ttyraw())
                FATAL("failed to configure raw mode");

        if (!(emu_thread=util_thread_alloc()))
                FATAL("calloc failed");

        if (util_thread_new((thread_type)&emu_thread, runEmulator, emu))
                FATAL("creating emulator thread failed!");

        while ((c = util_getchraw()) != EOF) { 
                if (util_mutex_lock(emu_mutex))
                        FATAL("mutex failed lock, exiting");

                uart_RecieveChar(emu, c);

                if (util_mutex_unlock(emu_mutex))
                        FATAL("mutex failed unlock, exiting");
        }
        return 0;
}
