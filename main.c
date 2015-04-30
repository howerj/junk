#include "mips.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>

pthread_mutex_t emu_mutex;

void *runEmulator(void *p)
{
        Mips *emu = (Mips *) p;

        while (emu->shutdown != 1) {
                uint64_t i;

                if (pthread_mutex_lock(&emu_mutex))
                        FATAL("mutex failed lock, exiting");

                for (i = 0; i < 1024; i++)
                        step_mips(emu);

                if (pthread_mutex_unlock(&emu_mutex))
                        FATAL("mutex failed unlock, exiting");
        }
        free_mips(emu);
        exit(0);
}

int main(int argc, char *argv[])
{
        pthread_t emu_thread;
        Mips *emu;

        if (pthread_mutex_init(&emu_mutex, NULL)) {
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

        if (pthread_create(&emu_thread, NULL, runEmulator, emu))
                FATAL("creating emulator thread failed!");

        while (1) {
                int c;
                if ((c = getchar()) == EOF)
                        return 1;

                if (pthread_mutex_lock(&emu_mutex))
                        FATAL("mutex failed lock, exiting");

                uart_RecieveChar(emu, c);

                if (pthread_mutex_unlock(&emu_mutex))
                        FATAL("mutex failed unlock, exiting");
        }
        return 0;
}
