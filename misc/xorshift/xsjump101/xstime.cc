#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>

#include "xs.h"

#include <NTL/mat_GF2.h>

class timer
{
public:

  void tick()
  {
    starting_time = std::chrono::high_resolution_clock::now();
  }

  double tock()
  {
    auto current_time = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double, std::micro>(current_time - starting_time).count();
  }

private:
  std::chrono::high_resolution_clock::time_point starting_time = {};
};

///////////////////////////////////////////////////////////////////////////////
//
// Matrix-based transition using NTL (partial implementation)
//
///////////////////////////////////////////////////////////////////////////////

using tr_ntl_t = NTL::mat_GF2;

extern uint32_t get_bit(const tr_t A, size_t row, size_t col);

NTL::mat_GF2 T_ntl(NTL::INIT_SIZE, STATE_SIZE_EXP, STATE_SIZE_EXP);

void init_ntl_transition_matrices()
{
  extern tr_t T;

  for(size_t i = 0; i < STATE_SIZE_EXP; i++)
  {
    for(size_t j = 0; j < STATE_SIZE_EXP; j++)
      T_ntl[i][j] = get_bit(T, i, j);
  }
}

void prepare_transition(tr_ntl_t &tr_k, uint64_t k, Direction dir)
{
    NTL::power(tr_k, T_ntl, k);
}

void do_transition(state_t &s, const tr_ntl_t &tr)
{
}

///////////////////////////////////////////////////////////////////////////////
//
// Timing utilities
//
///////////////////////////////////////////////////////////////////////////////

using jump_magnitudes_t = std::vector<std::vector<uint64_t>>;

void generate_test_cases(const uint64_t msb_pow, const size_t n_trials,
			 jump_magnitudes_t &jump_magnitudes)
{
  std::mt19937 rng(42);

  jump_magnitudes.resize(msb_pow);

  for(uint64_t k = 1; k <= msb_pow; k++)
  {
    uint64_t msb = 1LL << k;

    std::uniform_int_distribution<int64_t> irng(0, msb);

    auto &bucket_magnitudes = jump_magnitudes[k - 1];

    bucket_magnitudes.clear();
    bucket_magnitudes.reserve(n_trials);

    for(size_t i = 0; i < n_trials; i++)
    {
      uint64_t jump_magnitude = msb | irng(rng);

      bucket_magnitudes.push_back(jump_magnitude);

      tr_t T_j;

      tr_poly_t t_j;

      tr_ntl_t T_ntl_j;

      prepare_transition(t_j, jump_magnitude, Direction::FWD);

      prepare_transition(T_j, jump_magnitude, Direction::FWD);

      prepare_transition(T_ntl_j, jump_magnitude, Direction::FWD);

      state_t s_1, s_2;

      init(s_1);

      s_2 = s_1;

      do_transition(s_1, T_j);
      do_transition(s_2, t_j);

      bool ntl_matrix_failed = false;

      for(size_t i = 0; i < STATE_SIZE_EXP && !ntl_matrix_failed; i++)
      {
	for(size_t j = 0; j < STATE_SIZE_EXP && !ntl_matrix_failed; j++)
	{
	  if(T_ntl_j[i][j] != get_bit(T_j, i, j))
	    ntl_matrix_failed = true;
	}
      }

      if(s_1 != s_2 || ntl_matrix_failed)
      {
	std::cout << "Failed: "
		  << std::hex << std::uppercase << std::setfill('0')
		  << std::setw(16) << jump_magnitude << std::endl;
      }
    }
  }
}

template<typename T>
void run_test(const std::string &test_name, Direction dir,
	      const uint64_t msb_pow, const size_t n_trials,
	      const jump_magnitudes_t &jump_magnitudes)
{
  timer clk;

  std::cout << std::fixed << std::setprecision(2);

  const std::string dir_s = (dir == Direction::FWD ? "FWD" : "BWD");

  for(uint64_t k = 1; k <= msb_pow; k++)
  {
    for(uint64_t jump_magnitude: jump_magnitudes[k - 1])
    {
      T T_j;

      clk.tick();

      prepare_transition(T_j, jump_magnitude, dir);

      double prep_time = clk.tock();

      std::cout << test_name <<"\t" << dir_s
		<< "\t" << k << "\t" << prep_time;

      state_t s;

      init(s);

      clk.tick();

      do_transition(s, T_j);

      double jump_time = clk.tock();

      std::cout << "\t" << jump_time << "\n";
    }
  }
}

int main(int argc, char *argv[])
{
  const uint64_t msb_pow = 62;

  const size_t n_trials = 10;

  init_transition_polynomials();

  init_ntl_transition_matrices();

  tr_t T_back;

  prepare_transition(T_back, 1, Direction::BWD);

  jump_magnitudes_t jump_magnitudes;

  generate_test_cases(msb_pow, n_trials, jump_magnitudes);

  run_test<tr_t>("matr", Direction::FWD, msb_pow, n_trials, jump_magnitudes);

  run_test<tr_poly_t>("poly", Direction::FWD, msb_pow, n_trials, jump_magnitudes);

  run_test<tr_t>("matr", Direction::BWD, msb_pow, n_trials, jump_magnitudes);

  run_test<tr_poly_t>("poly", Direction::BWD, msb_pow, n_trials, jump_magnitudes);

  run_test<tr_ntl_t>("matr_ntl", Direction::FWD, msb_pow, n_trials, jump_magnitudes);

  return 0;
}
