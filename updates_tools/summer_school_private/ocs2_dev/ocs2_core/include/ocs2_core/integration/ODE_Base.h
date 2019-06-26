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

#ifndef ODE_BASE_OCS2_H_
#define ODE_BASE_OCS2_H_

#include <Eigen/Dense>

#include "ocs2_core/Dimensions.h"

namespace ocs2{

/**
 * The base class for autonomous system dynamics.
 *
 * @tparam STATE_DIM: Dimension of the state space.
 */
template <int STATE_DIM>
class ODE_Base
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef Dimensions<STATE_DIM, 0> DIMENSIONS;
	typedef typename DIMENSIONS::scalar_t         scalar_t;
	typedef typename DIMENSIONS::state_vector_t	  state_vector_t;
	typedef typename DIMENSIONS::dynamic_vector_t dynamic_vector_t;

    /**
     * Default constructor
     */
	ODE_Base()
	: numFunctionCalls_(0)
	{}

	/**
	 * Default destructor
	 */
	virtual ~ODE_Base() = default;

    /**
     * Default copy constructor
     */
	ODE_Base(const ODE_Base& rhs)
	: ODE_Base()
	{}

	/**
	 * Gets the number of function calls.
	 *
	 * @return size_t: number of function calls
	 */
	size_t getNumFunctionCalls() {
		return numFunctionCalls_;
	}

	/**
	 * Resets the number of function calls to zero.
	 *
	 */
	void resetNumFunctionCalls() {
		numFunctionCalls_ = 0;
	}

	/**
	 * Computes the autonomous system dynamics.
	 * @param [in] t: Current time.
	 * @param [in] x: Current state.
	 * @param [out] dxdt: Current state time derivative
	 */
	virtual void computeFlowMap(
			const scalar_t& t,
			const state_vector_t& x,
			state_vector_t& dxdt) = 0;

	/**
	 * State map at the transition time
	 *
	 * @param [in] time: transition time
	 * @param [in] state: transition state
	 * @param [out] mappedState: mapped state after transition
	 */
	virtual void computeJumpMap(
			const scalar_t& time,
			const state_vector_t& state,
			state_vector_t& mappedState) {

		mappedState = state;
	}

	/**
	 * Interface method to the guard surfaces.
	 *
	 * @param [in] time: transition time
	 * @param [in] state: transition state
	 * @param [out] guardSurfacesValue: An array of guard surfaces values
	 */
	virtual void computeGuardSurfaces(
			const scalar_t& time,
			const state_vector_t& state,
			dynamic_vector_t& guardSurfacesValue) {

		guardSurfacesValue = -dynamic_vector_t::Ones(1);
	}

protected:
	size_t numFunctionCalls_;

};

} // namespace ocs2

#endif /* ODE_BASE_OCS2_H_ */
