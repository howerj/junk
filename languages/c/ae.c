/* <https://groups.google.com/forum/#!topic/comp.editors/NMhsctXnQ6I>
 *        ae.c                Anthony's Editor  Mar '92
 *
 *        Public Domain 1991, 1992 by Anthony Howe.  All rights released.
 */

#include <ctype.h>
#include <curses.h>
#include <stdio.h>
#include <string.h>

#ifndef BUF
#define BUF        32767
#endif /* BUF */

#ifndef HUP
#define HUP        "ae.hup"
#endif /* HUP */

typedef struct keytable_t {
        int key;
        void (*func)();
} keytable_t;

int done;
int row, col;
int index, page, epage;
int input;
char buf[BUF];
char *ebuf;
char *gap = buf;
char *egap;
char *filename;
keytable_t *table;

/*
 *        The following assertions must be maintained.
 *
 *        o  buf <= gap <= egap <= ebuf
 *                If gap == egap then the buffer is full.
 *
 *        o  point = ptr(index) and point < gap or egap <= point
 *
 *        o  page <= index < epage
 *
 *        o  0 <= index <= pos(ebuf) <= BUF
 *
 *
 *        Memory representation of the file:
 *
 *                low        buf  -->+----------+
 *                                |  front   |
 *                                | of file  |
 *                        gap  -->+----------+<-- character not in file
 *                                |   hole   |
 *                        egap -->+----------+<-- character in file
 *                                |   back   |
 *                                | of file  |
 *                high        ebuf -->+----------+<-- character not in file
 *
 *
 *        point & gap
 *
 *        The Point is the current cursor position while the Gap is the
 *        position where the last edit operation took place. The Gap is
 *        ment to be the cursor but to avoid shuffling characters while
 *        the cursor moves it is easier to just move a pointer and when
 *        something serious has to be done then you move the Gap to the
 *        Point.
 *
 *
 *        Use of stdio for portability.
 *
 *        Stdio will handle the necessary conversions of text files to
 *        and from a machine specific format.  Things like fixed length
 *        records; CRLF mapping into <newline> (\n) and back again;
 *        null padding; control-Z end-of-file marks; and other assorted
 *        bizare issues that appear on many unusual machines.
 *
 *        AE is meant to be simple in both code and usage.  With that
 *        in mind certain assumptions are made.
 *
 *        Reading:  If a file can not be opened, assume that it is a
 *        new file.  If an error occurs, fall back to a safe state and
 *        assume an empty file.  fread() is typed size_t which is an
 *        unsigned number.  Zero (0) would indicate a read error or an
 *        empty file.  A return value less than BUF is alright, since
 *        we asked for the maximum allowed.
 *
 *        Writing:  If the file can not be opened or a write error occurs,
 *        then we scramble and save the user's changes in a file called
 *        ae.hup.  If ae.hup fails to open or a write error occurs, then
 *        we assume that shit happens.
 *
 */

int adjust();
int nextline();
int pos();
int prevline();
int save();
char *ptr();

void backsp();
void bottom();
void delete();
void display();
void down();
void file();
void insert();
void insert_mode();
void left();
void lnbegin();
void lnend();
void movegap();
void pgdown();
void pgup();
void redraw();
void right();
void quit();
void flip();
void top();
void up();
void wleft();
void wright();

#undef CTRL
#define CTRL(x)                ((x) & 0x1f)

keytable_t modeless[] = {
        { KEY_LEFT, left },
        { KEY_RIGHT, right },
        { KEY_DOWN, down },
        { KEY_UP, up },
        { CTRL('w'), wleft },
        { CTRL('e'), wright },
        { CTRL('n'), pgdown },
        { CTRL('p'), pgup },
        { CTRL('a'), lnbegin },
        { CTRL('d'), lnend },
        { CTRL('t'), top },
        { CTRL('b'), bottom },
        { KEY_BACKSPACE, backsp },
        { '\b', backsp },
        { KEY_DC, delete },
        { CTRL('f'), file },
        { CTRL('r'), redraw },
        { CTRL('\\'), quit },
        { CTRL('z'), flip },
        { 0, insert }
};

keytable_t modual[] = {
        { 'h', left },
        { 'j', down },
        { 'k', up },
        { 'l', right },
        { 'H', wleft },
        { 'J', pgdown },
        { 'K', pgup },
        { 'L', wright },
        { '[', lnbegin },
        { ']', lnend },
        { 't', top },
        { 'b', bottom },
        { 'i', insert_mode },
        { 'x', delete },
        { 'W', file },
        { 'R', redraw },
        { 'Q', quit },
        { 'Z', flip },
        { 0, movegap }
};


char *
ptr(offset)
int offset;
{
        if (offset < 0)
                return (buf);
        return (buf+offset + (buf+offset < gap ? 0 : egap-gap));
}

int
pos(pointer)
char *pointer;
{
        return (pointer-buf - (pointer < egap ? 0 : egap-gap));
}

void
top()
{
        index = 0;
}

void
bottom()
{
        epage = index = pos(ebuf);
}

void
quit()
{
        done = 1;
}

void
redraw()
{
        clear();
        display();
}

void
movegap()
{
        char *p = ptr(index);
        while (p < gap)
                *--egap = *--gap;
        while (egap < p)
                *gap++ = *egap++;
        index = pos(egap);
}

int
prevline(offset)
int offset;
{
        char *p;
        while (buf < (p = ptr(--offset)) && *p != '\n')
                ;
        return (buf < p ? ++offset : 0);
}

int
nextline(offset)
int offset;
{
        char *p;
        while ((p = ptr(offset++)) < ebuf && *p != '\n')        
                ;
        return (p < ebuf ? offset : pos(ebuf));
}

int
adjust(offset, column)
int offset, column;
{
        char *p;
        int i = 0;
        while ((p = ptr(offset)) < ebuf && *p != '\n' && i < column) {
                i += *p == '\t' ? 8-(i&7) : 1;
                ++offset;
        }
        return (offset);
}

void
left()
{
        if (0 < index)
                --index;
}

void
right()
{
        if (index < pos(ebuf))
                ++index;
}

void
up()
{
        index = adjust(prevline(prevline(index)-1), col);
}

void
down()
{
        index = adjust(nextline(index), col);
}

void
lnbegin()
{
        index = prevline(index);
}

void
lnend()
{
        index = nextline(index);
        left();
}

void
wleft()
{
        char *p;
        while (!isspace(*(p = ptr(index))) && buf < p)
                --index;
        while (isspace(*(p = ptr(index))) && buf < p)
                --index;
}

void
pgdown()
{
        page = index = prevline(epage-1);
        while (0 < row--)
                down();
        epage = pos(ebuf);
}

void
pgup()
{
        int i = LINES;
        while (0 < --i) {
                page = prevline(page-1);
                up();
        }
}

void
wright()
{
        char *p;
        while (!isspace(*(p = ptr(index))) && p < ebuf)
                ++index;
        while (isspace(*(p = ptr(index))) && p < ebuf)
                ++index;
}

void
insert()
{
        movegap();
        if (gap < egap)
                *gap++ = input == '\r' ? '\n' : input;
        index = pos(egap);
}

void
insert_mode()
{
        int ch;
        movegap();
        while ((ch = getch()) != '\f') {
                if (ch == '\b') {
                        if (buf < gap)
                                --gap;
                } else if (gap < egap) {
                        *gap++ = ch == '\r' ? '\n' : ch;
                }
                index = pos(egap);
                display();
        }
}

void
backsp()
{
        movegap();
        if (buf < gap)
                --gap;
        index = pos(egap);
}

void
delete()
{
        movegap();
        if (egap < ebuf)
                index = pos(++egap);
}

void
file()
{
        if (!save(filename))
                save(HUP);
}

int
save(fn)
char *fn;
{
        FILE *fp;
        int i, ok;
        size_t length;
        fp = fopen(fn, "w");
        if ((ok = fp != NULL)) {
                i = index;
                index = 0;
                movegap();
                length = (size_t) (ebuf-egap);
                ok = fwrite(egap, sizeof (char), length, fp) == length;
                (void) fclose(fp);
                index = i;
        }
        return (ok);
}

void
flip()
{
        table = table == modual ? modeless : modual;
}

void
display()
{
        char *p;
        int i, j;
        if (index < page)
                page = prevline(index);
        if (epage <= index) {
                page = nextline(index);
                i = page == pos(ebuf) ? LINES-2 : LINES;
                while (0 < i--)
                        page = prevline(page-1);
        }
        move(0, 0);
        i = j = 0;
        epage = page;
        while (1) {
                if (index == epage) {
                        row = i;
                        col = j;
                }
                p = ptr(epage);
                if (LINES <= i || ebuf <= p)
                        break;
                if (*p != '\r') {
                        addch(*p);
                        j += *p == '\t' ? 8-(j&7) : 1;
                }
                if (*p == '\n' || COLS <= j) {
                        ++i;
                        j = 0;
                }
                ++epage;
        }
        clrtobot();
        if (++i < LINES)
                mvaddstr(i, 0, "<< EOF >>");
        move(row, col);
        refresh();
}

int
main(argc, argv)
int argc;
char **argv;
{
        FILE *fp;
        char *p = *argv;
        int i = (int) strlen(p);
        egap = ebuf = buf + BUF;
        if (argc < 2)
                return (2);
        /* Find basename. */
        while (0 <= i && p[i] != '\\' && p[i] != '/')
                --i;
        p += i+1;
        if (strncmp(p, "ae", 2) == 0 || strncmp(p, "AE", 2) == 0)
                table = modual;
        else if (strncmp(p, "ea", 2) == 0 || strncmp(p, "EA", 2) == 0)
                table = modeless;
        else
                return (2);
        if (initscr() == NULL)
                return (3);
        raw();
        noecho();
        idlok(stdscr, 1);
        keypad(stdscr, 1);
        fp = fopen(filename = *++argv, "r");
        if (fp != NULL) {
                gap += fread(buf, sizeof (char), (size_t) BUF, fp);
                fclose(fp);
        }
        top();
        while (!done) {
                display();
                i = 0;
                input = getch();
                while (table[i].key != 0 && input != table[i].key)
                        ++i;
                (*table[i].func)();
        }
        endwin();
        return (0);
}

