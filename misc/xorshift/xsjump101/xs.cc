#include "xs.h"
#include <iostream>

std::ostream &operator <<(std::ostream &os, const state_t &s)
{
  char buf[40];
  
  sprintf(buf, "%08X-%08X-%08X-%08X", s[0], s[1], s[2], s[3]);

  os << buf;

  return os;
}

void init(state_t &s)
{
  s[0] = 123456789;
  s[1] = 362436069;
  s[2] = 521288629;
  s[3] = 88675123;
}

void step_forward(state_t &s)
{
  uint32_t &x = s[0], &y = s[1], &z = s[2], &w = s[3];
 
  uint32_t t = x ^ (x << 11);

  x = y; y = z; z = w;
  w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}

void step_backward(state_t &s)
{
  uint32_t &x = s[0], &y = s[1], &z = s[2], &w = s[3];

  uint32_t tt = w ^ (z ^ z >> 19); // (t ^ (t >> 8))

  uint32_t t_3 = tt & 0xFF000000U;
  uint32_t t_2 = (tt & 0x00FF0000U) ^ (t_3 >> 8);
  uint32_t t_1 = (tt & 0x0000FF00U) ^ (t_2 >> 8);
  uint32_t t_0 = (tt & 0x000000FFU) ^ (t_1 >> 8);

  uint32_t t = t_3 | t_2 | t_1 | t_0; // x ^ (x << 11)

  uint32_t x_10_00 = t & 0x000007FFU;
  uint32_t x_21_11 = (t & 0x003FF800U) ^ (x_10_00 << 11);
  uint32_t x_31_22 = (t & 0xFFC00000U) ^ (x_21_11 << 11);

  w = z;
  z = y;
  y = x;
  x = x_31_22 | x_21_11 | x_10_00;
}

uint32_t gen_u32(state_t &s)
{
  step_forward(s);

  return s[3];
}

///////////////////////////////////////////////////////////////////////////
//
// Matrix-based transition
//
///////////////////////////////////////////////////////////////////////////

extern tr_t T;

void mat_mul(tr_t C, const tr_t A, const tr_t B)
{
  tr_t T;

  for(size_t i = 0; i < 128; i++)
  {
    for(size_t j = 0; j < 128; j++)
    {
       uint32_t v = 0;

       for(size_t k = 0; k < 128; k++)
        v ^= get_bit(A, i, k) & get_bit(B, k, j);

       set_bit(T, i, j, v);
    }
  }

  for(size_t i = 0; i < 128; i++)
  {
    for(size_t j = 0; j < 4; j++)
    {
      C[i][j] = T[i][j];
    }
  }
}

void mat_pow(tr_t B, const tr_t A, uint64_t n)
{
    tr_t T;

    for(size_t i = 0; i < 128; i++)
    {
	for(size_t j = 0; j < 4; j++)
	{
	    T[i][j] = A[i][j];
	    B[i][j] = 0;
	}

	set_bit(B, i, i, 1);
    }

    for(uint64_t k = n; k > 0; k /= 2)
    {
	if(k % 2)
	{
	    mat_mul(B, T, B);
	}

       mat_mul(T, T, T);
    }
}

void calculate_bwd_transition_matrix(tr_t invT)
{
    tr_t tmpT;

    for(size_t i = 0; i < 128; i++)
    {
	for(size_t j = 0; j < 4; j++)
	{
	    tmpT[i][j] = T[i][j];
	    invT[i][j] = 0;
	}

	set_bit(invT, i, i, 1);
    }

    for(size_t i = 0; i < 128; i++)
    {
        if(i > 0)
            mat_mul(invT, tmpT, invT);

        mat_mul(tmpT, tmpT, tmpT);
    }
}

void prepare_transition(tr_t T_k, uint64_t k, Direction dir)
{
  switch(dir)
  {
    case Direction::FWD:
      mat_pow(T_k, T, k);
      break;

    case Direction::BWD:
    {
      static bool invT_initialized = false;

      static tr_t invT;

      if(!invT_initialized)
      {
	calculate_bwd_transition_matrix(invT);
	invT_initialized = true;
      }

      mat_pow(T_k, invT, k);
      break;
    }
  }
}

void do_transition(state_t &s, const tr_t T)
{
  state_t t;

  for(size_t i = 0; i < 4; i++)
  {
    t[i] = 0;

    for(size_t j = 0; j < 32; j++)
    {
      uint32_t row_ndx = i * 32 + j;

      uint32_t w = 0;

      for(size_t k = 0; k < 4; k++)
	w ^= T[row_ndx][k] & s[k];

      uint32_t u = w ;

      for(size_t k = 1; k < 32; k++)
	u ^= (w << k);

      t[i] = t[i] >> 1 | (u & 0x80000000U);
    }
  }

  s = t;
}

///////////////////////////////////////////////////////////////////////////
//
// Polynomial-based transition
//
///////////////////////////////////////////////////////////////////////////

NTL::GF2XModulus fwd_step_mod;
NTL::GF2XModulus bwd_step_mod;

void init_transition_polynomials()
{
  state_t s;

  init(s);

  const size_t N = 2*STATE_SIZE_EXP;

  NTL::vec_GF2 vf(NTL::INIT_SIZE, N);
  NTL::vec_GF2 vb(NTL::INIT_SIZE, N);

  for(long i = 0; i < N; i++)
  {
    step_forward(s);

    vf[i] = vb[N - 1 - i] = s[3] & 0x01ul;
  }

  NTL::GF2X fwd_step_poly;
  NTL::GF2X bwd_step_poly;

  NTL::MinPolySeq(fwd_step_poly, vf, STATE_SIZE_EXP);
  NTL::MinPolySeq(bwd_step_poly, vb, STATE_SIZE_EXP);

  NTL::build(fwd_step_mod, fwd_step_poly);
  NTL::build(bwd_step_mod, bwd_step_poly);
}

void prepare_transition(tr_poly_t &tr_k, uint64_t k, Direction dir)
{
  tr_k.dir = dir;

  NTL::GF2X x(1, 1);

  switch(dir)
  {
    case Direction::FWD:
      NTL::PowerMod(tr_k.poly, x, k, fwd_step_mod);
      break;

    case Direction::BWD:
      NTL::PowerMod(tr_k.poly, x, k, bwd_step_mod);
      break;
  }
}

void add_state(state_t &x, const state_t &y)
{
  for(size_t i = 0; i < x.size(); i++)
    x[i] = x[i] ^ y[i];
}

void horner(state_t &s, const tr_poly_t &tr)
{
  state_t tmp_state = s;

  tmp_state.fill(0);

  int i = NTL::deg(tr.poly);

  if(i > 0)
  {
    for( ; i > 0; i--)
    {
      if(NTL::coeff(tr.poly, i) != 0)
	add_state(tmp_state, s);

      if(tr.dir == Direction::FWD)
	step_forward(tmp_state);
      else
	step_backward(tmp_state);
    }

    if(NTL::coeff(tr.poly, 0) != 0)
      add_state(tmp_state, s);
  }

  s = tmp_state;
}

void do_transition(state_t &s, const tr_poly_t &tr)
{
  horner(s, tr);
}
