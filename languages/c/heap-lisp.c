// https://news.ycombinator.com/item?id=45491609
// https://gist.github.com/swatson555/8cc36d8d022d7e5cc44a5edb2c4f7d0b

/* Heap based virtual machine described in section 3.4 of Three Implementation Models for Scheme, Dybvig
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

char token[128][32];

int lexer(char* input) {
  int ii = 0; // input index
  int ti = 0; // token index

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

    // Turn an apostrophe into a token.
    case '\'':
      token[ti][0] = '\'';
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


typedef struct Pair {
  void* car;
  void* cdr;
} Pair;

typedef struct Text {
  char* car;
  struct Text* cdr;
} Text;

Pair text[1280];
Pair* textptr;

int istext(void* x) {
  return x >= (void*)&text &&
         x <  (void*)&text[1280];
}

Pair* cons(void* x, void* y) {
  assert(istext(textptr));
  textptr->car = x;
  textptr->cdr = y;
  return textptr++;
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
  if (tok[0] == '(' && peektok()[0] == ')') {
    nexttok();
    return NULL;
  }
  else if (tok[0] == '\'')
    return cons("quote", cons(read_exp(), NULL));
  else if (tok[0] == '(')
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
  else if(tok[0] == '.') {
    nexttok();
    tok = read_exp();
    nexttok();    
    return tok;
  }
  else {
    void* fst = read_exp();
    void* snd = read_list();
    return cons(fst, snd);
  }
}

void print(void* exp);
void print_exp(void* exp);
void print_list(Pair* list);
void print_cons(Pair* pair);

void print(void* exp) {
  print_exp(exp);
  printf("\n");
}

void print_exp(void* exp) {
  if (istext(exp)) {
    Pair* pair = exp;
    if(!istext(pair->cdr) && pair->cdr != NULL) {
      printf("(");
      print_cons(exp);
      printf(")");
    }
    else {
      printf("(");
      print_list(exp);
    }
  }
  else
    printf("%s", exp ? (char*)exp : "()");
}

void print_list(Pair* list) {
  if (list->cdr == NULL) {
    print_exp(list->car);
    printf(")");
  }
  else {
    if(!istext(list->cdr) && list->cdr != NULL) {
      print_cons(list);
      printf(")");
    }
    else {
      print_exp(list->car);
      printf(" ");
      print_list(list->cdr);
    }
  }
}

void print_cons(Pair* pair) {
  print_exp(pair->car);
  printf(" . ");
  print_exp(pair->cdr);
}


Pair* compile(void* exp, void* next) {
  if (istext(exp)) {
    Text* p = exp;
    if (strcmp(p->car, "quote") == 0) {
      return cons("constant", cons(p->cdr->car, cons(next, NULL)));
    }
    else if (strcmp(p->car, "lambda") == 0) {
      return cons("close", cons(p->cdr->car, cons(compile(p->cdr->cdr->car, cons("return", NULL)), cons(next, NULL))));
    }
    else if (strcmp(p->car, "if") == 0) {
      return compile(p->cdr->car, cons("test", cons(compile(p->cdr->cdr->car, next),
                                                    cons(compile(p->cdr->cdr->cdr->car, next),
                                                         NULL))));
    }
    else if (strcmp(p->car, "set!") == 0) {
      return compile(p->cdr->cdr->car, cons("assign", cons(p->cdr->car, cons(next, NULL))));
    }
    else if (strcmp(p->car, "call/cc") == 0) {
      void* c = cons("conti", cons(cons("argument", cons(compile(p->cdr->car, cons("apply", NULL)), NULL)), NULL));
      Text* n = next;
      if (strcmp(n->car, "return") == 0)
        return c;
      else
        return cons("frame", cons(next, cons(c, NULL)));
    }
    else {
      Pair* args = (Pair*)p->cdr;
      void* c = compile(p->car, cons("apply", NULL));
      while (args) {
        c = compile(args->car, cons("argument", cons(c, NULL)));
        args = args->cdr;
      }
      Text* n = next;
      if (strcmp(n->car, "return") == 0)
        return c;
      else
        return cons("frame", cons(next, cons(c, NULL)));
    }
  }
  else if(isdigit(*((char*)exp))) { // a number
    return cons("constant", cons(exp, cons(next, NULL)));
  }
  else if(strcmp(exp, "#t") == 0) { // a boolean
    return cons("constant", cons(exp, cons(next, NULL)));
  }
  else if(strcmp(exp, "#f") == 0) { // a boolean
    return cons("constant", cons(exp, cons(next, NULL)));
  }
  else { // a symbol
    return cons("refer", cons(exp, cons(next, NULL)));
  }
}

void* get(void* env, char* var) {
  Pair* e = env;
  while(env) {
    Pair* cur = e->car;
    Pair* vars = cur->car;
    Pair* vals = cur->cdr;
    while (vars && vals) {
      if (strcmp(vars->car, var) == 0)
        return vals->car;
      vars = vars->cdr;
      vals = vals->cdr;
    }
    e = e->cdr;
  }
  fprintf(stderr, "No definition in environment for %s.\n", var);
  assert(0);
}

void set(void* env, char* var, char* val) {
  void* ref = get(env, var);
  ref = val;
}

void* extend(void* env, void* vars, void* vals) {
  return cons(cons(vars, vals), env);
}

void* callframe(void* next, void* env, void* rib, void* stack) {
  return cons(next, cons(env, cons(rib, cons(stack, NULL))));
}

void* closure(void* body, void* env, void* vars) {
  return cons(body, cons(env, cons(vars, NULL)));
}

void* continuation(void* stack) {
  return closure(cons("nuate", cons(stack, cons("v", NULL))), NULL, cons("v", NULL));
}


void* accum;
void* next;
void* env;
void* rib;
void* stack;

void virtmach() {
  Text* n = next;
  if (strcmp(n->car, "halt") == 0) {
  }
  else if (strcmp(n->car, "refer") == 0) {
    accum = get(env, n->cdr->car);
    next = n->cdr->cdr->car;
    return virtmach();
  }
  else if (strcmp(n->car, "constant") == 0) {
    accum = n->cdr->car;
    next = n->cdr->cdr->car;
    return virtmach();
  }
  else if (strcmp(n->car, "close") == 0) {
    void* vars = n->cdr->car;
    void* body = n->cdr->cdr->car;
    void* x = n->cdr->cdr->cdr->car;
    accum = closure(body, env, vars);
    next = x;
    return virtmach();
  }
  else if (strcmp(n->car, "test") == 0) {
    void* consequent = n->cdr->car;
    void* alternate = n->cdr->cdr->car;
    next = strcmp(accum, "#f") == 0 ? alternate : consequent;
    return virtmach();
  }
  else if (strcmp(n->car, "assign") == 0) {
    set(env, n->cdr->car, accum);
    next = n->cdr->cdr->car;
    return virtmach();
  }
  else if (strcmp(n->car, "conti") == 0) {
    accum = continuation(stack);
    next = n->cdr->car;
    return virtmach();
  }
  else if (strcmp(n->car, "nuate") == 0) {
    stack = n->cdr->car;
    accum = get(env, n->cdr->cdr->car);
    next = cons("return", NULL);
    return virtmach();
  }
  else if (strcmp(n->car, "frame") == 0) {
    stack = callframe(n->cdr->car, env, rib, stack);
    rib = NULL;
    next = n->cdr->cdr->car;
    return virtmach();
  }
  else if (strcmp(n->car, "argument") == 0) {
    rib = cons(accum, rib);
    next = n->cdr->car;
    return virtmach();
  }
  else if (strcmp(n->car, "apply") == 0) {
    Text* a = accum;
    void* body = a->car;
    void* clos = a->cdr->car;
    void* vars = a->cdr->cdr->car;
    env = extend(env, vars, rib);
    rib = NULL;
    next = body;
    return virtmach();
  }
  else if (strcmp(n->car, "return") == 0) {
    Text* s = stack;
    next = s->car;
    env = s->cdr->car;
    rib = s->cdr->cdr->car;
    stack = s->cdr->cdr->cdr->car;
    return virtmach();
  }
  else {
    fprintf(stderr, "Unhandled operation.\n");
    assert(0);
  }
}


int main(int argc, char** argv) {
  // note! repl implies there's a top-level but there isn't...
  printf("Lisp REPL\n\n");
  printf(">> ");

  char buffer[256];
  while (fgets(buffer, 256, stdin)) {
    next = compile(read(buffer), cons("halt", NULL));
    virtmach();
    print(accum);
    printf(">> ");
  }
  return 0;
}


