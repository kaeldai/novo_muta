/**
 * @file sufficient_statistics.h
 * @author Melissa Ip
 *
 * The SufficientStatistics class is used to hold ~S_T of type T sufficient
 * statistics produced in the E-Step of the expectation-maximization algorithm.
 * This also calculates the maximized-likelihood estimates for the M-Step.
 */
#ifndef SUFFICIENT_STATISTICS_H
#define SUFFICIENT_STATISTICS_H

#include "em_algorithm.h"

/**
 * SufficientStatistics class header. See top of file for a complete description.
 */
class SufficientStatistics {
 public:
  SufficientStatistics(double sites_count);  // Default constructor.
  void Update(TrioModel &params, const TrioVector &sites);
  void Clear();
  void Print();

  double MaxGermlineMutationRate();  // M-step functions.
  double MaxSomaticMutationRate();
  double MaxSequencingErrorRate();
  bool IsNan();

  double e() const;  // Get and set functions.
  double hom() const;
  double het() const;
  double som() const;
  double germ() const;
  double n_s() const;
  void set_e(double max);
  void set_hom(double max);
  void set_het(double max);
  void set_som(double max);
  void set_germ(double max);
  void set_n_s(double num);

 private:
  // Instance member variables.
  double e_;
  double hom_;
  double het_;
  double som_;
  double germ_;  // S_M + S_F.
  double n_s_;  // Number of sites.
};

#endif