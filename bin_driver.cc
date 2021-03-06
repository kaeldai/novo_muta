/**
 * @file bin_driver.cc
 * @author Melissa Ip
 *
 * This file handles four cases:
 *
 * 1. Parses file generated by simulation_driver.cc.
 *
 *    Column #
 *           1  Probability of mutation (double [0, 1]).
 *           2  Whether the site contains a mutation (1 true, 0 false).
 * 
 * 2. Parses file generated by simulation_trio.cc.
 *
 *    Column #
 *           1  Probability of mutation (double [0, 1]).
 *
 * 3. Parses file generated by simulation_driver.cc.
 *
 *    Column #
 *           1  Index of trio in the reference TrioVector at 4x coverage.
 *           2  Number of random trios that have a mutation.
 *           3  Number of random trios that do not have a mutation.  
 *
 *    For every site/row, the sum of column 2 and 3 is the total number of
 *    random trios that match the key trio (given its index in the first
 *    column).
 *
 *    This calculates the empirical probability for each trio using
 *
 *    P(mutation|trio) = #trios with mutation / #total trios.
 *
 *    The probabilities should match the probabilities from the result of
 *    MutationProbability() from simulation_trio.cc, which outputs the
 *    probability for each trio on a new line.
 *
 * 4. Parses file generated by simulation_driver.cc when running multiple jobs
 *    in parallel and will have more than one set of counts for each trio index.
 *    Thus, this keeps track of all data by index. See case 3.
 *
 * In all four cases, each site is placed on a new line.
 *
 * This creates 10 bins numbered 0-9 with probability cateogories at 10%
 * intervals:
 * 
 * BIN   0        1         2        ...   9
 * %    [0, 10), [10, 20), [20, 30), ..., [90, 100]
 * 
 * This calculates the percentage of the sites in each bin that contain a real
 * mutation (value in column 2 is 1). The digit in the tenths place of
 * the probability represents the number of the bin it belongs to. A probability
 * of 1.00 (100%) will go in the highest bin possible, bin 9.
 *
 * To compile on Herschel:
 * c++ -std=c++11 -L/usr/local/lib -I/usr/local/include -o bin_driver utility.cc bin_driver.cc
 *
 * To run this file, provide the following command line inputs:
 * ./bin_driver <input>.txt
 */
#include <fstream>
#include <sstream>

#include "utility.h"

const int kNumBins = 10;  // 10 bins cover 0-100% with 10% intervals.

/**
 * See case 1.
 *
 * @param  fin  File input stream.
 */
void CountBin(ifstream &fin) {
  string line;
  int bin = 0;
  int has_mutation = 0;
  int counts[kNumBins] = {0};
  int totals[kNumBins] = {0};
  double probability = 0.0;
  double has_mutation_percent = 0.0;

  while (getline(fin, line)) {
    line.erase(remove(line.begin(), line.end(), '\n'), line.end());
    stringstream str(line);
    str >> probability;
    str >> has_mutation;
    bin = (int) fmin(floor(probability * kNumBins), kNumBins - 1);
    totals[bin]++;
    if (has_mutation == 1) {
      counts[bin]++;
    }
  }
  fin.close();

  for (int i = 0; i < kNumBins; ++i) {
    if (totals[i] > 0) {
      has_mutation_percent = (double) counts[i] / totals[i] * 100;
      printf("%.2f%% or %d/%d sites in bin %d contain a mutation.\n",
             has_mutation_percent, counts[i], totals[i], i);
    } else {
      printf("There are no sites in bin %d.\n", i);
    }
  }
}

/**
 * See case 2.
 *
 * @param  fin  File input stream.
 */
void CountBinTrio(ifstream &fin) {
  string line;
  int bin = 0;
  int total = 0;
  int probability_count = 0;  // Number of sites above the probability cut.
  int counts[kNumBins] = {0};
  double probability;
  double probability_cut = 0.1;

  while (getline(fin, line)) {
    line.erase(remove(line.begin(), line.end(), '\n'), line.end());
    stringstream str(line);
    str >> probability;
    bin = (int) fmin(floor(probability * kNumBins), kNumBins - 1);
    total++;

    if (probability > probability_cut) {
      probability_count++;
    }

    if (bin < 0) {
      Die("Negative probability.");
    } else {
      counts[bin]++;
    }
  }
  fin.close();
  
  double percent = (double) probability_count / total * 100;
  printf("%.2f%% or %d/%d sites have a probability greater than %.2f.\n",
         percent, probability_count, total, probability_cut);

  for (int i = 0; i < kNumBins; ++i) {
    if (counts[i] > 0) {
      percent = (double) counts[i] / total * 100;
      printf("%.2f%% or %d/%d sites in bin %d.\n",
             percent, counts[i], total, i);
    } else {
      printf("There are no sites in bin %d.\n", i);
    }
  }
}

/**
 * See case 3.
 *
 * @param  fin  File input stream.
 */
void CountProbability(ifstream &fin) {
    string output_name;
    cout << "Provide an output file name: ";
    getline(cin, output_name);
    ofstream fout(output_name);

    string line;
    int index = 0;  // Unused placeholder.
    int total_trios = 0;
    int has_mutation_total = 0;
    int has_no_mutation_total = 0;
    double probability = 0.0;
    vector<double> probabilities;
 
    while (getline(fin, line)) {
      line.erase(remove(line.begin(), line.end(), '\n'), line.end());
      stringstream str(line);
      str >> index;
      str >> has_mutation_total;
      str >> has_no_mutation_total;
      total_trios = has_mutation_total + has_no_mutation_total;
      if (total_trios == 0) {
        probability = 0.0;
      } else {
        probability = (double) has_mutation_total / total_trios;
      }
      probabilities.push_back(probability);
    }
    fin.close();

    ostream_iterator<double> output_iter(fout, "\n");
    copy(probabilities.begin(), probabilities.end(), output_iter);
    fout.close();
}

/**
 * See case 4.
 *
 * @param  fin  File input stream.
 */
void CountProbabilityIndex(ifstream &fin) {
  string output_name;
  cout << "Provide an output file name: ";
  getline(cin, output_name);
  ofstream fout(output_name);

  string line;
  int index = 0;
  int total_trios = 0;
  int has_mutation_total = 0;
  int has_no_mutation_total = 0;
  int trio_totals[kTrioCount] = {0};
  int has_mutation_totals[kTrioCount] = {0};
  double probability = 0.0;
  vector<double> probabilities;

  while (getline(fin, line)) {
    line.erase(remove(line.begin(), line.end(), '\n'), line.end());
    stringstream str(line);
    str >> index;
    str >> has_mutation_total;
    str >> has_no_mutation_total;
    total_trios = has_mutation_total + has_no_mutation_total;
    has_mutation_totals[index] += has_mutation_total;
    trio_totals[index] += total_trios;
  }
  fin.close();

  for (int i = 0; i < kTrioCount; ++i) {
    if (has_mutation_totals[i] == 0) {
      probability = 0.0;
    } else {
      probability = (double) has_mutation_totals[i] / trio_totals[i];
    }
    probabilities.push_back(probability);
  }

  ostream_iterator<double> output_iter(fout, "\n");
  copy(probabilities.begin(), probabilities.end(), output_iter);
  fout.close();
}


int main(int argc, const char *argv[]) {
  if (argc < 2) {        
    Die("USAGE: bin_driver <input>.txt");
  }

  const string file_name = argv[1];
  char case_num = '0';  // Initially not a valid case number.
  cout << "Provide a case number: ";
  cin.get(case_num);
  cin.ignore(20, '\n');  // Flush buffer.
  cout << endl;

  ifstream fin(file_name);
  if (!fin.is_open() || 0 != fin.fail()) {
    Die("Input file cannot be read.");
  }
  
  switch (case_num) {
  case '1':
    CountBin(fin);
    break;
  case '2':
    CountBinTrio(fin);
    break;
  case '3':
    CountProbability(fin);
    break;
  case '4':
    CountProbabilityIndex(fin);
    break;
  }

  return 0;
}
