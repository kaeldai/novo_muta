/**
 * @file pileup_utility.h
 * @author Melissa Ip
 *
 * This file contains methods to parse and align the the sequences in .pileup
 * files. Pileup files must be in the format described here:
 *
 * http://samtools.sourceforge.net/pileup.shtml
 *
 * This can create a TrioModel object using the parsed sequencing reads,
 * and write the probability of mutation to a text file.
 */
#ifndef PILEUP_UTILITY_H
#define PILEUP_UTILITY_H

#include <fstream>
#include <iterator>
#include <sstream>

#include "trio_model.h"


namespace PileupUtility {
  string GetSequence(string &line);
  string TrimHeader(ifstream &f);
  ReadData GetReadData(const string &line);
  double GetProbability(TrioModel &params, const string &child_line,
                        const string &mother_line, const string &father_line);
  void WriteProbability(const string &file_name, const string &child_pileup,
                        const string &mother_pileup, const string &father_pileup);
};

#endif
