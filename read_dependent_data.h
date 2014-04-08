/**
 * @file read_dependent_data.h
 * @author Melissa Ip
 *
 * The ReadDependentData contains the members that are dependent on
 * ReadData/ReadDataVector including matrices and vectors that are calculated
 * from sequencing error and by the tree-peeling method. 
 *
 * The simulation program needs to be refactored to use this class.
 */
#ifndef READ_DEPENDENT_DATA_H
#define READ_DEPENDENT_DATA_H

#include "utilities.cc"

/**
 * ReadDependentData class header. See top of file for a complete description.
 */
class ReadDependentData {
public:
  // Default constructor.
  ReadDependentData();

  // Instance variables.
  vector<double> max_elements;  // stores max element of sequencing_probability_mat when rescaling to normal space
  Matrix3_16d sequencing_probability_mat;  // P(R|somatic genotype)
  RowVector16d child_vec;
  RowVector16d mother_vec;
  RowVector16d father_vec;
  struct TreePeels {
    RowVector16d child_probability;  // P(R|zygotic genotype)
    RowVector16d mother_probability;
    RowVector16d father_probability;
    RowVector256d child_germline_probability;
    RowVector256d parent_probability;  // P(R|mom and dad genotype)
    RowVector256d root_mat;  // P(R|mom and dad genotype) * P(mom and dad genotype)
    double sum;  // P(R)
  } denominator, numerator;
  bool has_mutation;  // simulation only
};

#endif