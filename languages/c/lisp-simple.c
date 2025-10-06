#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

// We'll have 128 tokens. Each token can be up to 32 characters long.
char token[128][32];

int lexer(char* input) {
  int ii = 0; // input index
  int ti = 0; // token index

  // Loop thru the whole string
  while(input[ii] != '\0')
    switch(input[ii]) {
    // Ignore whitespace and newlines
    case ' ':
    case '\n':
      ++ii;
      break;

    // Turn a left parenthesis into a token.
    case '(':
      token[ti][0] = '(';
      token[ti][1] = '\0';
      ++ii;
      ++ti;
      break;

    // Turn a right parenthesis into a token.
    case ')':
      token[ti][0] = ')';
      token[ti][1] = '\0';
      ++ii;
      ++ti;
      break;

    // Anything else is a symbol
    default:
      for(int i = 0;; ++i) {
	if(input[ii] != ' '  &&
	   input[ii] != ')'  &&
	   input[ii] != '('  &&
	   input[ii] != '\n' &&
	   input[ii] != '\0') {
	  token[ti][i] = input[ii++];
	}
        else {
	  token[ti][i] = '\0';
	  break;
	}
      }
      ++ti;
      break;
    }
  return ti;
}


int curtok;

char* nexttok() {
  return token[curtok++];
}

char* peektok() {
  return token[curtok];
}


typedef struct {
  void* car;
  void* cdr;
} Pair;

Pair text[256];
Pair* textptr;

Pair* tcons(void* x, void* y) {
  textptr->car = x;
  textptr->cdr = y;
  return textptr++;
}

int istext(void* x) {
  return x >= (void*)&text &&
         x <  (void*)&text[256];
}


void* read(char* ln);
void* read_exp();
void* read_list();

void* read(char* ln) {
  // Initialize the lexer and list memory.
  curtok = 0;
  textptr = text;

  lexer(ln);
  return read_exp();
}

void* read_exp() {
  char* tok = nexttok();
  if(tok[0] == '(')
    return read_list();
  else
    return tok;
}

void* read_list() {
  char* tok = peektok();
  if(tok[0] == ')') {
    nexttok();
    return NULL;
  }
  else {
    void* fst = read_exp();
    void* snd = read_list();
    return tcons(fst, snd);
  }
}


void print(void* exp);
void print_exp(void* exp);
void print_list(Pair* list);

void print(void* exp) {
  print_exp(exp);
  printf("\n");
}

void print_exp(void* exp) {
  if(istext(exp)) {
    printf("(");
    print_list(exp);
  }
  else
    printf("%s", (char*)exp);
}

void print_list(Pair* list) {
  if(list->cdr == NULL) {
    print_exp(list->car);
    printf(")");
  }
  else {
    print_exp(list->car);
    printf(" ");
    print_list(list->cdr);
  }
}


typedef struct Text {
  char* car;
  struct Text* cdr;
} Text;

char ret[32];

void* eval(void* exp);
void* eval_exp(void* exp);

void* eval(void* exp) {
  return eval_exp(exp);
}

void* eval_exp(void* exp) {
  if (istext(exp)) {
    Text* text = exp;
    if (strcmp("=", text->car) == 0) {
      void* left = eval_exp(text->cdr->car);
      void* right = eval_exp(text->cdr->cdr->car);
      if (left && right)
        return strcmp(left, right) == 0 ? "#t" : "#f";
      else
        return left == right ? "#t" : "#f";
    }
    else if (strcmp("if", text->car) == 0) {
      void* conditional = eval_exp(text->car);
      if (strcmp("#t", text->cdr->car) == 0)
        return eval_exp(text->cdr->cdr->car);
      else
        return eval_exp(text->cdr->cdr->cdr->car);
    }
    else if (strcmp("+", text->car) == 0) {
      void* left = eval_exp(text->cdr->car);
      void* right = eval_exp(text->cdr->cdr->car);
      sprintf(ret, "%d", atoi(left) + atoi(right));
      return ret;
    }
    else if (strcmp("-", text->car) == 0) {
      void* left = eval_exp(text->cdr->car);
      void* right = eval_exp(text->cdr->cdr->car);
      sprintf(ret, "%d", atoi(left) - atoi(right));
      return ret;
    }
    else if (strcmp("*", text->car) == 0) {
      void* left = eval_exp(text->cdr->car);
      void* right = eval_exp(text->cdr->cdr->car);
      sprintf(ret, "%d", atoi(left) * atoi(right));
      return ret;
    }
    else if (strcmp("/", text->car) == 0) {
      void* left = eval_exp(text->cdr->car);
      void* right = eval_exp(text->cdr->cdr->car);
      sprintf(ret, "%d", atoi(left) / atoi(right));
      return ret;
    }
  }
  return exp;
}


int main(int argc, char** argv) {
  printf("Lisp REPL\n\n");
  printf(">> ");

  char buffer[256];
  while(fgets(buffer, 256, stdin)) {
    print(eval(read(buffer)));
    printf(">> ");
  }
  return 0;
}

// https://gist.github.com/swatson555/13da367e38bcb3feb344aa2c5f5ae36b
