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

#ifndef CONSTRAINTBASE_OCS2_H_
#define CONSTRAINTBASE_OCS2_H_

#include <memory>
#include <cstring>

#include "ocs2_core/Dimensions.h"
#include "ocs2_core/logic/rules/LogicRulesBase.h"
#include "ocs2_core/logic/rules/NullLogicRules.h"
#include "ocs2_core/logic/machine/LogicRulesMachine.h"

namespace ocs2 {

/**
 * Base class for the constraints and its Derivatives. The linearized constraints are defined as: \n
 * Here we consider two types of constraints: state-input constraints, \f$ g_1(x,u,T)\f$ and state-only
 * constraints, \f$ g_2(x,T)\f$. \f$ x \f$, \f$ u \f$, and \f$ T \f$ are state, input, and vector of
 * event times respectively.
 *
 * - Linearized state-input constraints:       \f$ C(t) \delta x + D(t) \delta u + e(t) = 0 \f$ \n
 * - Linearized only-state constraints:        \f$ F(t) \delta x + h(t) = 0 \f$ \n
 * - Linearized only-state final constraints:  \f$ F_f(t) \delta x + h_f(t) = 0 \f$
 *
 * @tparam STATE_DIM: Dimension of the state space.
 * @tparam INPUT_DIM: Dimension of the control input space.
 * @tparam LOGIC_RULES_T: Logic Rules type (default NullLogicRules).
 */
template <size_t STATE_DIM, size_t INPUT_DIM, class LOGIC_RULES_T=NullLogicRules>
class ConstraintBase
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	static_assert(std::is_base_of<LogicRulesBase, LOGIC_RULES_T>::value,
			"LOGIC_RULES_T must inherit from LogicRulesBase");

	typedef std::shared_ptr<ConstraintBase<STATE_DIM, INPUT_DIM, LOGIC_RULES_T> > Ptr;
	typedef std::shared_ptr<const ConstraintBase<STATE_DIM, INPUT_DIM, LOGIC_RULES_T> > ConstPtr;

	typedef Dimensions<STATE_DIM, INPUT_DIM> DIMENSIONS;
	typedef typename DIMENSIONS::scalar_t                    scalar_t;
	typedef typename DIMENSIONS::state_vector_t              state_vector_t;
	typedef typename DIMENSIONS::input_vector_t              input_vector_t;
	typedef typename DIMENSIONS::state_matrix_t              state_matrix_t;
	typedef typename DIMENSIONS::state_input_matrix_t        state_input_matrix_t;
	typedef typename DIMENSIONS::constraint1_vector_t        constraint1_vector_t;
	typedef typename DIMENSIONS::constraint1_vector_array_t  constraint1_vector_array_t;
	typedef typename DIMENSIONS::constraint2_vector_t        constraint2_vector_t;
	typedef typename DIMENSIONS::constraint2_vector_array_t  constraint2_vector_array_t;
	typedef typename DIMENSIONS::constraint1_state_matrix_t  constraint1_state_matrix_t;
	typedef typename DIMENSIONS::constraint1_input_matrix_t  constraint1_input_matrix_t;
	typedef typename DIMENSIONS::constraint2_state_matrix_t  constraint2_state_matrix_t;

	/**
	 * Default constructor
	 */
	ConstraintBase() = default;

	/**
	 * Default copy constructor
	 */
	ConstraintBase(const ConstraintBase& rhs) = default;

	/**
	 * Default destructor
	 */
	virtual ~ConstraintBase() = default;

	/**
	 * Sets the current time, state, and control input.
	 *
	 * @param [in] t: Current time.
	 * @param [in] x: Current state.
	 * @param [in] u: Current input.
	 */
	virtual void setCurrentStateAndControl(
			const scalar_t& t,
			const state_vector_t& x,
			const input_vector_t& u) {

		t_ = t;
		x_ = x;
		u_ = u;
	}

	/**
	 * Initializes the system constraints.
	 *
	 * @param [in] logicRulesMachine: A class which contains and parse the logic rules e.g
	 * method findActiveSubsystemHandle returns a Lambda expression which can be used to
	 * find the ID of the current active subsystem.
	 * @param [in] partitionIndex: index of the time partition.
	 * @param [in] algorithmName: The algorithm that class this class (default not defined).
	 */
	virtual void initializeModel(
			LogicRulesMachine<LOGIC_RULES_T>& logicRulesMachine,
			const size_t& partitionIndex,
			const char* algorithmName = nullptr)
	{}

	/**
	 * Clones the class.
	 *
	 * @return A raw pointer to the class.
	 */
	virtual ConstraintBase<STATE_DIM, INPUT_DIM, LOGIC_RULES_T>* clone() const {

		return new ConstraintBase<STATE_DIM, INPUT_DIM, LOGIC_RULES_T>(*this);
	}

	/**
	 * Computes the state-input equality constraints.
	 *
	 * @param [out] e: The state-input equality constraints value.
	 */
	virtual void getConstraint1(constraint1_vector_t& e)  {}

	/**
	 * Gets the number of active state-input equality constraints.
	 *
	 * @param [in] time: time.
	 * @return number of state-input active equality constraints.
	 */
	virtual size_t numStateInputConstraint(const scalar_t& time) {

		return 0;
	}

	/**
	 * Gets the state-only equality constraints.
	 *
	 * @param [out] h: The state-only (in)equality constraints value.
	 */
	virtual void getConstraint2(constraint2_vector_t& h) {}

	/**
	 * Get the number of state-only active equality constraints.
	 *
	 * @param [in] time: time.
	 * @return number of state-only active (in)equality constraints.
	 */
	virtual size_t numStateOnlyConstraint(const scalar_t& time) {

		return 0;
	}

	/**
	 * Compute the final state-only equality constraints.
	 *
	 * @param [out] h_f: The final state-only (in)equality constraints value.
	 */
	virtual void getFinalConstraint2(constraint2_vector_t& h_f) {}

	/**
	 * Get the number of final state-only active (in)equality constraints.
	 *
	 * @param [in] time: time.
	 * @return number of final state-only active equality constraints.
	 */
	virtual size_t numStateOnlyFinalConstraint(const scalar_t& time) {

		return 0;
	}

	/**
	 * The C matrix at a given operating point for the linearized state-input constraints,
	 * \f$ C(t) \delta x + D(t) \delta u + e(t) = 0 \f$.
	 *
	 * @param [out] C: \f$ C(t) \f$ matrix.
	 */
	virtual void getConstraint1DerivativesState(constraint1_state_matrix_t& C) {}

	/**
	 * The D matrix at a given operating point for the linearized state-input constraints,
	 * \f$ C(t) \delta x + D(t) \delta u + e(t) = 0 \f$.
	 *
	 * @param [out] D: \f$ D(t) \f$ matrix.
	 */
	virtual void getConstraint1DerivativesControl(constraint1_input_matrix_t& D) {}

	/**
	 * calculate and retrieve the the derivative of the state-input constraints w.r.t. event times.
	 * g1DevArray[i] is a vector of dimension MAX_CONSTRAINT1_DIM_ which is the partial derivative of
	 * state-input equality constraints with respect to i'th event time.
	 *
	 * Note that only nc1 top rows of g1DevArray[i] are valid where nc1 is the number of active
	 * state-input constraints at the current time.
	 *
	 * If the constraints are not a function of event times either set the array size to zero (as the default)
	 * implementation or set it to an array of zero vector with a size equal to number event times.
	 *
	 * @param [out] g1DevArray: an array of nc1-by-1 vector.
	 */
	virtual void getConstraint1DerivativesEventTimes(constraint1_vector_array_t& g1DevArray) { g1DevArray.clear(); }

	/**
	 * The F matrix at a given operating point for the linearized state-only constraints,
	 * \f$ F(t) \delta x + h(t) = 0 \f$.
	 *
	 * @param [out] F: \f$ F(t) \f$ matrix.
	 */
	virtual void getConstraint2DerivativesState(constraint2_state_matrix_t& F) {}

	/**
	 * The F matrix at a given operating point for the linearized terminal state-only constraints,
	 * \f$ F_f(t) \delta x + h_f(t) = 0 \f$.
	 *
	 * @param [out] F_f: \f$ F_f(t) \f$ matrix.
	 */
	virtual void getFinalConstraint2DerivativesState(constraint2_state_matrix_t& F_f) {}

protected:
	scalar_t        t_;
	state_vector_t  x_;
	input_vector_t  u_;
};

} // end of namespace ocs2

#endif /* CONSTRAINTBASE_OCS2_H_ */
