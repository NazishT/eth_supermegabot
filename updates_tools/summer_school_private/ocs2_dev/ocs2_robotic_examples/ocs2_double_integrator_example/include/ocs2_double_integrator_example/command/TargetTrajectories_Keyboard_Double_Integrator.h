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

#ifndef TARGETTRAJECTORIES_KEYBOARD_DOUBLE_INTEGRATOR_OCS2_H_
#define TARGETTRAJECTORIES_KEYBOARD_DOUBLE_INTEGRATOR_OCS2_H_

#include <ocs2_robotic_tools/command/TargetTrajectories_Keyboard_Interface.h>
#include <ocs2_robotic_tools/command/TargetPoseTransformation.h>

namespace ocs2 {
namespace double_integrator {

/**
 * This class implements TargetTrajectories communication using ROS.
 *
 * @tparam SCALAR_T: scalar type.
 */
template <typename SCALAR_T>
class TargetTrajectories_Keyboard_Double_Integrator: public ocs2::TargetTrajectories_Keyboard_Interface<SCALAR_T>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	enum
	{
		command_dim_ = 2
	};

	typedef ocs2::TargetTrajectories_Keyboard_Interface<SCALAR_T> BASE;
	typedef typename BASE::scalar_t scalar_t;
	typedef typename BASE::scalar_array_t scalar_array_t;
	typedef typename BASE::dynamic_vector_t dynamic_vector_t;
	typedef typename BASE::dynamic_vector_array_t dynamic_vector_array_t;
	typedef typename BASE::cost_desired_trajectories_t cost_desired_trajectories_t;

	/**
	 * Constructor.
	 *
	 * @param robotName: The robot's name.
	 * @param goalPoseLimit: Limits for the input command. It has size 12 with following entries.
	 *
	 * goalPoseLimit(0): X
	 * goalPoseLimit(1): V_X
	 */
	TargetTrajectories_Keyboard_Double_Integrator(
				const std::string& robotName = "robot",
				const scalar_array_t& goalPoseLimit = scalar_array_t{10.0, 10.0})
	: BASE(robotName, command_dim_, goalPoseLimit)
	{}

	/**
	* Default destructor
	*/
	~TargetTrajectories_Keyboard_Double_Integrator() = default;

	/**
	 * From command line loaded command to desired time, state, and input.
	 *
	 * @param [out] commadLineTarget: The loaded command target.
	 * @param [in] desiredTime: Desired time to be published.
	 * @param [in] desiredState: Desired state to be published.
	 * @param [in] desiredInput: Desired input to be published.
	 */
	void toCostDesiredTimeStateInput(
			const scalar_array_t& commadLineTarget,
			scalar_t& desiredTime,
			dynamic_vector_t& desiredState,
			dynamic_vector_t& desiredInput) final {

		// time
		desiredTime = 0.0;
		// state
		desiredState = Eigen::Map<const dynamic_vector_t>(commadLineTarget.data(), command_dim_);
		// input
		desiredInput = dynamic_vector_t::Zero(0);
	}

private:

};

} // namespace double_integrator
} // namespace ocs2

#endif /* TARGETTRAJECTORIES_KEYBOARD_DOUBLE_INTEGRATOR_OCS2_H_ */

