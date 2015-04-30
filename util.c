#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>

#include <termios.h>
#include <unistd.h>

/* Warnings and error messages */

void util_fatal(char *msg, char *file, unsigned line)
{
        util_warn(msg, file, line);
        exit(EXIT_FAILURE);
}

void util_warn(char *msg, char *file, unsigned line)
{
        assert(msg && file);
        fprintf(stderr, "(%s %s %u)\n", msg, file, line);
}

/* Terminal Settings */

struct termios oldtermios, newtermios;
static int terminalSettingsHaveChanged = 0;
static void util_restoreTermAtExit(void)
{
        if(terminalSettingsHaveChanged)
                tcsetattr(STDIN_FILENO, TCSANOW, &oldtermios);
}

void util_restoreTermAtSignal(int sig)
{       UNUSED(sig);
        util_restoreTermAtExit();
        abort();
}

int util_ttyraw(void)
{
        int fd = STDIN_FILENO;
        /* Set terminal mode as follows:
           Noncanonical mode - turn off ICANON.
           Turn off signal-generation (ISIG)
           including BREAK character (BRKINT).
           Turn off any possible preprocessing of input (IEXTEN).
           Turn ECHO mode off.
           Disable CR-to-NL mapping on input.
           Disable input parity detection (INPCK).
           Disable stripping of eighth bit on input (ISTRIP).
           Disable flow control (IXON).
           Use eight bit characters (CS8).
           Disable parity checking (PARENB).
           Disable any implementation-dependent output processing (OPOST).
           One byte at a time input (MIN=1, TIME=0). */
        if (tcgetattr(fd, &oldtermios) < 0)
                return 1;
        newtermios = oldtermios;

        /* OK, why IEXTEN? If IEXTEN is on, the DISCARD character
           is recognized and is not passed to the process. This 
           character causes output to be suspended until another
           DISCARD is received. The DSUSP character for job control,
           the LNEXT character that removes any special meaning of
           the following character, the REPRINT character, and some
           others are also in this category. */
        newtermios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

        /* If an input character arrives with the wrong parity, then INPCK
           is checked. If this flag is set, then IGNPAR is checked
           to see if input bytes with parity errors should be ignored.
           If it shouldn't be ignored, then PARMRK determines what
           character sequence the process will actually see.

           When we turn off IXON, the start and stop characters can be read. */
        newtermios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

        /* CSIZE is a mask that determines the number of bits per byte.
           PARENB enables parity checking on input and parity generation
           on output. */
        newtermios.c_cflag &= ~(CSIZE | PARENB);

        /* Set 8 bits per character. */
        newtermios.c_cflag |= CS8;

        /* This includes things like expanding tabs to spaces. */
        newtermios.c_oflag &= ~(OPOST);

        newtermios.c_cc[VMIN] = 1;
        newtermios.c_cc[VTIME] = 0;

        /* You tell me why TCSAFLUSH. */
        if (tcsetattr(fd, TCSAFLUSH, &newtermios) < 0)
                return -1;

        terminalSettingsHaveChanged = 1;

        atexit(util_restoreTermAtExit);

        if(signal(SIGTERM, util_restoreTermAtSignal))
                FATAL("Installation of signal handler failed.");

        return 0;
}


