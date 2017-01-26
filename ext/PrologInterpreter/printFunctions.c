#include "miniprolog.h"

//debugging
void printProgRules() {
	int i = 0;
	printf("=-=-=-=-=-=-=-=-=-=\n");
	for(; i < numProgRules; i++) {
		printf("Head of rule %d\n", i);
		Term * curNode = progRules[i]->head;
		printTerm(curNode);
		Term * tail = progRules[i]->tail;
		if(tail) {
			printf("Tail of rule %d\n", i);
			printTail(tail);
		}
	}
	printf("=-=-=-=-=-=-=-=-=-=\n");
}

void printTail(Term * tail) {
	while(tail) {
		printTerm(tail);
		tail = tail->next;
	}
}

void printTerm(Term * curNode) {
	switch(curNode->type) {
		case(Variable):
			printVariable(curNode);
			break;
		case(Constant):
			printConstant(curNode);
			break;
		case(Structure):
			printStructure(curNode);
			break;
	}
}

void printStructure(Term * a) {
	printf("\tStructure: %s", a->id);
	Clause * s = a->structure;
	Term * paramList = s->paramList;
	printTail(paramList);
	printf("\n");
}

void printVariable(Term * a) {
	printf("\tVariable: %s ", a->id);
	Var * curVar = a->variable;
	if(curVar->bound) {
		printf("bounded to ");
		printTerm(curVar->bound);
	}
	printf("\n");
}

void printConstant(Term * a) {
	printf("\tConstant: %s\n", a->id);
}

void printResults(int val) {
	if(val) printf("yes.\n");
	else 	printf("no.\n");
}

void printTermArrays() {
	int i;
	printf("=-=-=-=-=-=-=-=-=-=-=\n");
	for(i = 0; i < numTerms; i++)
		printf("\t%s\n", Terms[i]->id);
	printf("=-=-=-=-=-=-=-=-=-=-=\n");
	for(i = 0; i < numBound; i++)
		printf("\t%s\n", Binds[i]->id);
	printf("=-=-=-=-=-=-=-=-=-=-=\n");
}

void printVariableBindings() {
	int i;
	for(i = 0; i < numBound; i++) {
		if(Binds[i]->variable->bound){
			printf("Variable: %s Bound to : %s\n", 
				Binds[i]->id, 
				Binds[i]->variable->bound->id);
			printf("\n");
		}
	}
}