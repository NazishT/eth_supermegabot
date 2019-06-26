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

#ifndef BALLBOTINTERFACE_OCS2_BALLBOT_OCS2_H_
#define BALLBOTINTERFACE_OCS2_BALLBOT_OCS2_H_

// C++
#include <string>
#include <iostream>
#include <stdlib.h>

// OCS2
#include <ocs2_core/Dimensions.h>
#include <ocs2_core/misc/loadEigenMatrix.h>
#include <ocs2_core/constraint/ConstraintBase.h>
#include <ocs2_core/initialization/SystemOperatingPoint.h>
#include <ocs2_mpc/MPC_SLQ.h>
#include <ocs2_robotic_tools/common/RobotInterfaceBase.h>

// Ballbot
#include "ocs2_ballbot_example/definitions.h"
#include "ocs2_ballbot_example/dynamics/BallbotSystemDynamics.h"
#include "ocs2_ballbot_example/cost/BallbotCost.h"

namespace ocs2 {
namespace ballbot {

/**
 * BallbotInterface class
 * General interface for mpc implementation on the ballbot model
 */
class BallbotInterface : public RobotInterfaceBase<ballbot::STATE_DIM_, ballbot::INPUT_DIM_>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef RobotInterfaceBase<ballbot::STATE_DIM_, ballbot::INPUT_DIM_> BASE;

	using dim_t = Dimensions<ballbot::STATE_DIM_, ballbot::INPUT_DIM_>;
	using ballbotConstraint_t = ConstraintBase<ballbot::STATE_DIM_, ballbot::INPUT_DIM_>;
	using ballbotOperatingPoint_t = SystemOperatingPoint<ballbot::STATE_DIM_, ballbot::INPUT_DIM_>;

	typedef ocs2::MPC_SLQ<ballbot::STATE_DIM_, ballbot::INPUT_DIM_> mpc_t;

	/**
	 * Constructor
	 * @param [in] taskFileFolderName: The name of the folder containing task file
	 */
	BallbotInterface(const std::string& taskFileFolderName);

	/**
	 * Destructor
	 */
	~BallbotInterface() = default;

	/**
	 * setup all optimizes.
	 *
	 * @param [in] taskFile: Task's file full path.
	 */
	void setupOptimizer(const std::string& taskFile) final;

	/**
	 * Gets a pointer to the internal SLQ-MPC class.
	 *
	 * @return Pointer to the internal MPC
	 */
	mpc_t::Ptr& getMPCPtr();

protected:
	/**
	 * Load the settings from the path file.
	 *
	 * @param [in] taskFile: Task's file full path.
	 */
	void loadSettings(const std::string& taskFile) final;

	/**************
	 * Variables
	 **************/
	std::string taskFile_;
	std::string libraryFolder_;

	mpc_t::Ptr mpcPtr_;

	BallbotSystemDynamics::Ptr ballbotSystemDynamicsPtr_;
	BallbotCost::Ptr ballbotCostPtr_;
	ballbotConstraint_t::Ptr ballbotConstraintPtr_;
	ballbotOperatingPoint_t::Ptr ballbotOperatingPointPtr_;

	// cost parameters
	dim_t::state_matrix_t Q_;
	dim_t::input_matrix_t R_;
	dim_t::state_matrix_t QFinal_;
	dim_t::state_vector_t xFinal_;
	dim_t::state_vector_t xNominal_;
	dim_t::input_vector_t uNominal_;

	size_t numPartitions_ = 0;
	dim_t::scalar_array_t partitioningTimes_;

	// flag to generate dynamic files
	bool libraryFilesAreGenerated_ = false;
};

} // namespace ballbot
} // namespace ocs2

#endif /* BALLBOTINTERFACE_OCS2_BALLBOT_OCS2_H_ */
