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

#ifndef ROLLOUTSENSITIVITYEQUATIONS_OCS2_H_
#define ROLLOUTSENSITIVITYEQUATIONS_OCS2_H_

#include <array>
#include <Eigen/Dense>

#include <ocs2_core/Dimensions.h>
#include <ocs2_core/dynamics/ControlledSystemBase.h>
#include <ocs2_core/misc/LinearInterpolation.h>

namespace ocs2{

/**
 * Rollout sensitivity equations class
 *
 * @tparam STATE_DIM: Dimension of the state space.
 * @tparam INPUT_DIM: Dimension of the control input space.
 */
template <size_t STATE_DIM, size_t INPUT_DIM>
class RolloutSensitivityEquations : public ControlledSystemBase<STATE_DIM, INPUT_DIM>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef ControlledSystemBase<STATE_DIM, INPUT_DIM> BASE;

	typedef Dimensions<STATE_DIM, INPUT_DIM> DIMENSIONS;
	typedef typename DIMENSIONS::scalar_t       scalar_t;
	typedef typename DIMENSIONS::scalar_array_t scalar_array_t;
	typedef typename DIMENSIONS::state_vector_t       state_vector_t;
	typedef typename DIMENSIONS::state_vector_array_t state_vector_array_t;
	typedef typename DIMENSIONS::input_vector_t       input_vector_t;
	typedef typename DIMENSIONS::input_vector_array_t input_vector_array_t;
	typedef typename DIMENSIONS::state_matrix_t       state_matrix_t;
	typedef typename DIMENSIONS::state_matrix_array_t state_matrix_array_t;
	typedef typename DIMENSIONS::state_input_matrix_t       state_input_matrix_t;
	typedef typename DIMENSIONS::state_input_matrix_array_t state_input_matrix_array_t;
	typedef typename DIMENSIONS::input_state_matrix_t       input_state_matrix_t;
	typedef typename DIMENSIONS::input_state_matrix_array_t input_state_matrix_array_t;

	/**
	 * The default constructor.
	 */
	RolloutSensitivityEquations() = default;

	/**
	 * Default destructor.
	 */
	virtual ~RolloutSensitivityEquations() = default;

	/**
	 * Returns pointer to the class.
	 *
	 * @return A raw pointer to the class.
	 */
	virtual RolloutSensitivityEquations<STATE_DIM, INPUT_DIM>* clone() const {

		return new RolloutSensitivityEquations<STATE_DIM, INPUT_DIM>(*this);
	}

	/**
	 * Sets Data
	 */
	void setData(
			const scalar_array_t* timeTrajectoryPtr,
			const state_matrix_array_t* AmTrajectoryPtr,
			const state_input_matrix_array_t* BmTrajectoryPtr,
			const state_vector_array_t* flowMapTrajectoryPtr,
			const scalar_array_t* sensitivityControllerTimePtr,
			const input_vector_array_t* sensitivityControllerFeedforwardPtr,
			const input_state_matrix_array_t* sensitivityControllerFeedbackPtr)  {

		AmFunc_.setTimeStamp(timeTrajectoryPtr);
		AmFunc_.setData(AmTrajectoryPtr);

		BmFunc_.setTimeStamp(timeTrajectoryPtr);
		BmFunc_.setData(BmTrajectoryPtr);

		flowMapFunc_.setTimeStamp(timeTrajectoryPtr);
		flowMapFunc_.setData(flowMapTrajectoryPtr);

		this->setController(
				*sensitivityControllerTimePtr,
				*sensitivityControllerFeedforwardPtr,
				*sensitivityControllerFeedbackPtr);
	}

	/**
	 * Reset the Riccati equation
	 */
	void reset() {

		AmFunc_.reset();
		BmFunc_.reset();
		flowMapFunc_.reset();
	}

	/**
	 * Sets the multiplier of exogenous part of the equation. It is either zero
	 * or plus-minus 1/(s_{i+1}-s_{i})
	 *
	 * @param [in] multiplier: the multiplier of exogenous part of the equation.
	 */
	void setMultiplier(const scalar_t& multiplier) {

		multiplier_ = multiplier;
	}

	/**
	 * Computes Derivative
	 *
	 * @param [in] t: time
	 * @param [in] nabla_Xv: state sensitivity vector
	 * @param [in] nabla_Uv: input sensitivity vector which is computed by using sensitivity controller.
	 * @param [out] derivatives: time derivative of the state sensitivity vector
	 */
	void computeFlowMap(
			const scalar_t& t,
			const state_vector_t& nabla_x,
			const input_vector_t& nabla_u,
			state_vector_t& derivative) {

		AmFunc_.interpolate(t, Am_);
		size_t greatestLessTimeStampIndex = AmFunc_.getGreatestLessTimeStampIndex();
		BmFunc_.interpolate(t, Bm_, greatestLessTimeStampIndex);

		if (std::abs(multiplier_) > 1e-9) {
			flowMapFunc_.interpolate(t, flowMap_, greatestLessTimeStampIndex);
			derivative = Am_*nabla_x + Bm_*nabla_u + multiplier_*flowMap_;
		} else {
			derivative = Am_*nabla_x + Bm_*nabla_u;
		}
	}


protected:
	scalar_t multiplier_ = 0.0;

	state_matrix_t       Am_;
	state_input_matrix_t Bm_;
	state_vector_t       flowMap_;

	LinearInterpolation<state_matrix_t,Eigen::aligned_allocator<state_matrix_t> >             AmFunc_;
	LinearInterpolation<state_input_matrix_t,Eigen::aligned_allocator<state_input_matrix_t> > BmFunc_;
	LinearInterpolation<state_vector_t,Eigen::aligned_allocator<state_vector_t> >             flowMapFunc_;
};

} // namespace ocs2

#endif /* ROLLOUTSENSITIVITYEQUATIONS_OCS2_H_ */

