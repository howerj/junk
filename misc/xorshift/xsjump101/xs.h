#ifndef __xs_h__
#define __xs_h__

#include <cstdio>
#include <climits>
#include <array>
#include <iosfwd>

#include <NTL/GF2x.h>

using state_t = std::array<uint32_t, 4>;

const size_t STATE_SIZE_EXP  = 128;
const size_t STATE_WORD_SIZE = CHAR_BIT * sizeof(uint32_t);
const size_t STATE_SIZE      = STATE_SIZE_EXP / STATE_WORD_SIZE;

static_assert(STATE_WORD_SIZE * STATE_SIZE == STATE_SIZE_EXP, "Inconsistent state parameters");

std::ostream &operator <<(std::ostream &os, const state_t &s);

void init(state_t &s);

void step_forward(state_t &s);

void step_backward(state_t &s);

uint32_t gen_u32(state_t &s);

///////////////////////////////////////////////////////////////////////////
//
// Matrix-based transition
//
///////////////////////////////////////////////////////////////////////////

using tr_t = uint32_t[STATE_SIZE_EXP][STATE_SIZE];

inline uint32_t get_bit(const tr_t A, size_t row, size_t col)
{
  size_t col_ndx = col / 32;
  size_t col_ofs = col % 32;

  return (A[row][col_ndx] >> col_ofs) & 0x00000001u;
}

inline void set_bit(tr_t A, size_t row, size_t col, uint32_t val)
{
  size_t col_ndx = col / 32;
  size_t col_ofs = col % 32;

  uint32_t mask = 1u << col_ofs;

  A[row][col_ndx] = (A[row][col_ndx] & ~mask) | (val << col_ofs);
}

void calculate_bwd_transition_matrix(tr_t invT);

enum class Direction {FWD, BWD};

void prepare_transition(tr_t T_k, uint64_t k, Direction dir);

void do_transition(state_t &s, const tr_t T);

///////////////////////////////////////////////////////////////////////////
//
// Polynomial-based transition
//
///////////////////////////////////////////////////////////////////////////

void init_transition_polynomials();

struct tr_poly_t
{
  NTL::GF2X poly;
  Direction dir;
};

void prepare_transition(tr_poly_t &tr_k, uint64_t k, Direction dir);

void do_transition(state_t &s, const tr_poly_t &tr);

#endif
