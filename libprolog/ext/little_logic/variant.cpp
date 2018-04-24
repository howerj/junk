//
//  variant.cpp
//  little_logic
//
//  Created by Phillip Voyle on 1/11/15.
//  Copyright © 2015 PV. All rights reserved.
//

#include "variant.h"

bool variant::compare(variant *v1, variant* v2)
{
   if(v1->type == v2->type)
   {
      switch (v1->type)
      {
         case string_value:
            if(((string_variant*)v1)->value == ((string_variant*)v2)->value)
            {
               return true;
            }
         default:
            break;
      }
   }
   return false;
}