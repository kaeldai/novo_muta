/**
 * @file em_algorithm.h
 * @author Melissa Ip
 *
 * This file contains the implementation of the expectation-maximization
 * algorithm applied to a simplified and modified version of the trio model
 * (infinite sites model), which will be updated as necessary in Fall 2014 and
 * Spring 2015 to more complex and biologically realistic models including the
 * custom model using Dirichlet-multinomial approximations instead of
 * multinomial approximations.
 */
#ifndef EM_ALGORITHM_H
#define EM_ALGORITHM_H

#include "trio_model.h"


// E-step methods.
RowVector16d GetHomozygousMatches(const ReadData &data);
RowVector16d GetHeterozygousMatches(const ReadData &data);
RowVector16d GetMismatches(const ReadData &data);
double GetMismatchStatistic(const TrioModel &params);
double GetHomozygousStatistic(const TrioModel &params);
double GetHeterozygousStatistic(const TrioModel &params);
double GetGermlineStatistic(const TrioModel &params);
Matrix16_256d GermlineMutationCounts(const TrioModel &params);
Matrix4_16d GermlineMutationCountsSingle(const TrioModel &params);
double GetSomaticStatistic(const TrioModel &params);
Matrix16_16d SomaticMutationCounts();

#endif
