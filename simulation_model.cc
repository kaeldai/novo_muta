/**
 * @file simulation_model.cc
 * @author Melissa Ip
 *
 * This file contains the implementation of the SimulationModel class.
 *
 * See top of simulation_model.h for a complete description.
 */
#include "simulation_model.h"


/**
 * Constructor to initialize SimulationModel object that contains necessary
 * parameters for the random generation of samples and probabilities.
 *
 * has_mutation_ keeps track of whether each site contains a mutation.
 *
 * @param  coverage               Coverage.
 * @param  germline_mutation_rate Germline mutation rate.
 * @param  somatic_mutation_rate  Somatic mutation rate.
 */
SimulationModel::SimulationModel(unsigned int coverage,
                                 double germline_mutation_rate,
                                 double somatic_mutation_rate)
    :  coverage_{coverage} {
  params_.set_germline_mutation_rate(germline_mutation_rate);
  params_.set_somatic_mutation_rate(somatic_mutation_rate);
  SimulationModel::Seed();
}

/**
 * Seeds rand function with time and GSL functions with random number generator.
 */
void SimulationModel::Seed() {
  srand(time(NULL));
  // Creates a generator chosen by the environment variable GSL_RNG_TYPE.
  const gsl_rng_type *T;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
}

/**
 * Frees GSL random number generator.
 */
void SimulationModel::Free() {
  gsl_rng_free(r);
}

/**
* Mutates a numeric genotype based on either germline or somatic transition
* matrix in the TrioModel.
*
* is_germline is false by default to process somatic mutation. If this is set
* to true, then this method will process germline mutation and assume
* parent_genotype_idx is not -1.
*
* @param  genotype_idx        Index of genotype.
* @param  is_germline         False by default. Set to true to process germline
*                             mutation.
* @param  parent_genotype_idx Index of parent genotype.
* @return                     Index of mutated genotype.
*/
int SimulationModel::Mutate(int genotype_idx, bool is_germline,
                            int parent_genotype_idx) {
  // Sets probability matrices to use either germline or somatic probabilities.
  RowVector16d mat;
  if (!is_germline) {
    mat = params_.somatic_probability_mat().row(genotype_idx);
  } else {
    mat = params_.germline_probability_mat().col(parent_genotype_idx);
  }

  // Randomly mutates the genotype using the probabilities as weights.
  int mutated_genotype_idx = SimulationModel::RandomDiscreteChoice(
    kGenotypeCount,
    mat
  );
  if (mutated_genotype_idx != genotype_idx) {
    params_.set_has_mutation(true);
  }
  return mutated_genotype_idx;
}

/**
 * Generates a numeric child genotype by picking a random allele from each of
 * the given numeric parent genotypes.
 *
 * @param  mother_genotype Numeric mother genotype.
 * @param  father_genotype Numeric father genotype.
 * @return                 Numeric child genotype.
 */
int SimulationModel::GetChildGenotype(int mother_genotype, int father_genotype) {
  int child_allele1 = kGenotypeNumIndex(mother_genotype, rand() % 2);
  int child_allele2 = kGenotypeNumIndex(father_genotype, rand() % 2);
  for (int i = 0; i < kGenotypeCount; ++i) {
    if (child_allele1 == kGenotypeNumIndex(i, 0) &&
        child_allele2 == kGenotypeNumIndex(i, 1)) {
      return i;
    }
  }
  return -1;  // ERROR: This should not happen.
}

/**
 * Uses alpha frequencies based on the somatic genotype to select nucleotide
 * frequencies and uses these frequencies to draw sequencing reads at a
 * specified coverage (Dirichlet multinomial). K is kNucleotideCount.
 *
 * @param  genotype_idx Index of genotype.
 * @return              Read counts drawn from Dirichlet multinomial.
 */
ReadData SimulationModel::DirichletMultinomialSample(int genotype_idx) {
  // Converts alpha to double array.
  auto alpha_vec = params_.alphas().row(genotype_idx);
  const double alpha[kNucleotideCount] = {alpha_vec(0), alpha_vec(1),
                                          alpha_vec(2), alpha_vec(3)};

  // Sets alpha frequencies using dirichlet distribution in theta.
  double theta[kNucleotideCount] = {0.0};
  gsl_ran_dirichlet(r, kNucleotideCount, alpha, theta);

  // Sets sequencing reads using multinomial distribution in reads.
  unsigned int reads[kNucleotideCount] = {0};
  gsl_ran_multinomial(r, kNucleotideCount, coverage_, theta, reads);

  // Converts reads to ReadData.
  ReadData data = {0};
  for (int i = 0; i < kNucleotideCount; ++i) {
    data.reads[i] = reads[i];
  }
  return data;
}

/**
 * Generates experiment_count random samples using population priors as weights
 * and outputs their probabilities and whether that site contains a mutation
 * (1=true, 0=false) to a text file. The file is tab deliminated and each site
 * is on a new line.
 *
 * @param  file_name              File name.
 * @param  experiment_count       Number of experiments.
 */
void SimulationModel::WriteProbability(const string &file_name,
                                       int experiment_count) {
  // Generates experiment_count random samples using population priors as weights.
  RowVectorXi parent_genotypes = SimulationModel::RandomDiscreteChoice(
    kGenotypeCount * kGenotypeCount,
    params_.population_priors(),
    experiment_count
  );

  // Extracts father genotype indices from samples.
  RowVectorXi father_genotypes(experiment_count);
  father_genotypes = parent_genotypes / kGenotypeCount;

  ofstream fout(file_name);
  for (int i = 0; i < experiment_count; ++i) {
    // Extracts mother genotype indices from samples.
    int mother_genotype = parent_genotypes(i) % kGenotypeCount;
    int father_genotype = father_genotypes(i);

    // Creates child genotype by picking a random allele from each parent.
    int child_genotype = SimulationModel::GetChildGenotype(mother_genotype,
                                                           father_genotype);

    // Processes germline mutation. Germline matrix requires no Kronecker.
    int child_germline_genotype = SimulationModel::Mutate(
      child_genotype,
      true,
      mother_genotype*kGenotypeCount + father_genotype
    );

    // Processes somatic mutation.
    int child_somatic_genotype = SimulationModel::Mutate(child_germline_genotype);
    int mother_somatic_genotype = SimulationModel::Mutate(mother_genotype);
    int father_somatic_genotype = SimulationModel::Mutate(father_genotype);

    // Creates reads from somatic genotypes using the Dirichlet multinomial.
    ReadData child_read = SimulationModel::DirichletMultinomialSample(child_somatic_genotype);
    ReadData mother_read = SimulationModel::DirichletMultinomialSample(mother_somatic_genotype);
    ReadData father_read = SimulationModel::DirichletMultinomialSample(father_somatic_genotype);
    ReadDataVector data_vec = {child_read, mother_read, father_read};

    // Writes probability to text file.
    double probability = params_.MutationProbability(data_vec);
    fout << probability << "\t" << params_.has_mutation() << "\n";
    params_.set_has_mutation(false);  // Resets for the next simulation.
  }
  fout.close();
}

/**
 * Generates a random sample from the range [0, K) using K size probabilities
 * as weights.
 *
 * @param  K              Number of cateogories in probabilities.
 * @param  probabilitites RowVector of probabilities associated with each entry
 *                        in the samples generated by K.
 * @return                Random element based on probabilities from samples
 *                        generated by K.
 */
int SimulationModel::RandomDiscreteChoice(size_t K,
                                          const RowVectorXd &probabilities) {
  // Converts probabilities to double array p.
  int length = probabilities.size();
  double p[length];
  for (int i = 0; i < length; ++i) {
    p[i] = probabilities(i);
  }

  // Creates discrete random number generator.
  gsl_ran_discrete_t *g = gsl_ran_discrete_preproc(K, p);
  size_t random = gsl_ran_discrete(r, g);
  gsl_ran_discrete_free(g);
  
  return (int) random;
}

/**
 * Generates RowVector of random samples from the range [0, K) using K size
 * probabilities as weights.
 *
 * @param  K              Number of cateogories in probabilities.
 * @param  probabilitites RowVector of probabilities associated with each entry
 *                        in the samples generated by K.
 * @param  size           Number of samples.
 * @return                Random samples based on probabilities from samples
 *                        generated by K.
 */
RowVectorXi SimulationModel::RandomDiscreteChoice(
    size_t K, const RowVectorXd &probabilities, int size) {
  // Creates size RowVector to hold random samples.
  RowVectorXi random_samples(size);
  for (int i = 0; i < size; ++i) {
    random_samples(i) = SimulationModel::RandomDiscreteChoice(K, probabilities);
  }
  return random_samples;
}

unsigned int SimulationModel::coverage() {
  return coverage_;
}

void SimulationModel::set_coverage(unsigned int coverage) {
  coverage_ = coverage;
}

double SimulationModel::germline_mutation_rate() {
  return params_.germline_mutation_rate();
}

void SimulationModel::set_germline_mutation_rate(double rate) {
  params_.set_germline_mutation_rate(rate);
}

double SimulationModel::somatic_mutation_rate() {
  return params_.somatic_mutation_rate();
}

void SimulationModel::set_somatic_mutation_rate(double rate) {
  params_.set_somatic_mutation_rate(rate);
}