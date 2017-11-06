This code was taken from a blog post describing how to make a prolog
interpreter.

From
<https://dabblingseriously.wordpress.com/2015/11/15/a-toy-prolog-interpreter-in-c/>

# A toy prolog interpreter in c++
## Introduction

One of my old mentors told me that he thought you were never a real computer
programmer until you’ve written a compiler. I was still at high school at that
time and a few years later I had the opportunity to implement a compiler as
part of a university paper. That idea has stuck with me and a few years back I
got interested in the inner workings of prolog. I had used prolog briefly at
university but hadn’t ever attempted to understand how that was implemented and
set about implementing an interpreter rather than a compiler, just to
understand the details and problems that come about when you try to execute
prolog.

In this article I’m going to give you a brief tour of some of the parts of an
implementation of a logic and inference algorithm. The kinds of logic used in
this program can be used as a basis of a logic programming language, or a type
inference system. Obviously a real prolog implementation would have many more
features.
Download Link <https://www.dropbox.com/s/7k5zrflhskw3yu2/little_logic.zip?dl=0>

The program is split into two main parts, the parser, and the interpreter.
First you parse the program, and then process the clauses into compressed
clauses that use integers for identifiers rather than strings, and then you can
execute the clauses. Here’s a sample main:

	int main(int argc, char* argv[])
	{
		std::string program = ""
		"parent(great_grandad, nana).\n"
		"parent(great_nana, nana).\n"
		"parent(great_pop, grandad).\n"
		"parent(great_grandma, grandad).\n"
		"parent(grandad, dad).\n"
		"parent(nana, dad).\n"
		"parent(grandad, uncle).\n"
		"parent(nana, uncle).\n"
		"parent(dad, miss).\n"
		"parent(dad, master).\n"
		"ancestor(X, Y) :- parent(X, Y).\n"
		"ancestor(X, Y) :- parent(X, Z), ancestor(Z, Y).\n";

		runtime rt;
		std::stringstream ssprogram(program);
		parser p;
		execute_program(p.parse_program(ssprogram), rt);
		rt.dump_program();

		run_predicate("ancestor(dad,B)", rt);
		enumerate_predicate("ancestor(Ancestor, master)", rt);
		return 0;
	}

This code creates a runtime and parses the program and then executes it in the
runtime. This will install the clauses into the runtime.

The very next thing the sample does, is dumps the program out to the console,
this helps me see what is actually loaded.

the final two things that the program does are running a predicate to get one
answer using run\_predicate, and then running another predicate to list all of
the answers using enumerate\_predicate.
How it works

Prolog clauses are made up of a left side, and a right side. The left side
might be considered a goal that you might be wanting to prove, and the right
side would be one possible list of things that if true would mean that the left
side is true. Clauses can have variables, beginning with a capital that can
match any value.

	#if You're happy and you know it clap your hands
	clap_your_hands(You) :- happy(You), know_it(You). 

One of the first problems you encounter when executing prolog, is how when you
attempt to prove a clause and find a solution, you may need to attempt several
possible candidate clauses to check which of those might lead to an answer. In
the example above, there might be several reasons to be happy, but you might
not know it.
Unification of Values

Solving a clause with two predicates on the right side might mean delving deep
into one predicate before you can know the parameters that can be passed into
the second predicate, especially in the case that the clause specifies
variables rather than values. Basically what we need to do is specify some
things that we want to do for every possible match, and we can do that with a
lambda.

Prolog’s unification mechanism, means that you can specify a variable at one
point in a clause, which will bind to the first value you specify, and which
later when you use that variable again the same value will be used.

The following process will attempt to unify either two values, or two
variables. If either variable or both are not bound, then the unification
passes, otherwise the values must be identical. Note the function thenwhat,
which is used to continue execution once values are unified. Once execution is
finished, the unifications are rolled back so that another one can be
attempted.

The point I’m trying to stress to you here, is that it’s more difficult than
returning yes/no answer to a question, you might also have other things you
need to take care of before you know the actual answer.

	bool unify_variants(
	int base,
	variable_handle v1,
	variable_handle v2,
	const std::function<bool()>& thenwhat)
	{
		variable_handle v1_root = obtain_root_index(v1);
		variable_handle v2_root = obtain_root_index(v2);

		if(v1_root == v2_root)
		{
			return thenwhat(); //already are
		}

		variable* v1_value = v1_root.get_value(stack);
		variable* v2_value = v2_root.get_value(stack);

		bool result = false;
		if(v1_value == nullptr)
		{
			auto bound_variable = 
			v2_root.get_bound_variable(stack);
			v1_root.set_value(stack, bound_variable);
			result = thenwhat();
			v1_root.unlink(stack);
		}
		else if(v2_value == nullptr)
		{
			auto bound_variable = 
			v1_root.get_bound_variable(stack);
			v2_root.set_value(stack, bound_variable);
			result = thenwhat();
			v2_root.unlink(stack);
		}
		else
		{
			if(variant::compare(
			((value_variable*)v1_value)->value.get(),
			((value_variable*)v2_value)->value.get()))
			{
				result = thenwhat();
			}
		}
		return result;
	}

## Unification of Parameters

In order to solve a clause you first need to be able to pass arguments into the
formal parameters of the clause. Some of the arguments might be constrained,
and some might not.

The following functions take arguments and attempt to unify those arguments
with the parameters of a clause. Performing the unifications recursively allows
the interpreter to keep track of the variables and unifications already
applied.

	bool unify_parameter(
	int base,
	variable_handle argument,
	parameter parameter,
	const std::function& thenwhat)
	{
		variable_handle new_argument =
		parameter.instantiate_argument(base, stack);
		bool result = unify_variants(
		base, argument, new_argument, thenwhat);
		stack.pop_back();
		return result;
	}

	bool unify_parameters(
	int base, int parameter_index,
	const std::vector<parameter>& parameters,
	const std::vector<variable_handle>& arguments,
	const std::function<bool()>& thenwhat)
	{
		if(parameter_index == parameters.size())
		{
			if(parameter_index == arguments.size())
			{
				return thenwhat();
			}
			else
			{
				return false; //argument list too long
			}
		}
		else if(parameter_index == arguments.size())
		{
			return false; //argument list too short
		}
		else
		{
			return unify_parameter(
			base,
			arguments[parameter_index],
			parameters[parameter_index],
			[&](){
			return unify_parameters(
			base,
			parameter_index + 1,
			parameters,
			arguments,
			thenwhat);
			});
		}
	}

## Proving Conjunctions

In my implementation, parameters specify where in the clause or in the value
database a variable or value should come from. The following code pushes values
onto the stack for each formal parameter and then unifies the arguments with
the parameters as above.

The unify\_conjunction method will instantiate all arguments, and then attempt
to prove each clause in the conjunction.

	std::vector<variable_handle>
	instantiate_predicate_arguments(
	int base,
	const std::vector<parameter>& parameters)
	{
	std::vector<variable_handle> arguments;
	for(auto p : parameters)
	{
	arguments.push_back(
	p.instantiate_argument(base, stack));
	}
	return arguments;
	}

	std::vector<variable_handle>
	instantiate_predicate_arguments(
	const compiled_predicate& predicate)
	{
		return instantiate_predicate_arguments(
		stack.size(), predicate.parameters);
	}

	void unstack_arguments(
	const std::vector<variable_handle>& arguments)
	{
		for(int n = 0; n < arguments.size(); n++)
		{
			stack.pop_back();
		}
	}

	bool unify_conjunction(
	int base,
	int index,
	const conjunction& conj,
	const std::function<bool()>& thenwhat)
	{
		if(index == conj.size())
		{
			return thenwhat();
		}
		else
		{
			std::vector<variable_handle> arguments =
			instantiate_predicate_arguments(
			base, conj[index].parameters);

			bool result = unify_clause(
			conj[index].predicate_index,
			arguments,
			[&]{
			return unify_conjunction(
			base,
			index + 1,
			conj, thenwhat);
			});

			unstack_arguments(arguments);
			return result;
		}
	}

## Proving Clauses

The last piece of the interpretation puzzle is unification of clauses. In
order to prove a clause, you need to first check that the arguments passed to
the goal match, and then you need to prove the conjunction. The following code
goes through each clause in the clause dictionary and attempts to match the
arguments with the parameters of the clause. If they match, then the next thing
it does is attempt to prove the conjunction.

	bool unify_clause(
	int predicate,
	const std::vector<variable_handle>& arguments,
	const std::function<bool()>& thenwhat)
	{
		int base = stack.size();
		for(const auto& clause: clause_db[predicate])
		{
			if(unify_parameters(
			base,
			0,
			clause.goal_parameters,
			arguments,
			[&](){
			return unify_conjunction(
			base,
			0,
			clause.requirements,
			thenwhat);
			}))
			{
				return true;
			}
		}
		return false;
	}

## Summary

In this article I’ve given you a tour of some of the complicated parts of a toy
implementation of the inference part of prolog. Hopefully you found something
of use in here, and if you have any comments or questions, let me know either
by commenting here or by sending me an email at
phillipvoyle@hotmail.com

Thanks for reading.
