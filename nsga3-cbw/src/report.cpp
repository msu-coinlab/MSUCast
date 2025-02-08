/**
 * @file report.cpp
 * @brief Routines for storing population data into files.
 * 
 * This file contains functions for reporting population information
 * and writing data to files, including Parquet file format.
 */

 #include <arrow/api.h>
 #include <arrow/io/api.h>
 #include <arrow/type_fwd.h>
 #include <parquet/arrow/reader.h>
 #include <parquet/arrow/writer.h>
 #include <parquet/exception.h>
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 
 #include <iostream>
 
 #include "global.h"
 #include "rand.h"
 
 /**
  * @brief Prints the information of a population to a file.
  * 
  * This function iterates through the population and writes the
  * objective values, constraints, and other relevant data to the
  * specified file pointer.
  * 
  * @param pop Pointer to the population to report.
  * @param fpt File pointer to the output file.
  */
 void report_pop(population *pop, FILE *fpt)
 {
     int i, j, k;
     for (i = 0; i < popsize; i++)
     {
         for (j = 0; j < nobj; j++)
         {
             fprintf(fpt, "%e\t", pop->ind[i].obj[j]);
         }
         if (ncon != 0)
         {
             // Constraints reporting can be added here if needed
         }
         if (nreal != 0)
         {
             // Real values reporting can be added here if needed
         }
         if (nbin != 0)
         {
             for (j = 0; j < nbin; j++)
             {
                 for (k = 0; k < nbits[j]; k++)
                 {
                     fprintf(fpt, "%d\t", pop->ind[i].gene[j][k]);
                 }
             }
         }
         fprintf(fpt, "%e\t", pop->ind[i].constr_violation);
         fprintf(fpt, "%d\n", pop->ind[i].rank);
     }
     return;
 }
 
 /**
  * @brief Writes the feasible population data to a Parquet file.
  * 
  * This function collects the feasible individuals from the population
  * and writes their data to a Parquet file for further analysis.
  * 
  * @param pop Pointer to the population to report.
  * @param fpt File pointer to the output file.
  */
 void report_feasible(population *pop, FILE *fpt)
 {
     int i, j, k;
     int ncols = nobj + nreal;
     int nrows = 0;
     for (i = 0; i < popsize; i++)
     {
         if (pop->ind[i].constr_violation == 0.0 && pop->ind[i].rank == 1)
         {
             ++nrows;
         }
     }
 
     std::vector<std::vector<double>> mat(ncols, std::vector<double>(nrows));
     std::vector<double> tmp_row(ncols);
     int pop_counter = 0;
 
     std::vector<std::shared_ptr<arrow::Field>> schema_labels_vec;
     for (i = 0; i < popsize; i++)
     {
         if (pop->ind[i].constr_violation == 0.0 && pop->ind[i].rank == 1)
         {
             std::copy(pop->ind[i].obj, pop->ind[i].obj + nobj, tmp_row.begin());
 
             for (j = 0; j < nobj; j++)
             {
                 fprintf(fpt, "%e\t", pop->ind[i].obj[j]);
                 mat[j][pop_counter] = pop->ind[i].obj[j];
                 if (pop_counter == 0)
                 {
                     char tmp_str[20];
                     sprintf(tmp_str, "obj_%i", j);
                     schema_labels_vec.push_back(arrow::field(tmp_str, arrow::float64()));
                 }
             }
             if (nreal != 0)
             {
                 std::copy(pop->ind[i].xreal, pop->ind[i].xreal + nreal, tmp_row.begin() + nobj);
                 for (j = 0; j < nreal; j++)
                 {
                     fprintf(fpt, "%e\t", pop->ind[i].xreal[j]);
                     mat[nobj + j][pop_counter] = pop->ind[i].xreal[j];
                     if (pop_counter == 0)
                     {
                         char tmp_str[20];
                         sprintf(tmp_str, "x_%i", j);
                         schema_labels_vec.push_back(arrow::field(tmp_str, arrow::float64()));
                     }
                 }
             }
             if (nbin != 0)
             {
                 for (j = 0; j < nbin; j++)
                 {
                     for (k = 0; k < nbits[j]; k++)
                     {
                         fprintf(fpt, "%d\t", pop->ind[i].gene[j][k]);
                     }
                 }
             }
             ++pop_counter;
             fprintf(fpt, "\n");
         }
     }
 
     std::shared_ptr<arrow::Schema> schema = arrow::schema(schema_labels_vec);
     std::vector<std::shared_ptr<arrow::Array>> schema_values_vec;
     for (auto col : mat)
     {
         arrow::DoubleBuilder double_builder;
         PARQUET_THROW_NOT_OK(double_builder.AppendValues(col));
         std::shared_ptr<arrow::Array> double_array;
         PARQUET_THROW_NOT_OK(double_builder.Finish(&double_array));
         schema_values_vec.push_back(double_array);
     }
 
     std::shared_ptr<arrow::Table> table = arrow::Table::Make(schema, schema_values_vec);
     // Write the table to a Parquet file
     write_parquet_file(*table);
     return;
 }