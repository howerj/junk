#include "mips.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __WIN32

#elif __unix__
#include <pthread.h>
#include <unistd.h>
#endif 

mutex_type emu_mutex;

void *runEmulator(void *p)
{
        Mips *emu = (Mips *) p;

        while (emu->shutdown != 1) {
                uint64_t i;

                if (util_mutex_lock(emu_mutex))
                        FATAL("mutex failed lock, exiting");

                for (i = 0; i < 1024; i++)
                        step_mips(emu);

                if (util_mutex_unlock(emu_mutex))
                        FATAL("mutex failed unlock, exiting");
        }
        free_mips(emu);
        exit(0);
}

int main(int argc, char *argv[])
{
        thread_type emu_thread;
        Mips *emu;

        if (!(emu_mutex = util_mutex_create())) {
                puts("failed to create mutex");
                return 1;
        }

        if (argc < 2) {
                printf("usage: %s image.srec\n", argv[0]);
                return 1;
        }

        if (!(emu = new_mips(64 * 1024 * 1024))) 
                FATAL("allocating emu failed.");

        if (loadSrecFromFile_mips(emu, argv[1]) != 0)
                FATAL("failed loading srec");

        if (util_ttyraw())
                FATAL("failed to configure raw mode");

        if (!(emu_thread=util_thread_alloc()))
                FATAL("calloc failed");

        if (util_thread_new((thread_type)&emu_thread, runEmulator, emu))
                FATAL("creating emulator thread failed!");

        while (1) { 
                int c;
                if ((c = util_getchraw()) == EOF)
                        return 1;

                if (util_mutex_lock(emu_mutex))
                        FATAL("mutex failed lock, exiting");

                uart_RecieveChar(emu, c);

                if (util_mutex_unlock(emu_mutex))
                        FATAL("mutex failed unlock, exiting");
        }
        return 0;
}
