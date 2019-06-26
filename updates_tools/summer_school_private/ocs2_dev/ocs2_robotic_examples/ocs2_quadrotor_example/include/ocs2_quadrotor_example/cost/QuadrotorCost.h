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

#ifndef QUADROTOR_COST_OCS2_H_
#define QUADROTOR_COST_OCS2_H_

#include <ocs2_core/cost/QuadraticCostFunction.h>
#include <ocs2_core/logic/rules/NullLogicRules.h>

#include "ocs2_quadrotor_example/QuadrotorParameters.h"
#include "ocs2_quadrotor_example/definitions.h"

namespace ocs2 {
namespace quadrotor {

class QuadrotorCost final : public QuadraticCostFunction<quadrotor::STATE_DIM_, quadrotor::INPUT_DIM_>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef std::shared_ptr<QuadrotorCost> Ptr;
	typedef std::shared_ptr<const QuadrotorCost> ConstPtr;

	typedef QuadraticCostFunction<quadrotor::STATE_DIM_, quadrotor::INPUT_DIM_> BASE;
	typedef typename BASE::scalar_t scalar_t;
	typedef typename BASE::state_vector_t state_vector_t;
	typedef typename BASE::state_matrix_t state_matrix_t;
	typedef typename BASE::input_vector_t input_vector_t;
	typedef typename BASE::input_matrix_t input_matrix_t;

	/**
	 * Constructor for the running and final cost function defined as the following:
	 * - \f$ L = 0.5(x-x_{nominal})' Q (x-x_{nominal}) + 0.5(u-u_{nominal})' R (u-u_{nominal}) \f$
	 * - \f$ \Phi = 0.5(x-x_{final})' Q_{final} (x-x_{final}) \f$.
	 * @param [in] Q: \f$ Q \f$
	 * @param [in] R: \f$ R \f$
	 * @param [in] xNominal: \f$ x_{nominal}\f$
	 * @param [in] uNominal: \f$ u_{nominal}\f$
	 * @param [in] QFinal: \f$ Q_{final}\f$
	 * @param [in] xFinal: \f$ x_{final}\f$
	 */
	QuadrotorCost(
			const state_matrix_t& Q,
			const input_matrix_t& R,
			const state_vector_t& x_nominal,
			const input_vector_t& u_nominal,
			const state_matrix_t& Q_final,
			const state_vector_t& x_final)
	: QuadraticCostFunction(Q, R, x_nominal, u_nominal, Q_final, x_final)
	{}

	/**
	 * Destructor
	 */
	~QuadrotorCost() = default;

    /**
     * Returns pointer to the class.
     *
     * @return A raw pointer to the class.
     */
	QuadrotorCost* clone() const {

		return new QuadrotorCost(*this);
	}

	/**
	 * Sets the current time, state, and control input.
	 *
	 * @param [in] t: Current time.
	 * @param [in] x: Current state vector.
	 * @param [in] u: Current input vector.
	 */
	void setCurrentStateAndControl(
			const scalar_t& t,
			const state_vector_t& x,
			const input_vector_t& u) {

		dynamic_vector_t xNominal;
		BASE::xNominalFunc_.interpolate(t, xNominal);
		dynamic_vector_t uNominal;
		BASE::uNominalFunc_.interpolate(t, uNominal);

		// set base class
		BASE::setCurrentStateAndControl(t, x, u, xNominal, uNominal, xNominal);
	}

private:

};

} // namespace quadrotor
} // namespace ocs2

#endif //QUADROTOR_COST_OCS2_H_
