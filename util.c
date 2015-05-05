#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>

#ifdef __WIN32
#include <windows.h>
#include <conio.h>
#elif __unix__
#include <termios.h>
#include <pthread.h>
#include <unistd.h>
#else
#error "Not Unix or Windows :C"
#endif

/* Warnings and error messages */

void util_fatal(char *msg, char *file, unsigned line)
{
        util_warn(msg, file, line);
        exit(EXIT_FAILURE);
}

void util_warn(char *msg, char *file, unsigned line)
{
        assert(msg && file);
        fprintf(stderr, "(error \"%s\" %s %u)\n", msg, file, line);
}

/* Terminal Settings */

#ifdef __WIN32
void util_restoreTermAtSignal(int sig) { UNUSED(sig); }
int util_ttyraw(void) { return 0; }
int util_getchraw(void) { return _getch(); /*from CONIO.H*/ }
#elif __unix__
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

int util_getchraw(void) { return getchar(); }

#endif

/*locks and threads; needs work.*/

mutex_type util_mutex_create(void)
{
        mutex_type p;
#ifdef __WIN32 
        p = calloc(1, sizeof(CRITICAL_SECTION));
        if(!p)
                return NULL;
        InitializeCriticalSection(p);
        return p; 
#elif __unix__
        p = calloc(1, sizeof(pthread_mutex_t));
        if(!p)
                return NULL;
        pthread_mutex_init(p, NULL);
        return p;
#endif
}

int util_mutex_lock(mutex_type m) 
{
#ifdef __WIN32
        EnterCriticalSection((LPCRITICAL_SECTION)m);
        return 0; 
#elif __unix__
        return pthread_mutex_lock((pthread_mutex_t*)m); 
#endif
}

int util_mutex_unlock(mutex_type m) 
{ 
#ifdef __WIN32
        LeaveCriticalSection((LPCRITICAL_SECTION)m);
        return 0;
#elif __unix__
        return pthread_mutex_unlock((pthread_mutex_t*)m); 
#endif
}

/*The thread section needs rewriting, it implements just enough to
 *get the emulator this util library was written for and nothing more*/

thread_type util_thread_alloc(void)
{
#ifdef __WIN32
        return calloc(1,1);
#else
        return calloc(1, sizeof(pthread_t));
#endif
}

int util_thread_new(thread_type thread, void *(*routine) (void*), void *args) 
{  
#ifdef __WIN32
        return CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)routine,args,0,0)?0:-1;
#else
        return pthread_create(thread, NULL, routine, args);
#endif
}

/*do not need join and the like yet*/
