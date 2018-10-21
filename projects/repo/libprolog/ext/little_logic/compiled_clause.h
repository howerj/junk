//
//  compiled_clause.h
//  little_logic
//
//  Created by Phillip Voyle on 1/11/15.
//  Copyright © 2015 PV. All rights reserved.
//

#ifndef compiled_clause_h
#define compiled_clause_h

class compiled_predicate
{
public:
   int predicate_index;
   std::vector<parameter> parameters;
   compiled_predicate(int i, const std::vector<parameter>& p)
   {
      predicate_index = i;
      parameters = p;
   }
   compiled_predicate()
   {
      predicate_index = -1;
   }
};

typedef std::vector<compiled_predicate> conjunction;

class compiled_clause_body
{
public:
   std::vector<parameter> goal_parameters;
   conjunction requirements;
};

class compiled_clause
{
public:
   compiled_predicate goal;
   conjunction requirements;
};

typedef std::vector<compiled_clause_body> compiled_clause_collection;

#endif /* compiled_clause_h */
