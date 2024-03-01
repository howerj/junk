#include <gtest/gtest.h>

#include "xs.h"

TEST(XS, Init)
{
  state_t s;

  init(s);

  EXPECT_EQ(s, state_t({123456789, 362436069, 521288629, 88675123}));
}

TEST(XS, StepForward)
{
  state_t s;

  init(s);

  step_forward(s);
  EXPECT_EQ(s, state_t({0x159A55E5, 0x1F123BB5, 0x05491333, 0xDCA345EA}));

  step_forward(s);
  EXPECT_EQ(s, state_t({0x1F123BB5, 0x05491333, 0xDCA345EA, 0x1B5116E6}));

  step_forward(s);
  EXPECT_EQ(s, state_t({0x05491333, 0xDCA345EA, 0x1B5116E6, 0x951049AA}));

  step_forward(s);
  EXPECT_EQ(s, state_t({0xDCA345EA, 0x1B5116E6, 0x951049AA, 0xD88D00B0}));

  step_forward(s);
  EXPECT_EQ(s, state_t({0x1B5116E6, 0x951049AA, 0xD88D00B0, 0x1EC7825E}));
}

TEST(XS, StepBackward)
{
  const size_t k = 10;

  state_t s_1;

  init(s_1);

  state_t s_2 = s_1;

  for(size_t i = 0; i < k; i++)
    step_forward(s_2);

  for(size_t i = 0; i < k; i++)
    step_backward(s_2);

  EXPECT_EQ(s_1, s_2);
}

TEST(XS, GenU32)
{
  state_t s;

  init(s);

  using sample_t = std::array<uint32_t, 5>;

  sample_t rv;

  for(size_t i = 0; i < rv.size(); i++)
  {
    rv[i] = gen_u32(s);
  }

  EXPECT_EQ(rv, sample_t({
    0xDCA345EA,
    0x1B5116E6,
    0x951049AA,
    0xD88D00B0,
    0x1EC7825E
  }));
}

TEST(XS, StepForwardMatrix)
{
  uint64_t k = 10;

  tr_t T_k;

  prepare_transition(T_k, k, Direction::FWD);

  state_t s_1, s_2;

  init(s_1);

  s_2 = s_1;

  do_transition(s_1, T_k);

  for(uint64_t i = 0 ; i < k; i++)
    step_forward(s_2);

  EXPECT_EQ(s_1, s_2);
}

TEST(XS, StepBackwardMatrix)
{
  uint64_t k = 10;

  tr_t T_k;

  prepare_transition(T_k, k, Direction::BWD);

  state_t s_1, s_2;

  init(s_1);

  s_2 = s_1;

  for(uint64_t i = 0 ; i < k; i++)
    step_forward(s_2);

  do_transition(s_2, T_k);

  EXPECT_EQ(s_1, s_2);
}

TEST(XS, StepForwardPoly)
{
  init_transition_polynomials();

  uint64_t k = 10;

  tr_poly_t t_k;

  prepare_transition(t_k, k, Direction::FWD);

  state_t s_1, s_2;

  init(s_1);

  s_2 = s_1;

  do_transition(s_1, t_k);

  for(uint64_t i = 0 ; i < k; i++)
    step_forward(s_2);

  EXPECT_EQ(s_1, s_2);
}

TEST(XS, StepBackwardPoly)
{
  init_transition_polynomials();

  uint64_t k = 10;

  tr_poly_t t_k;

  prepare_transition(t_k, k, Direction::BWD);

  state_t s_1, s_2;

  init(s_1);

  s_2 = s_1;

  for(uint64_t i = 0 ; i < k; i++)
    step_forward(s_2);

  do_transition(s_2, t_k);

  EXPECT_EQ(s_1, s_2);
}

TEST(XS, StepForwardCross)
{
  init_transition_polynomials();

  uint64_t k = 12345678;

  tr_t T_k;

  tr_poly_t t_k;

  prepare_transition(t_k, k, Direction::FWD);

  prepare_transition(T_k, k, Direction::FWD);

  state_t s_1, s_2;

  init(s_1);

  s_2 = s_1;

  do_transition(s_1, T_k);
  do_transition(s_2, t_k);

  EXPECT_EQ(s_1, s_2);

}
