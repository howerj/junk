//
//  variable_handle.h
//  little_logic
//
//  Created by Phillip Voyle on 1/11/15.
//  Copyright © 2015 PV. All rights reserved.
//

#ifndef variable_handle_h
#define variable_handle_h

#include "variant.h"

class variable_handle
{
   int stack_index;
public:
   variable_handle(int index)
   {
      stack_index = index;
   }
   
   bool operator==(const variable_handle& v)
   {
      return stack_index == v.stack_index;
   }
   
   variable_handle get_root(const std::vector<std::shared_ptr<variable>>& stack) const
   {
      int index = stack_index;
      for(;;)
      {
         variable* v = stack[index].get();
         if((v == nullptr) || (v->type != bound_value))
            return index;
         index = ((bound_variable*)v)->var;
      }
      return variable_handle(index);
   }
   
   variable* get_value(const std::vector<std::shared_ptr<variable>>& stack)
   {
      int index = stack_index;
      for(;;)
      {
         variable* v = stack[index].get();
         if((v == nullptr) || (v->type != bound_value))
            return v;
         index = ((bound_variable*)v)->var;
      }
   }
   
   void set_value(std::vector<std::shared_ptr<variable>>& stack, std::shared_ptr<variable> value)
   {
      stack[stack_index] = value;
   }
   
   void unlink(std::vector<std::shared_ptr<variable>>& stack)
   {
      stack[stack_index] = nullptr;
   }
   
   std::shared_ptr<variable> get_bound_variable(const std::vector<std::shared_ptr<variable>>& stack)
   {
      return std::make_shared<bound_variable>(get_root(stack).stack_index);
   }
   
   void print(const std::vector<std::shared_ptr<variable>>& stack) const
   {
      variable_handle root = get_root(stack);
      variable* var = root.get_value(stack);
      
      if(var == nullptr)
      {
         std::cout << "V" << root.stack_index;
      }
      else if(var->type == value_value)
      {
         auto val = ((value_variable*) var);
         if(val->type == string_value)
         {
            std::cout << "\"" << ((string_variant*)val->value.get())->value << "\"";
         }
      }
   }
};

#endif /* variable_handle_h */
