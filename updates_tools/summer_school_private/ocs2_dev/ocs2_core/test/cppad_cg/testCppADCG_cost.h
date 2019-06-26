/******************************************************************************
Copyright (c) 2017, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#ifndef TESTCPPADCG_COST_OCS2_H_
#define TESTCPPADCG_COST_OCS2_H_

#include <iostream>
#include <mutex>
#include <functional>
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include "ocs2_core/cost/QuadraticCostFunction.h"
#include "QuadraticCostFunctionAD.h"

namespace testCppADCG_cost
{

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
template <size_t STATE_DIM, size_t INPUT_DIM>
void checkCostFunction(
		const size_t numTests,
		ocs2::CostFunctionBase<STATE_DIM, INPUT_DIM> *const quadraticCost1,
		ocs2::CostFunctionBase<STATE_DIM, INPUT_DIM> *const quadraticCost2,
		bool& success) {

	typedef ocs2::QuadraticCostFunction<STATE_DIM, INPUT_DIM> quadratic_cost_t;

	typedef typename quadratic_cost_t::scalar_t              scalar_t;
	typedef typename quadratic_cost_t::state_vector_t        state_vector_t;
	typedef typename quadratic_cost_t::state_matrix_t        state_matrix_t;
	typedef typename quadratic_cost_t::input_vector_t        input_vector_t;
	typedef typename quadratic_cost_t::input_matrix_t        input_matrix_t;
	typedef typename quadratic_cost_t::input_state_matrix_t  input_state_matrix_t;

	success = true;

	const scalar_t precision = 1e-9;

	state_vector_t x;
	input_vector_t u;

	for (size_t it=0; it<numTests; it++) {

		x.setRandom();
		u.setRandom();

		quadraticCost1->setCurrentStateAndControl(0.0, x, u);
		quadraticCost2->setCurrentStateAndControl(0.0, x, u);

		scalar_t L, ad_L;
		quadraticCost1->getIntermediateCost(L);
		quadraticCost2->getIntermediateCost(ad_L);
		if (std::abs(L-ad_L) > precision) {
			std::cout << "L:    " << L << std::endl;
			std::cout << "al_L: " << ad_L << std::endl;
			success = false;
		}

		state_vector_t dLdx, ad_dLdx;
		quadraticCost1->getIntermediateCostDerivativeState(dLdx);
		quadraticCost2->getIntermediateCostDerivativeState(ad_dLdx);
		if (!dLdx.isApprox(ad_dLdx, precision)) {
			std::cout << "dLdx:    " << dLdx.transpose() << std::endl;
			std::cout << "al_dLdx: " << ad_dLdx.transpose() << std::endl;
			success = false;
		}

		state_matrix_t dLdxx, ad_dLdxx;
		quadraticCost1->getIntermediateCostSecondDerivativeState(dLdxx);
		quadraticCost2->getIntermediateCostSecondDerivativeState(ad_dLdxx);
		if (!dLdxx.isApprox(ad_dLdxx, precision)) {
			std::cout << "dLdxx:    \n" << dLdxx << std::endl;
			std::cout << "al_dLdxx: \n" << ad_dLdxx << std::endl;
			success = false;
		}

		input_vector_t dLdu, ad_dLdu;
		quadraticCost1->getIntermediateCostDerivativeInput(dLdu);
		quadraticCost2->getIntermediateCostDerivativeInput(ad_dLdu);
		if (!dLdu.isApprox(ad_dLdu, precision)) {
			std::cout << "dLdu:    " << dLdu.transpose() << std::endl;
			std::cout << "al_dLdu: " << ad_dLdu.transpose() << std::endl;
			success = false;
		}

		input_matrix_t dLduu, ad_dLduu;
		quadraticCost1->getIntermediateCostSecondDerivativeInput(dLduu);
		quadraticCost2->getIntermediateCostSecondDerivativeInput(ad_dLduu);
		if (!dLduu.isApprox(ad_dLduu, precision)) {
			std::cout << "dLduu:    \n" << dLduu << std::endl;
			std::cout << "al_dLduu: \n" << ad_dLduu << std::endl;
			success = false;
		}

		input_state_matrix_t dLdxu, ad_dLdxu;
		quadraticCost1->getIntermediateCostDerivativeInputState(dLdxu);
		quadraticCost2->getIntermediateCostDerivativeInputState(ad_dLdxu);
		if (!dLdxu.isApprox(ad_dLdxu, precision)) {
			std::cout << "dLdxu:    \n" << dLdxu << std::endl;
			std::cout << "al_dLdxu: \n" << ad_dLdxu << std::endl;
			success = false;
		}

		scalar_t Phi, ad_Phi;
		quadraticCost1->getTerminalCost(Phi);
		quadraticCost2->getTerminalCost(ad_Phi);
		if (std::abs(Phi-ad_Phi) > precision) {
			std::cout << "Phi:    " << Phi << std::endl;
			std::cout << "al_Phi: " << ad_Phi << std::endl;
			success = false;
		}

		state_vector_t dPhidx, ad_dPhidx;
		quadraticCost1->getTerminalCostDerivativeState(dPhidx);
		quadraticCost2->getTerminalCostDerivativeState(ad_dPhidx);
		if (!dPhidx.isApprox(ad_dPhidx, precision)) {
			std::cout << "dPhidx:    " << dPhidx.transpose() << std::endl;
			std::cout << "al_dPhidx: " << ad_dPhidx.transpose() << std::endl;
			success = false;
		}

		state_matrix_t dPhidxx, ad_dPhidxx;
		quadraticCost1->getTerminalCostSecondDerivativeState(dPhidxx);
		quadraticCost2->getTerminalCostSecondDerivativeState(ad_dPhidxx);
		if (!dPhidxx.isApprox(ad_dPhidxx, precision)) {
			std::cout << "dPhidxx:    \n" << dPhidxx << std::endl;
			std::cout << "al_dPhidxx: \n" << ad_dPhidxx << std::endl;
			success = false;
		}
	} // end of for loop

}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
struct cost_parameters
{
	enum
	{
		state_dim_ = 4,
		input_dim_ = 2
	};

	typedef ocs2::CostFunctionBase<state_dim_, input_dim_> base_cost_t;

	typedef base_cost_t::scalar_t              scalar_t;
	typedef base_cost_t::state_vector_t        state_vector_t;
	typedef base_cost_t::state_matrix_t        state_matrix_t;
	typedef base_cost_t::input_vector_t        input_vector_t;
	typedef base_cost_t::input_matrix_t        input_matrix_t;
	typedef base_cost_t::input_state_matrix_t  input_state_matrix_t;

	cost_parameters()
	: Q(5.0 * state_matrix_t::Random())
	, R(3.0*input_matrix_t::Random())
	, P(2.0*input_state_matrix_t::Random())
	, xNominal(state_vector_t::Random())
	, uNominal(input_vector_t::Random())
	, QFinal(4.0 * state_matrix_t::Random())
	{
		Q = (Q+Q.transpose()).eval();
		R = (R+R.transpose()).eval();
		QFinal = (QFinal+QFinal.transpose()).eval();
	}

    // Define cost parameters
	state_matrix_t Q;
	input_matrix_t R;
	input_state_matrix_t  P;
	state_vector_t xNominal;
	input_vector_t uNominal;
	state_matrix_t QFinal;
};

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
TEST(testCppADCG_cost, quadratic_cost_test)
{
	enum
	{
		state_dim_ = cost_parameters::state_dim_,
		input_dim_ = cost_parameters::input_dim_
	};

	typedef ocs2::CostFunctionBase<state_dim_, input_dim_>      base_cost_t;
	typedef ocs2::QuadraticCostFunction<state_dim_, input_dim_> quadratic_cost_t;
	typedef ocs2::QuadraticCostFunctionAD<state_dim_, input_dim_, ocs2::NullLogicRules> ad_quadratic_cost_t;

	cost_parameters costParam;

    /***************************************************************************
     *               path to save the generated dynamic-library
     **************************************************************************/
    boost::filesystem::path filePath(__FILE__);
    std::string libraryFolder = filePath.parent_path().generic_string() + "/testCppADCG_generated";

	quadratic_cost_t quadraticCost(
			costParam.Q, costParam.R, costParam.xNominal, costParam.uNominal,
			costParam.QFinal, costParam.xNominal, costParam.P);

	ad_quadratic_cost_t ad_quadraticCost(
			costParam.Q, costParam.R, costParam.xNominal, costParam.uNominal,
			costParam.QFinal, costParam.P);
	ad_quadraticCost.createModels("testCppADCG_cost", libraryFolder);

	bool success;
	checkCostFunction(100, &quadraticCost, &ad_quadraticCost, success);

	ASSERT_TRUE(success);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
TEST(testCppADCG_cost, clone_test)
{
	enum
	{
		state_dim_ = cost_parameters::state_dim_,
		input_dim_ = cost_parameters::input_dim_
	};

	typedef ocs2::CostFunctionBase<state_dim_, input_dim_>      base_cost_t;
	typedef ocs2::QuadraticCostFunction<state_dim_, input_dim_> quadratic_cost_t;
	typedef ocs2::QuadraticCostFunctionAD<state_dim_, input_dim_, ocs2::NullLogicRules> ad_quadratic_cost_t;

	cost_parameters costParam;

    /***************************************************************************
     *               path to save the generated dynamic-library
     **************************************************************************/
    boost::filesystem::path filePath(__FILE__);
    std::string libraryFolder = filePath.parent_path().generic_string() + "/testCppADCG_generated";

	quadratic_cost_t quadraticCost(
			costParam.Q, costParam.R, costParam.xNominal, costParam.uNominal,
			costParam.QFinal, costParam.xNominal, costParam.P);

	ad_quadratic_cost_t ad_quadraticCost(
			costParam.Q, costParam.R, costParam.xNominal, costParam.uNominal,
			costParam.QFinal, costParam.P);
	ad_quadraticCost.createModels("testCppADCG_cost", libraryFolder);

	base_cost_t::Ptr ad_quadraticCostPtr(ad_quadraticCost.clone());

	bool success;
	checkCostFunction(100, &quadraticCost, ad_quadraticCostPtr.get(), success);

	ASSERT_TRUE(success);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
TEST(testCppADCG_cost, multithread_test)
{
	enum
	{
		state_dim_ = cost_parameters::state_dim_,
		input_dim_ = cost_parameters::input_dim_
	};

	typedef ocs2::CostFunctionBase<state_dim_, input_dim_>      base_cost_t;
	typedef ocs2::QuadraticCostFunction<state_dim_, input_dim_> quadratic_cost_t;
	typedef ocs2::QuadraticCostFunctionAD<state_dim_, input_dim_, ocs2::NullLogicRules> ad_quadratic_cost_t;

	cost_parameters costParam;

    /***************************************************************************
     *               path to save the generated dynamic-library
     **************************************************************************/
    boost::filesystem::path filePath(__FILE__);
    std::string libraryFolder = filePath.parent_path().generic_string() + "/testCppADCG_generated";

	quadratic_cost_t quadraticCost(
			costParam.Q, costParam.R, costParam.xNominal, costParam.uNominal,
			costParam.QFinal, costParam.xNominal, costParam.P);

	base_cost_t::Ptr quadraticCostPtr(quadraticCost.clone());

	ad_quadratic_cost_t ad_quadraticCost(
			costParam.Q, costParam.R, costParam.xNominal, costParam.uNominal,
			costParam.QFinal, costParam.P);

	ad_quadraticCost.createModels("testCppADCG_cost", libraryFolder);

	base_cost_t::Ptr ad_quadraticCostPtr(ad_quadraticCost.clone());


	/***************************************************************************
	 *               Multi-thread test
	 **************************************************************************/
	bool success = false;
    std::thread thread1(checkCostFunction<state_dim_, input_dim_>, 10000, &quadraticCost, &ad_quadraticCost, std::ref(success));

    bool successClone = false;
    std::thread thread2(checkCostFunction<state_dim_, input_dim_>, 10000, quadraticCostPtr.get(), ad_quadraticCostPtr.get(), std::ref(successClone));

    if (thread1.joinable())
    	thread1.join();
    if (thread2.joinable())
    	thread2.join();

    ASSERT_TRUE(success && successClone);
}

}  // end of namespace testCppADCG_cost

#endif /* TESTCPPADCG_COST_OCS2_H_ */
