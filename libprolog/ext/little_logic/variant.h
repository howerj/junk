//
//  variant.h
//  little_logic
//
//  Created by Phillip Voyle on 1/11/15.
//  Copyright © 2015 PV. All rights reserved.
//

#ifndef variant_h
#define variant_h

#include <string>
#include <memory>

enum variant_type
{
   string_value
};

class variant
{
public:
   const variant_type type;
   virtual ~variant() { }
   variant(variant_type t):type(t)
   {
   }
   
   static bool compare(variant* v1, variant* v2);
};

class string_variant: public variant
{
public:
   std::string value;
   string_variant(const std::string& v): variant(string_value), value(v)
   {
   }
};

enum variable_type
{
   value_value,
   bound_value
};

class variable
{
public:
   const variable_type type;
   virtual ~variable() { }
   variable(variable_type t):type(t)
   {
   }
   
   static bool compare(variant* v1, variant* v2);
};

class value_variable: public variable
{
public:
   std::shared_ptr<variant> value;
   value_variable(std::shared_ptr<variant> v): variable(value_value), value(v)
   {
   }
};

class bound_variable: public variable
{
public:
   int var;
   bound_variable(int v): variable(bound_value), var(v)
   {
   }
};

#endif /* variant_h */
