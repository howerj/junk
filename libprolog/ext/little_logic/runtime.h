//
//  runtime.h
//  little_logic
//
//  Created by Phillip Voyle on 1/11/15.
//  Copyright © 2015 PV. All rights reserved.
//

#ifndef runtime_h
#define runtime_h

#include "variant.h"
#include "compiled_clause.h"
#include "parameter.h"
#include <functional>

class runtime
{
   std::vector<compiled_clause_collection> clause_db;
   std::vector<std::shared_ptr<variable>> stack;
   
   std::vector<std::string> predicate_names;
   std::map<std::string, int> predicate_identifiers;
public:

   const std::vector<std::shared_ptr<variable>>& get_stack() const
   {
      return stack;
   }
   
   runtime():stack {nullptr}
   {
   }

   variable_handle obtain_root_index(variable_handle position)
   {
      return position.get_root(stack);
   }

   bool unify_variants(int base, variable_handle v1, variable_handle v2, const std::function<bool()>& thenwhat)
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
         auto bound_variable = v2_root.get_bound_variable(stack);
         v1_root.set_value(stack, bound_variable);
         result = thenwhat();
         v1_root.unlink(stack);
      }
      else if(v2_value == nullptr)
      {
         auto bound_variable = v1_root.get_bound_variable(stack);
         v2_root.set_value(stack, bound_variable);
         result = thenwhat();
         v2_root.unlink(stack);
      }
      else
      {
         if(variant::compare(((value_variable*)v1_value)->value.get(), ((value_variable*)v2_value)->value.get()))
         {
            result = thenwhat();
         }
      }
      return result;
   }


   bool unify_parameter(int base, variable_handle argument, parameter parameter, const std::function<bool()>& thenwhat)
   {
      variable_handle new_argument = parameter.instantiate_argument(base, stack);
      bool result = unify_variants(base, argument, new_argument, thenwhat);
      stack.pop_back();
      return result;
   }


   bool unify_parameters(int base, int parameter_index, const std::vector<parameter>& parameters, const std::vector<variable_handle>& arguments, const std::function<bool()>& thenwhat)
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
         return unify_parameter(base, arguments[parameter_index], parameters[parameter_index], [&](){
            return unify_parameters(base, parameter_index + 1, parameters, arguments, thenwhat);
         });
      }
   }

   std::vector<variable_handle> instantiate_predicate_arguments(int base, const std::vector<parameter>& parameters)
   {
      std::vector<variable_handle> arguments;
      for(auto p : parameters)
      {
         arguments.push_back(p.instantiate_argument(base, stack));
      }
      return arguments;
   }
   
   std::vector<variable_handle> instantiate_predicate_arguments(const compiled_predicate& predicate)
   {
      return instantiate_predicate_arguments(stack.size(), predicate.parameters);
   }
   
   void unstack_arguments(const std::vector<variable_handle>& arguments)
   {
      for(int n = 0; n < arguments.size(); n++)
      {
         stack.pop_back();
      }
   }

   bool unify_conjunction(int base, int index, const conjunction& conj,const std::function<bool()>& thenwhat)
   {
      if(index == conj.size())
      {
         return thenwhat();
      }
      else
      {
         std::vector<variable_handle> arguments = instantiate_predicate_arguments(base, conj[index].parameters);
         bool result = unify_clause(conj[index].predicate_index, arguments, [&]{
            return unify_conjunction(base, index + 1, conj, thenwhat);
         });
         
         unstack_arguments(arguments);
         return result;
      }
   }

   bool unify_clause(int predicate, const std::vector<variable_handle>& arguments, const std::function<bool()>& thenwhat)
   {
      int base = stack.size();
      for(const auto& clause: clause_db[predicate])
      {
         if(unify_parameters(base, 0, clause.goal_parameters, arguments, [&](){
               return unify_conjunction(base, 0, clause.requirements, thenwhat);
            }))
         {
            return true;
         }
      }
      return false;
   }
   
   
   compiled_predicate compile_predicate(const text_predicate& pred, std::map<std::string, int>& first_occurrence, int &position)
   {
      int predicate_id = 0;
      std::vector<parameter> result;
      auto it = predicate_identifiers.find(pred.predicate_name);
      if(it == predicate_identifiers.end())
      {
         predicate_id = predicate_names.size();
         predicate_names.push_back(pred.predicate_name);
         predicate_identifiers[pred.predicate_name] = predicate_id;
         clause_db.push_back({});
      }
      else
      {
         predicate_id = it->second;
      }
      
      for(auto parm : pred.parameters)
      {
         position ++;
         char c = parm[0];
         if(c >= 'A' && c <= 'Z') //variable
         {
            auto first = first_occurrence.find(parm);
            int parm_id = 0;
            if(first == first_occurrence.end())
            {
               first_occurrence[parm] = position;
            }
            else
            {
               parm_id = -first->second;
            }
            result.push_back(parm_id);
         }
         else
         {
            result.push_back(stack.size());
            stack.push_back(std::make_shared<value_variable>(std::make_shared<string_variant>(parm)));
         }
      }
      return compiled_predicate(predicate_id, result);
   }

   compiled_predicate compile_predicate(const text_predicate& pred)
   {
      int position = 0;
      std::map<std::string, int> first_occurrence;
      return compile_predicate(pred, first_occurrence, position);
   }

   compiled_clause compile_clause(const text_clause& clause)
   {
      std::map<std::string, int> first_occurrence;

      compiled_clause result;
      int position = 0;
      if(clause.goal.predicate_name != "")
      {
         result.goal = compile_predicate(clause.goal, first_occurrence, position);
      }
      
      for(auto pred : clause.requirements)
      {
         result.requirements.push_back(compile_predicate(pred, first_occurrence, position));
      }
      
      return result;
   }

   bool prove(const conjunction& conj)
   {
      return unify_conjunction(stack.size(), 0, conj, []{
         return true;
      });
   }
   
   bool prove(const compiled_predicate& pred, std::vector<variable_handle>& arguments, const std::function<bool()>& thenwhat)
   {
      return unify_clause(pred.predicate_index, arguments, [&]{
         return thenwhat();
      });
   }

   bool prove(const compiled_predicate& pred)
   {
      int base = stack.size();
      std::vector<variable_handle> arguments = instantiate_predicate_arguments(base, pred.parameters);
      
      bool result =  prove(pred, arguments,[](){
         return true;
      });
      
      unstack_arguments(arguments);
      return result;
   }
   
   void print_predicate(int predicate, const std::vector<parameter>& parameters, int& position)
   {
      std::cout << predicate_names[predicate] << "(";
      bool bfirst = true;
      for(parameter parm : parameters)
      {
         position ++;
         if(bfirst)
            bfirst = false;
         else
            std::cout << ",";

         parm.print_parameter(stack, position);
      }
      std::cout << ")";
   }
   
   void dump_clause(int predicate, const compiled_clause_body& body)
   {
      int position = 0;
      print_predicate(predicate, body.goal_parameters, position);
      
      if(body.requirements.size() > 0)
      {
         bool first_conjuction = true;
         std::cout << " :- ";
         for(compiled_predicate pred: body.requirements)
         {
            if(first_conjuction)
               first_conjuction = false;
            else
               std::cout << ", ";
            
            print_predicate(pred.predicate_index, pred.parameters, position);
         }
      }
      
      std::cout << "." << std::endl;
   }
   
   void dump_program()
   {
      for(int a = 0; a < predicate_names.size(); a++)
      {
         for(compiled_clause_body c: clause_db[a])
         {
            dump_clause(a, c);
         }
      }
   }
   
   void install_clause(const compiled_clause& compiled)
   {
      compiled_clause_body body;
      body.goal_parameters = compiled.goal.parameters;
      body.requirements = compiled.requirements;
      clause_db[compiled.goal.predicate_index].push_back(body);
   }
};

#endif /* runtime_h */
