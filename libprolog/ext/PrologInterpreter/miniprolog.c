#include "miniprolog.h"


extern int yylineno;

void prompt() {
	if(isatty(0)) {
		if(!inFile){ //only print prompt with standard input
			printf("%d>", yylineno);
			fflush(stdout);
		}
	}
	yyparse();
}

int isFunctor(char * id) {
	while(*id++) {
		if(isdigit(*id)) return 0;
	}
	return 1;
}

Var * initVar() {
	Var * v = malloc(sizeof(Var));
	v->bound = NULL;
	return v;
}

Clause * initStructure(char * string) {
	Clause * s = malloc(sizeof(Clause));
	s->paramList = malloc(sizeof(Term));
	s->paramList->id = strdup("*STRUCTURE*");
	return s;
}

Term * initTerm(char * id, enum Type type) {
	Term * term = malloc(sizeof(Term));
	term->id = id;
	term->type = type;
	term->parent = NULL;
	term->next = NULL;
	term->variable = NULL;
	term->structure = NULL;
	if(type == Variable) term->variable = initVar();
	if(type == Structure) term->structure = initStructure(id);
	return term;
}


void append(Term * head, Term * element) {
	if(head == NULL) return;
	Term * start = head;
	while(head->next != NULL) {
		head = head->next;
	}
	head->next = element;
	if(element != NULL)
		element->parent = head;
}

void assignConstant(char * string) {
 	if(isStructure){
		append(curStructure, initTerm(string, Constant));
		Term * finalNode = lastNode(curStructure);
	}
	else {
	 	if(curList->head == NULL)
	            curList->head = initTerm(string, Constant);
		else{
		    if(curList->tail != NULL)
		        append(curList->tail, initTerm(string, Constant));
		    else 
		        curList->tail = initTerm(string, Constant);
		}
	}
}

void assignStructure(char * string) {
	if(curList->head == NULL){
            curList->head = initTerm(string, Structure);
            curList->head->structure = initStructure(string); 
			curStructure = curList->head->structure->paramList;
			return;
	}
	else{
	    if(curList->tail != NULL){
	        append(curList->tail, initTerm(string, Structure));
	        Term * finalNode = lastNode(curList->tail);
	        finalNode->structure = initStructure(string); 
	        curStructure = finalNode->structure->paramList;
	    }
	    else{ 
	        curList->tail = initTerm(string, Structure);
	        curList->tail->structure = initStructure(string);
	        curStructure = curList->tail->structure->paramList;
		}
	}
}

void assignVariable(char * string) {
	if(isStructure){
		append(curStructure, initTerm(string, Variable));
		Term * finalNode = lastNode(curStructure);
		finalNode->variable = initVar();
	}
	else {
		if(curList->head == NULL){
	            curList->head = initTerm(string, Variable);
				curList->head->variable = initVar();
		}
		else{
		    if(curList->tail != NULL){
		        append(curList->tail, initTerm(string, Variable));
		        Term * finalNode = lastNode(curList->tail);
	        	finalNode->variable = initVar(); 
		    }
		    else {
		        curList->tail = initTerm(string, Variable);
		        curList->tail->variable = initVar();
		    }
		}
	}
}

Var * copyVar(Var * v){
	Var * newVar = initVar();
	newVar->bound = v->bound;
	return newVar;
}

Clause * copyStructure(Clause * c) {
	Clause * ns = initStructure("eyelid");
	ns->paramList = copy(c->paramList);
	return ns; 
}

Term * copy(Term * tail) {
	if(!tail) return NULL;
	Term * copy = initTerm(tail->id, tail->type);
	if(tail->type == Variable)
		copy->variable = copyVar(tail->variable);
	if(tail->type == Structure)
		copy->structure = copyStructure(tail->structure);
	tail = tail->next;
	while(tail) {
		Term * temp = initTerm(tail->id, tail->type);
		if(tail->type == Variable)
			temp->variable = copyVar(tail->variable);
		if(tail->type == Structure)
			temp->structure = copyStructure(tail->structure);
		append(copy, temp);
		tail = tail->next;
	}
	append(copy,NULL);
	return copy;
}


Term * lastNode(Term * node) {
	while(node->next) 
		node = node->next;
	return node;
}


void resolveQuery() {
	if(!containsVariable)
		printResults(solve(curQuery, curEnv));
	else
		printf("no\n");//figure out what to do
}

int solve(Term * query, Term * env) {
	int i = 0;
	int result = FALSE;
	if(query){
		for(; i < numProgRules; i++) {
			if(match(progRules[i]->head, query)) {
				Term * t = copy(progRules[i]->tail);	
				if(!t)
					result = solve(query->next,env);
				else {
					append(t,query->next);
					result = solve(t, env);	
				}
			}
		}
	}
	else {
		return TRUE;
	}
	return result;
}

int match(Term * a, Term * b) {
	if(a->type == Constant && b->type == Constant)
		return (!strcmp(a->id, b->id));

	else if (a->type == Structure && b->type == Structure)
		return matchStructures(a,b);

	else if (a->type == Variable || b-> type == Variable)
		return matchVariables(a,b);

	return FALSE;	
}

int matchStructures(Term * a, Term * b) {
	Term * arity1, * arity2;
	if(strcmp(a->id,b->id))
		return FALSE;

	arity1 = a->structure->paramList->next;
	arity2 = b->structure->paramList->next;
	while(arity1 && arity2) {

		if(!match(arity1, arity2))
			return FALSE;
		arity1 = arity1->next;
		arity2 = arity2->next;
	}
	if(!arity1 && !arity2) 
		return TRUE;
	return FALSE;
}

int matchVariables(Term  * rule, Term * query) {
	if(!containsVariable){
		if(rule->type != Variable){
			Term * binding = getBinding(query);
			if(!binding){
				bindVariable(query, rule);
				return TRUE;
			}
			else if(match(rule,binding))
				return TRUE;
		}
		else {
			
			Term * binding = getBinding(rule);
			if(!binding){
				bindVariable(rule, query);
				return TRUE;
			}
			else {
				Term * queryBind = getBinding(query);
				if(!queryBind) 
					queryBind = query;
				if(match(binding,queryBind))
					return TRUE;
				else {
					unbindAllVariables();
					return FALSE;
				}
			}
		}
	}
	else{
		

	}
	return FALSE;
}

int containsVar(Term * a) {
	int result = FALSE;
	while(a) {
		if(a->type == Variable)
			return TRUE;
		if(a->type == Structure)
			result = containsVar(a->structure->paramList);
		a = a->next;
	}
	return result;
}

void initGlobalIndices() {
	numProgRules = 0;
	numTerms = 0;
	numBound = 0;
}

void insertVariable(Term * t) {
	Terms[numTerms++] = t;
	Binds[numBound++] = t;
	if(isQuery) queryVars++;
}

void insertConstant(Term * t) {
	Terms[numTerms++] = t;
}

void insertTerm(Term * t) {
	if(t->type == Constant)
		insertConstant(t);
	else if(t->type == Variable)
		insertVariable(t);
	else if(t->type == Structure) {
		Term * pl= t->structure->paramList;
		pl = pl->next;
		while(pl) {
			insertTerm(pl);
			pl = pl->next;
		}
	}
}


void insertRule() {
	if(isQuery) queryLen = 1;
	Term * head = curList->head;
	insertTerm(head);
	Term * temp = curList->tail;
	while(temp) {
		insertTerm(temp);
		temp = temp->next;
		if(isQuery) queryLen++;
	}
}

void removeQuery() {
	numTerms -= queryLen;
	numTerms -= queryVars;
	numBound -= queryVars;
}

Term * getBinding(Term * v) {
	int i = 0;
	for(; i < numBound; i++) {
		if(!strcmp(v->id, Binds[i]->id))
			return Binds[i]->variable->bound;
	}
	return NULL;
}

void bindVariable(Term * v, Term * t){
	int i;
	for(i = 0; i < numBound; i++){
		if(!strcmp(v->id, Binds[i]->id)){
			Binds[i]->variable->bound  = t;
			return;
		}
	}
}

void unbindVariable(Term * v) {
	int i;
	for(i = 0; i < numBound; i++) {
		if(!strcmp(v->id, Binds[i]->id)) {
			Binds[i]->variable->bound = NULL;
			return;
		}
	}
}

void unbindAllVariables() {
	int i;
	for(i = 0; i < numBound; i++) 
		Binds[i]->variable->bound = NULL;
}

int main() {
	initGlobalIndices();
	prompt();
}

