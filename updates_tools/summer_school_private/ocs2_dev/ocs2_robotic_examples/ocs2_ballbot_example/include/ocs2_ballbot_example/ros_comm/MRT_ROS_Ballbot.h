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

#ifndef MRT_ROS_BALLBOT_OCS2_H_
#define MRT_ROS_BALLBOT_OCS2_H_

#include <ocs2_comm_interfaces/ocs2_ros_interfaces/mrt/MRT_ROS_Interface.h>
#include "ocs2_ballbot_example/definitions.h"

namespace ocs2 {
namespace ballbot {

/**
 * This class implements MRT (Model Reference Tracking) communication interface using ROS.
 *
 * @tparam STATE_DIM: Dimension of the state space.
 * @tparam ballbot::INPUT_DIM_: Dimension of the control input space.
 */
class MRT_ROS_Ballbot : public MRT_ROS_Interface<ballbot::STATE_DIM_, ballbot::INPUT_DIM_>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef MRT_ROS_Interface<ballbot::STATE_DIM_, ballbot::INPUT_DIM_> BASE;

	typedef Dimensions<ballbot::STATE_DIM_, ballbot::INPUT_DIM_> DIMENSIONS;
	typedef typename DIMENSIONS::controller_t                controller_t;
	typedef typename DIMENSIONS::controller_array_t          controller_array_t;
	typedef typename DIMENSIONS::scalar_t                    scalar_t;
	typedef typename DIMENSIONS::scalar_array_t              scalar_array_t;
	typedef typename DIMENSIONS::size_array_t                size_array_t;
	typedef typename DIMENSIONS::state_vector_t              state_vector_t;
	typedef typename DIMENSIONS::state_vector_array_t        state_vector_array_t;
	typedef typename DIMENSIONS::input_vector_t              input_vector_t;
	typedef typename DIMENSIONS::input_vector_array_t        input_vector_array_t;
	typedef typename DIMENSIONS::input_state_matrix_t        input_state_matrix_t;
	typedef typename DIMENSIONS::input_state_matrix_array_t  input_state_matrix_array_t;

	typedef ocs2::SystemObservation<ballbot::STATE_DIM_, ballbot::INPUT_DIM_> system_observation_t;

	typedef ocs2::RosMsgConversions<ballbot::STATE_DIM_, ballbot::INPUT_DIM_> ros_msg_conversions_t;

	typedef ocs2::LinearInterpolation<state_vector_t, Eigen::aligned_allocator<state_vector_t> > state_linear_interpolation_t;
	typedef ocs2::LinearInterpolation<input_vector_t, Eigen::aligned_allocator<input_vector_t> > input_linear_interpolation_t;
	typedef ocs2::LinearInterpolation<input_state_matrix_t, Eigen::aligned_allocator<input_state_matrix_t>> gain_linear_interpolation_t;

	/**
	 * Default constructor
	 */
	MRT_ROS_Ballbot() = default;

	/**
	 * Constructor
	 *
	 * @param [in] logicRules: A logic rule class of derived from the hybrid logicRules base.
	 * @param [in] useFeedforwardPolicy: Whether to receive the MPC feedforward (true) or MPC feedback policy (false).
	 * @param [in] robotName: The robot's name.
	 */
	MRT_ROS_Ballbot(
			const bool& useFeedforwardPolicy = true,
			const std::string& robotName = "robot")

	: BASE(NullLogicRules(), useFeedforwardPolicy, robotName)
	{}

	/**
	 * Destructor
	 */
	virtual ~MRT_ROS_Ballbot() = default;

	/**
	 * This method will be called either after the very fist call of the class or after a call to reset().
	 * Users can use this function for any sort of initialization that they may need in the first call.
	 *
	 * @param [in] planObservation: The observation of the policy.
	 */
	virtual void initCall(
			const system_observation_t& planObservation) override
	{}

};

} // namespace ballbot
} // namespace ocs2

#endif /* MRT_ROS_BALLBOT_OCS2_H_ */
