//
//  parser.h
//  little_logic
//
//  Created by Phillip Voyle on 1/11/15.
//  Copyright © 2015 PV. All rights reserved.
//

#ifndef parser_h
#define parser_h

#include "text_clause.h"
#include <stdio.h>

class parser
{
public:
   void skip_ws(std::istream& is)
   {
      for(;;)
      {
         char c = is.peek();
         if((c == '\n') || (c == '\r') || (c == ' ') || (c == '\t'))
         {
            is.get();
         }
         else
         {
            break;
         }
      }
   }

   std::string parse_atom(std::istream& is)
   {
      skip_ws(is);
      std::string result;
      for(;;)
      {
         char c = is.peek();
         if(isalnum(c) || (c == '_'))
         {
            is.get();
            result.push_back(c);
         }
         else
         {
            return result;
         }
      }
   }

   text_predicate parse_predicate(std::istream& is)
   {
      text_predicate result;
      result.predicate_name = parse_atom(is);
      if(result.predicate_name.length() > 0)
      {
         skip_ws(is);
         if(is.peek() == '(')
         {
            is.get();
         }
         
         for(;;)
         {
            skip_ws(is);
            if(is.peek() == ')')
            {
               is.get();
               return result;
            }
            if((result.parameters.size() > 0) && (is.get() != ','))
            {
               throw std::runtime_error("expected ','");
            }
            skip_ws(is);
            
            char c = is.peek();
            if(isalnum(c) || (c == '_'))
            {
               result.parameters.push_back(parse_atom(is));
            }
            else
            {
               throw std::runtime_error("expected a predicate");
            }
         }
      }
      throw std::runtime_error("not a predicate");
   }

   text_clause parse_clause(std::istream& is)
   {
      skip_ws(is);
      
      text_clause result;
      if(is.peek() == ':')
      {
         //uh-oh..
         is.get();
         if(is.get() != '-')
         {
            throw std::runtime_error("expected ':-'");
         }
      }
      else
      {
         result.goal = parse_predicate(is);
         skip_ws(is);
         char c = is.get();
         if(c == '.')
         {
            return result;
         }
         
         if((c != ':') || (is.get() != '-'))
         {
            throw std::runtime_error("expected ':-'");
         }
      }
      
      for(;;)
      {
         skip_ws(is);
         if(is.peek() == '.')
         {
            is.get();
            return result;
         }
         skip_ws(is);
         if(result.requirements.size() > 0)
         {
            char c = is.get();
            if(c != ',')
               throw std::runtime_error("expected ','");
         }
         skip_ws(is);
         result.requirements.push_back(parse_predicate(is));
      }
   }

   text_predicate parse_predicate(const std::string& sPredicate)
   {
      std::stringstream ssPredicate(sPredicate);
      return parse_predicate(ssPredicate);
   }

   text_clause parse_clause(const std::string& sClause)
   {
      std::stringstream ssClause(sClause);
      return parse_clause(ssClause);
   }

   text_program parse_program(std::istream& is)
   {
      text_program program;
      for(;;)
      {
         skip_ws(is);
         if(is.peek() == EOF)
         {
            return program;
         }
         
         program.push_back(parse_clause(is));
      }
   }
};


#endif /* parser_h */
