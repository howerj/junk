//
//  text_clause.h
//  little_logic
//
//  Created by Phillip Voyle on 1/11/15.
//  Copyright © 2015 PV. All rights reserved.
//

#ifndef text_clause_h
#define text_clause_h

class text_predicate
{
public:
   std::string predicate_name;
   std::vector<std::string> parameters;
};

class text_clause
{
public:
   text_predicate goal;
   std::vector<text_predicate> requirements;
};

typedef std::vector<text_clause> text_program;

#endif /* text_clause_h */
