#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_VARIABLES 10
#define ID_LEN 64
#define STR_LEN 128
#define MAX_RULES 128
#define MAX_TERMS 256
#define PRINTLIST 2
#define TRUE 1
#define FALSE 0
#define FUNCTOR(s) 							\
	if(!isFunctor(s))						\
		yyerror("syntax error");
#define START_QUERY() 						\
	isQuery = 1;							\
	queryLen = 0;							\
	curQuery = NULL;						\
	queryVars = 0;							\
	containsVariable = 0;					\
	curEnv = NULL;

#define END_QUERY() 						\
	insertRule();							\
	isQuery = 0; 							\
	append(curList->head, curList->tail); 	\
	curQuery = curList->head;				\
	curEnv = NULL;							\
	containsVariable = containsVar(curQuery);\
	resolveQuery(); 						\
	removeQuery();							\
	unbindAllVariables();					

#define START_STAT()						\
	curList = malloc(sizeof(Rule));			\
	curList->head = NULL;					\
	curList->tail = NULL;					\
	isStructure = 0;


#define END_STAT() 	 						\
	prompt();	

#define ADD_RULE() 							\
	progRules[numProgRules++] = curList;	\
	insertRule();							
#define END_STRUCTURE() 					\
	isStructure = 0;				

#define START_STRUCTURE()					\
	isStructure = 1;						\
	curStructure = 0;				


enum Type {
	Constant,Variable,Structure
};

//structs
typedef struct Term {
	char * id;								//ID of the term
	enum Type type;							//Type of the term
	struct Term *   	parent;				//Parent term in list
	struct Term * 		next;				//Next term in list
	union {
		struct Var * 	variable;  			//contains a variable if term is a variable
		struct Clause * structure;			//contains structure if term is a structure
	};
} Term;

typedef struct Rule {
	struct Term * head;
	struct Term * tail;
} Rule;

typedef struct Clause {
	struct Term * paramList;
} Clause;

typedef struct Var {
	struct Term * bound;
} Var;

typedef struct Environment {
	int goal;								//was goal matched
	Term * binds;							//variable bindings
} Environment;

//data structures
Rule * curList;					//current List buffer
Rule * progRules[MAX_RULES];	//all of the program rules
Term * Terms[MAX_TERMS];		//all terms (non structures) in program
Term * Binds[MAX_TERMS];		//current variable bindings

//indices
int numProgRules;				//number of program rules
int numTerms;					//number of non structures in program
int numBound;					//number of Variables in program
int queryLen;					//number of terms in query
int queryVars;

//buffers
Term * curQuery;				//current query buffer
Term * curStructure;			//current structure buffer
Term * curEnv;

//flags
int inFile; 				//flag : 1 if inputfile, 0 if stdin
int isQuery;				//flag: 1 if query, 0 if rule
int isStructure;			//term is a structure, parse and geneate terms accordingly
int containsVariable;		//query contains variable, handle unification and printing

//Interpreter functions
void initGlobalIndices();						//initalize global indices
Term * initTerm(char * id, enum Type type);		//initialize term
void append(Term * a, Term * b);				//append function shown in handout
void prompt();									//prompts the user and calls yyparse()
int isFunctor(char * id); 						//returns TRUE if functor, FALSE otherwise
int match(Term * a, Term * b); 					//match function show in handout
int matchStructures(Term * a, Term * b); 		//match helper function
int matchVariables(Term * a, Term * b);			//match helper function
Var * initVar();								//initalize variable
Clause * initStructure(char * s);				//initialize structure
void assignConstant(char * s);					//assign constant to the rule
void assignStructure(char * s);					//assign structure to the rule
void assignVariable(char * s);					//assign variable to the rule
Term * lastNode(Term * node);					//return last node in a list
int solve(Term * query, Term * env);			//solve function in handout
void printResults(int val);						//prints results
void resolveQuery();							//calls solve and prints out appropriate message
int containsVar(Term * query);					//checks to see if query contains variable
Term * copy(Term * src);						//returns copy of list
Var * copyVar(Var * v);							//returns copy of variable
Clause * copyStructure(Clause * c);				//returns copy of structure
Term * getBinding(Term * t);					//get variable binding
void bindVariable(Term * v, Term * t);			//binds variable to Term
void unbindVariable(Term * t);					//unbinds all variables
void unbindAllVariables();						//unbinds a single variable
void freeList(Term * t);						//free list of terms


//Not really needed (using data structure that was going to be used to implement Query Variables)
void insertVariable(Term * t);					//populates Terms data structure (not being used)	
void insertConstant(Term * t);					//populates Terms data structure (not being used)	
void inserTerm(Term * t);						//populates Terms data structure (not being used)	
void insertRule();								//modifies Terms datastructure (not being used)	
void removeQuery();								//removes query from Terms data structure (not being used)	


//debugging functions
void printTerm(Term * Node);
void printStructure(Term * Node);
void printVariable(Term * Node);
void printConstant(Term * Node);
void printTail(Term * Node);
void printTermArrays();
void printVariableBindings();