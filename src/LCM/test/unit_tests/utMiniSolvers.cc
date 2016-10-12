//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//
#include <gtest/gtest.h>
#include <MiniLinearSolver.h>
#include <MiniNonlinearSolver.h>
#include <MiniSolvers.h>

// Why is this needed?
bool TpetraBuild = false;

int
main(int ac, char * av[])
{
  Kokkos::initialize();

  ::testing::GTEST_FLAG(print_time) = (ac > 1) ? true : false;

  ::testing::InitGoogleTest(&ac, av);

  auto const
  retval = RUN_ALL_TESTS();

  Kokkos::finalize();

  return retval;
}

//
// Test the LCM mini minimizer.
//
TEST(AlbanyResidual, NewtonBanana)
{
  bool const
  print_output = ::testing::GTEST_FLAG(print_time);

  // outputs nothing
  Teuchos::oblackholestream
  bhs;

  std::ostream &
  os = (print_output == true) ? std::cout : bhs;

  using EvalT = PHAL::AlbanyTraits::Residual;
  using ScalarT = typename EvalT::ScalarT;
  using ValueT = typename Sacado::ValueType<ScalarT>::type;

  constexpr
  Intrepid2::Index
  DIM{2};

  using MIN = Intrepid2::Minimizer<ValueT, DIM>;
  using FN = LCM::Banana<ValueT>;
  using STEP = Intrepid2::StepBase<FN, ValueT, DIM>;

  MIN
  minimizer;

  std::unique_ptr<STEP>
  pstep =
      Intrepid2::stepFactory<FN, ValueT, DIM>(Intrepid2::StepType::NEWTON);

  assert(pstep->name() != nullptr);

  STEP &
  step = *pstep;

  FN
  banana;

  Intrepid2::Vector<ScalarT, DIM>
  x;

  x(0) = 0.0;
  x(1) = 3.0;

  LCM::MiniSolver<MIN, STEP, FN, EvalT, DIM>
  mini_solver(minimizer, step, banana, x);

  minimizer.printReport(os);

  ASSERT_EQ(minimizer.converged, true);
}

//
// Test the LCM mini minimizer.
//
TEST(AlbanyJacobian, NewtonBanana)
{
  bool const
  print_output = ::testing::GTEST_FLAG(print_time);

  // outputs nothing
  Teuchos::oblackholestream
  bhs;

  std::ostream &
  os = (print_output == true) ? std::cout : bhs;

  using EvalT = PHAL::AlbanyTraits::Jacobian;
  using ScalarT = typename EvalT::ScalarT;
  using ValueT = typename Sacado::ValueType<ScalarT>::type;

  constexpr
  Intrepid2::Index
  DIM{2};

  using MIN = Intrepid2::Minimizer<ValueT, DIM>;
  using FN = LCM::Banana<ValueT>;
  using STEP = Intrepid2::NewtonStep<FN, ValueT, DIM>;

  MIN
  minimizer;

  STEP
  step;

  FN
  banana;

  Intrepid2::Vector<ScalarT, DIM>
  x;

  x(0) = 0.0;
  x(1) = 3.0;

  LCM::MiniSolver<MIN, STEP, FN, EvalT, DIM>
  mini_solver(minimizer, step, banana, x);

  minimizer.printReport(os);

  ASSERT_EQ(minimizer.converged, true);
}
