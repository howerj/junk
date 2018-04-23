/* solitaire.c */

/*
 * The Solitaire encryption algorithm programmed in C.
 * solitaire encryption system by Bruce Schneier
 * based on a deck of cards
 * See <http://www.counterpane.com/solitaire.html> for details.
 * programming by Lloyd Miller, Sept 2000
*/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 *   usage :
 *       to encrypt
 *              solitair -e key <plaintext >cyphertext
 *       to decrypt
 *              solitair -d key <cyphertext >plaintext
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

void usage(void)
{
  fprintf(stderr,
	  "usage:\n"
	  "       to encrypt\n"
	  "              solitair -e key <plaintext >cyphertext\n"
	  "       to decrypt\n"
	  "              solitair -d key <cyphertext >plaintext\n");
  exit(EXIT_FAILURE);
}

int getalpha(void)
{
  int a;

  while (1) {
    a = getchar();
    if (a == EOF) return 0;
    if (a >= 'A' && a <= 'Z')
      return a;
    if (a >= 'a' && a <= 'z')
      return a - 'a' + 'A';
  }
}

int findit(char *deck, int val, int siz)
{
  int i = 0;
  while(siz--)
    {
      if (*deck == val)
	return i;
      deck++;
      i++;
    }
  return -1;
}

int step(char *deck)
{
  int c;
  int b;
  int a;
  char tmp[54];

  do {

    /* step 1 */

    c = findit(deck, 53, 54);
    if (c < 53)
      {
	a = c + 1;
	deck[c] = deck[a];
      }
    else
      {
	for (a = 53; a > 1; a--)
	  deck[a] = deck[a - 1];
	a = 1;
      }
    deck[a] = 53;

    /* step 2 */

    b = findit(deck, 54, 54);
    if (b < 52)
      {
	c = b + 1;
	deck[b] = deck[c];
	if (a == c)
	  a = b;
	b = c + 1;
	deck[c] = deck[b];
	if (a == b)
	  a = c;
      }
    else
      {
	c = b;
	b = b - 51;
	for (; c > b; c--)
	  {
	    deck[c] = deck[c - 1];
	    if (a == c - 1)
	      a = c;
	  }
      }
    deck[b] = 54;

    /* step 3 */

    if (a > b)
      {
	c = a;
	a = b;
	b = c;
      }
    tmp[53] = deck[b++];

    c = 0;
    while (b < 54)
      tmp[c++] = deck[b++];

    b = a;
    while (deck[b] != tmp[53])
      tmp[c++] = deck[b++];

    tmp[c++] = tmp[53];
    b = 0;
    while (b < a)
      tmp[c++] = deck[b++];

    assert(c == 54);

    /* step 4 */

    b = tmp[53];
    if (b == 54)
      b = 53;

    a = 0;
    for (c = b; c < 53; c++)
      deck[a++] = tmp[c];
    for (c = 0; c < b; c++)
      deck[a++] = tmp[c];
    assert(a == 53);

    deck[53] = tmp[53];

    /* step 5 */

    a = deck[0];
    if (a == 54)
      a = 53;
    a = deck[a];
  } while (a > 52);
  return (a - 1) % 26 + 1;
}

int sumchar(int a, int b)
{
  return 'A' + (toupper(a) - 'A' + b + 26) % 26;
}

int main(int argc, char **argv)
{
  char deck[54];
  int i, j;
  int mode = 0;  /* default mode is encode, non-zero is decode */

  for (i = 0; i < 54; i++)
    deck[i] = i + 1;

  if (argc > 1 && argv[1][0] == '-')
    {
      if(argv[1][1] == 'd')
	{
	  mode = 1;
	}
      else if (argv[1][1] == 'e')
	{
	  mode = 0;
	}
      else usage();
    }
  else usage();

  argc--;
  argv++;

  /* do key */
  while (argc-- > 1)
    {
      char *ap = *++argv;
      while (*ap)
	{
	  int c = *ap++;
	  char tmp[53];

	  if (c >= 'a' && c <= 'z')
	    c = c - 'a' + 'A';
	  if (c >= 'A' && c <= 'Z')
	    {
	      c = c - 'A' + 1;
	      step(deck);
	      i = 0;
	      for (j = c; j < 53; j++)
		tmp[i++] = deck[j];
	      for (j = 0; j < c; j++)
		tmp[i++] = deck[j];
	      for (j = 0; j < 53; j++)
		deck[j] = tmp[j];
	    }
	}
    }

  j = 0;
  while ((i = getalpha()) != 0)
    {
      putchar(mode ? sumchar(i, -step(deck)) : sumchar(i, step(deck)));
      j++;
      if (j % 5 == 0)
	{
	  if (j == 50)
	    {
	      j = 0;
	      putchar('\n');
	    }
	  else
	    putchar (' ');
	}
    }

  j = j % 5;
  if (j)
    while (j < 5)
      {
	i = 'X';
	putchar(mode ? sumchar(i, -step(deck)) : sumchar(i, step(deck)));
	j++;
      }

  printf("\n");

  return EXIT_SUCCESS;
}

/* end of file */