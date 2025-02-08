/**
 * @file dominance.cpp
 * @brief Domination checking routines
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 
 #include "global.h"
 #include "rand.h"
 
 /**
  * @brief Routine for usual non-domination checking.
  *
  * This function performs non-domination checking between two individuals.
  * 
  * @param a Pointer to the first individual.
  * @param b Pointer to the second individual.
  * @return int 
  *         - 1 if a dominates b,
  *         - -1 if b dominates a,
  *         - 0 if both a and b are non-dominated.
  */
 int check_dominance(individual *a, individual *b)
 {
     int i;
     int flag1;
     int flag2;
     flag1 = 0;
     flag2 = 0;
     if (a->constr_violation < 0 && b->constr_violation < 0)
     {
         if (a->constr_violation > b->constr_violation)
         {
             return 1;
         }
         else if (a->constr_violation < b->constr_violation)
         {
             return -1;
         }
         else
         {
             return 0;
         }
     }
     else
     {
         if (a->constr_violation < 0 && b->constr_violation >= 0)
         {
             return -1;
         }
         else
         {
             if (a->constr_violation >= 0 && b->constr_violation < 0)
             {
                 return 1;
             }
             else
             {
                 for (i = 0; i < nobj; i++)
                 {
                     if (a->obj[i] < b->obj[i])
                     {
                         flag1 += 1;
                     }
                     else
                     {
                         if (a->obj[i] > b->obj[i])
                         {
                             flag2 += 1;
                         }
                     }
                 }
                 if (flag1 > flag2 )
                 {
                     return (1);
                 }
                 else
                 {
                     if (flag1 < flag2)
                     {
                         return -1;
                     }
                   else if (flag1 == 0 && flag2 == 0) // are the same point, we select the first
                   {
                     return 1;
                   }
                   else
                     {
                       return 0;
                     }
                 }
             }
         }
     }
 }
 