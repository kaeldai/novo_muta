/**
 * @file count_bin_trio.cc
 * @author Melissa Ip
 *
 * This parses an input file, where each probability of mutation (as a float
 * [0, 1]) is placed on a new line.
 *
 * This creates 10 bins numbered 0-9 with probability cateogories at 10%
 * intervals:
 * 
 * BIN   0        1         2        ...   9
 * %    [0, 10), [10, 20), [20, 30), ..., [90, 100]
 * 
 * This calculates the percentage of the sites in each bin out of the total
 * number of sites. The digit in the tenths place of the probability represents
 * the number of the bin it belongs to. A probability of 1.00 (100%) will go in
 * the highest bin possible, bin 9. Negative probabilities are all grouped in a
 * -1 bin.
 */
#include <fstream>
#include <sstream>

#include "utility.h"

const int kNumBins = 10;  // 10 bins cover 0-100% with 10% intervals.


int main(int argc, const char *argv[]) {
  if (argc < 2) {        
    Die("USAGE: count_bin_trio <input.txt>");
  }

  const string file_name = argv[1];
  ifstream f(file_name);
  if (!f.is_open() || 0 != f.fail()) {
    Die("Input file cannot be read.");
  }

  string line;
  int bin = 0;
  int neg_bin = 0;
  int total = 0;
  int counts[kNumBins] = {0};
  int probability_count;  // Above the probability cut.
  double probability_cut = 0.1;
  double probability = 0.0;

  while (getline(f, line)) {
    line.erase(remove(line.begin(), line.end(), '\n'), line.end());
    stringstream str(line);
    str >> probability;
    bin = (int) fmin(floor(probability * kNumBins), kNumBins - 1);

    if (probability > probability_cut) {
      probability_count++;
    }

    if (bin < 0) {  // Negative probability.
      neg_bin++;
    } else {
      counts[bin]++;
    }
    total++;
  }
  f.close();
  
  double percent = (double) probability_count / total * 100;
  printf("%.2f%% or %d/%d sites have a probability greater than %.2f.\n",
         percent, probability_count, total, probability_cut);

  if (neg_bin > 0) {
    percent = (double) neg_bin / total * 100;
    printf("%.2f%% or %d/%d sites in bin %d.\n",
           percent, neg_bin, total, -1);  // All negative bins are grouped in -1.
  }

  for (int i = 0; i < kNumBins; i++) {
    if (counts[i] > 0) {
      percent = (double) counts[i] / total * 100;
      printf("%.2f%% or %d/%d sites in bin %d.\n",
             percent, counts[i], total, i);
    } else {
      printf("There are no sites in bin %d.\n", i);
    }
  }

  return 0;
}
