/**
 * @file simulation_model.h
 * @author Melissa Ip
 *
 * The SimulationModel class is used to validate TrioModel results using
 * default parameter values described in the TrioModel class. It generates a
 * random family pedigree based on population priors and calculates the
 * probability of mutation using the generated sample (sequencing reads are
 * drawn from the Dirichlet multinomial).
 */
#ifndef SIMULATION_MODEL_H
#define SIMULATION_MODEL_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "trio_model.h"


/**
 * SimulationModel class header. See top of file for a complete description.
 */
class SimulationModel {
 public:
  SimulationModel(unsigned int coverage, double germline_mutation_rate,  // No default constructor, because all parameters are given through command line inputs.
                  double somatic_mutation_rate);
  void Seed();  // Seeds random number generator during initialization.
  void Free();
  void WriteProbability(const string &file_name, int size);  // Generates random samples and probabilities in text file.
  void WriteMutationCounts(const string &file_name, int size);
  void PrintMutationCounts(int size); // Simulates trios to stdout.
  unsigned int coverage();  // Get and set functions.
  void set_coverage(unsigned int coverage);
  double germline_mutation_rate();
  void set_germline_mutation_rate(double rate);
  double somatic_mutation_rate();
  void set_somatic_mutation_rate(double rate);
  bool has_mutation();
  void set_has_mutation(bool has_mutation);

 private:
  int Mutate(int genotype_idx, bool is_germline=false,
             int parent_genotype_idx=-1);
  int GetChildGenotype(int mother_genotype, int father_genotype);
  ReadData DirichletMultinomialSample(int genotype_idx);
  vector<RowVectorXi> GetGenotypesMatrix(int size);
  TrioVector GetRandomTrios(int size);
  int RandomDiscreteChoice(size_t K, const RowVectorXd &probabilities);
  RowVectorXi RandomDiscreteChoice(size_t K, const RowVectorXd &probabilities,
                                   int size);

  // Instance member variables.
  TrioModel params_;  // Default initialization.
  unsigned int coverage_;
  bool has_mutation_;
  vector<bool> has_mutation_vec_;
  vector<bool> mutation_table_[42875];
  gsl_rng *r;
};

#endif
