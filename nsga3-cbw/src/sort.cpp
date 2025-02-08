/**
 * @file sort.cpp
 * @brief Routines for randomized recursive quick-sort.
 * 
 * This file contains functions for sorting populations based on
 * different objectives using the quick-sort algorithm.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 
 #include "global.h"
 #include "rand.h"
 
 /**
  * @brief Sorts a population based on a particular objective.
  * 
  * This function sorts the population using the quicksort algorithm
  * based on the specified objective.
  * 
  * @param pop Pointer to the population to be sorted.
  * @param objcount The index of the objective to sort by.
  * @param obj_array Array containing the indices of the individuals to sort.
  * @param obj_array_size The size of the objective array.
  */
 void quicksort_front_obj(population *pop, int objcount, int obj_array[], int obj_array_size)
 {
     q_sort_front_obj(pop, objcount, obj_array, 0, obj_array_size - 1);
     return;
 }
 
 /**
  * @brief Actual implementation of the randomized quick sort.
  * 
  * This function sorts a population based on a particular objective
  * using the quick-sort algorithm.
  * 
  * @param pop Pointer to the population to be sorted.
  * @param objcount The index of the objective to sort by.
  * @param obj_array Array containing the indices of the individuals to sort.
  * @param left The left index for the sorting range.
  * @param right The right index for the sorting range.
  */
 void q_sort_front_obj(population *pop, int objcount, int obj_array[], int left, int right)
 {
     int index;
     int temp;
     int i, j;
     double pivot;
     if (left < right)
     {
         index = rnd(left, right);
         temp = obj_array[right];
         obj_array[right] = obj_array[index];
         obj_array[index] = temp;
         pivot = pop->ind[obj_array[right]].obj[objcount];
         i = left - 1;
         for (j = left; j < right; j++)
         {
             if (pop->ind[obj_array[j]].obj[objcount] <= pivot)
             {
                 i += 1;
                 temp = obj_array[j];
                 obj_array[j] = obj_array[i];
                 obj_array[i] = temp;
             }
         }
         index = i + 1;
         temp = obj_array[index];
         obj_array[index] = obj_array[right];
         obj_array[right] = temp;
         q_sort_front_obj(pop, objcount, obj_array, left, index - 1);
         q_sort_front_obj(pop, objcount, obj_array, index + 1, right);
     }
     return;
 }
 
 /**
  * @brief Sorts a population based on crowding distance.
  * 
  * This function sorts the population using the quicksort algorithm
  * based on crowding distance.
  * 
  * @param pop Pointer to the population to be sorted.
  * @param dist Array containing the crowding distances of the individuals.
  * @param front_size The size of the front to be sorted.
  */
 void quicksort_dist(population *pop, int *dist, int front_size)
 {
     q_sort_dist(pop, dist, 0, front_size - 1);
     return;
 }
 
 /**
  * @brief Actual implementation of the randomized quick sort for crowding distance.
  * 
  * This function sorts a population based on crowding distance using
  * the quick-sort algorithm.
  * 
  * @param pop Pointer to the population to be sorted.
  * @param dist Array containing the crowding distances of the individuals.
  * @param left The left index for the sorting range.
  * @param right The right index for the sorting range.
  */
 void q_sort_dist(population *pop, int *dist, int left, int right)
 {
     int index;
     int temp;
     int i, j;
     double pivot;
     if (left < right)
     {
         index = rnd(left, right);
         temp = dist[right];
         dist[right] = dist[index];
         dist[index] = temp;
         pivot = pop->ind[dist[right]].crowd_dist;
         i = left - 1;
         for (j = left; j < right; j++)
         {
             if (pop->ind[dist[j]].crowd_dist <= pivot)
             {
                 i += 1;
                 temp = dist[j];
                 dist[j] = dist[i];
                 dist[i] = temp;
             }
         }
         index = i + 1;
         temp = dist[index];
         dist[index] = dist[right];
         dist[right] = temp;
         q_sort_dist(pop, dist, left, index - 1);
         q_sort_dist(pop, dist, index + 1, right);
     }
     return;
 }