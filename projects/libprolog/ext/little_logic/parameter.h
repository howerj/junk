//
//  parameter.h
//  little_logic
//
//  Created by Phillip Voyle on 1/11/15.
//  Copyright © 2015 PV. All rights reserved.
//

#ifndef parameter_h
#define parameter_h

class parameter
{
   int parm;
public:
   parameter(int p)
   {
      parm = p;
   }
   
   variable_handle instantiate_argument(int base, std::vector<std::shared_ptr<variable>>& stack)
   {
      if(parm == 0) //unbound variable
      {
         stack.push_back(nullptr);
      }
      else if(parm > 0) //refer to existing value
      {
         stack.push_back(std::make_shared<bound_variable>(parm));
      }
      else //refer to a previous parameter
      {
         int refer_parameter = base - parm - 1;
         stack.push_back(std::make_shared<bound_variable>(refer_parameter));
      }
      
      return variable_handle(stack.size() - 1);
   }
   
   void print_parameter(const std::vector<std::shared_ptr<variable>>& stack, int position)
   {
      if(parm > 0)
         variable_handle(parm).print(stack);
      else if(parm == 0)
         std::cout << "P" << position;
      else
         std::cout << "P" << -parm;
   }
};


#endif /* parameter_h */
