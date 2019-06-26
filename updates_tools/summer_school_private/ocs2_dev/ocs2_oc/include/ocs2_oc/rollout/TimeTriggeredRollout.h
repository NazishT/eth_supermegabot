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

#ifndef TIMETRIGGERED_ROLLOUT_OCS2_H_
#define TIMETRIGGERED_ROLLOUT_OCS2_H_

#include <memory>

#include <ocs2_core/dynamics/ControlledSystemBase.h>
#include <ocs2_core/integration/Integrator.h>
#include <ocs2_core/integration/SystemEventHandler.h>
#include <ocs2_core/integration/StateTriggeredEventHandler.h>

#include "RolloutBase.h"

namespace ocs2 {

/**
 * This class is an interface class for forward rollout of the system dynamics.
 *
 * @tparam STATE_DIM: Dimension of the state space.
 * @tparam INPUT_DIM: Dimension of the control input space.
 * @tparam LOGIC_RULES_T: Logic Rules type (default NullLogicRules).
 */
template <size_t STATE_DIM, size_t INPUT_DIM, class LOGIC_RULES_T=NullLogicRules>
class TimeTriggeredRollout : public RolloutBase<STATE_DIM, INPUT_DIM, LOGIC_RULES_T>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	static_assert(std::is_base_of<LogicRulesBase, LOGIC_RULES_T>::value,
			"LOGIC_RULES_T must inherit from LogicRulesBase");

	typedef RolloutBase<STATE_DIM, INPUT_DIM, LOGIC_RULES_T> BASE;

	typedef typename BASE::controller_t         controller_t;
	typedef typename BASE::size_array_t         size_array_t;
	typedef typename BASE::scalar_t             scalar_t;
	typedef typename BASE::scalar_array_t       scalar_array_t;
	typedef typename BASE::state_vector_t       state_vector_t;
	typedef typename BASE::state_vector_array_t state_vector_array_t;
	typedef typename BASE::input_vector_t       input_vector_t;
	typedef typename BASE::input_vector_array_t input_vector_array_t;

	typedef SystemEventHandler<STATE_DIM> event_handler_t;
	typedef ControlledSystemBase<STATE_DIM, INPUT_DIM, LOGIC_RULES_T> controlled_system_base_t;

	typedef LogicRulesMachine<LOGIC_RULES_T> logic_rules_machine_t;

	typedef IntegratorBase<STATE_DIM> ode_base_t;

	/**
	 * Constructor.
	 *
	 * @param [in] systemDynamics: The system dynamics for forward rollout.
	 * @param [in] rolloutSettings: The rollout settings.
	 * @param [in] algorithmName: The algorithm that calls this class (default not defined).
	 */
	TimeTriggeredRollout(
			const controlled_system_base_t& systemDynamics,
			const Rollout_Settings& rolloutSettings = Rollout_Settings(),
			const char* algorithmName = NULL)

	: BASE(rolloutSettings, algorithmName)
	, systemDynamicsPtr_(systemDynamics.clone())
	, systemEventHandlersPtr_(new event_handler_t)
	, dynamicsIntegratorsPtr_(new ODE45<STATE_DIM>(systemDynamicsPtr_, systemEventHandlersPtr_))
	{}

	/**
	 * Default destructor.
	 */
	~TimeTriggeredRollout() = default;

	/**
	 * Forward integrate the system dynamics with given controller. It uses the given control policies and initial state,
	 * to integrate the system dynamics in time period [initTime, finalTime].
	 *
	 * @param [in] partitionIndex: Time partition index.
	 * @param [in] initTime: The initial time.
	 * @param [in] initState: The initial state.
	 * @param [in] finalTime: The final time.
	 * @param [in] controller: control policy.
	 * @param [in] logicRulesMachine: logic rules machine.
	 * @param [out] timeTrajectory: The time trajectory stamp.
	 * @param [out] eventsPastTheEndIndeces: Indices containing past-the-end index of events trigger.
	 * @param [out] stateTrajectory: The state trajectory.
	 * @param [out] inputTrajectory: The control input trajectory.
	 *
	 * @return The final state (state jump is considered if it took place)
	 */
	state_vector_t run(
			const size_t& partitionIndex,
			const scalar_t& initTime,
			const state_vector_t& initState,
			const scalar_t& finalTime,
			const controller_t& controller,
			logic_rules_machine_t& logicRulesMachine,
			scalar_array_t& timeTrajectory,
			size_array_t& eventsPastTheEndIndeces,
			state_vector_array_t& stateTrajectory,
			input_vector_array_t& inputTrajectory) override {

		if (initTime > finalTime)
			throw std::runtime_error("Initial time should be less-equal to final time.");

		if (controller.empty() == true)
			throw std::runtime_error("The input controller is empty.");

		const size_t numEvents = logicRulesMachine.getNumEvents(partitionIndex);
		const size_t numSubsystems = logicRulesMachine.getNumEventCounters(partitionIndex);
		const scalar_array_t& switchingTimes = logicRulesMachine.getSwitchingTimes(partitionIndex);

		// max number of steps for integration
		const size_t maxNumSteps =
				BASE::settings().maxNumStepsPerSecond_ * std::max(1.0, finalTime-initTime);

		// index of the first subsystem
		size_t beginItr = findActiveIntervalIndex(switchingTimes, initTime, 0);
		// index of the last subsystem
		size_t finalItr = findActiveIntervalIndex(switchingTimes, finalTime, numSubsystems-1);

		// clearing the output trajectories
		timeTrajectory.clear();
		timeTrajectory.reserve(maxNumSteps+1);
		stateTrajectory.clear();
		stateTrajectory.reserve(maxNumSteps+1);
		inputTrajectory.clear();
		inputTrajectory.reserve(maxNumSteps+1);
		eventsPastTheEndIndeces.clear();
		eventsPastTheEndIndeces.reserve(numEvents);

		// set controller
		systemDynamicsPtr_->setController(controller);

		// reset function calls counter
		systemDynamicsPtr_->resetNumFunctionCalls();

		// Reset the event class
		systemEventHandlersPtr_->reset();

		// initialize subsystem
		systemDynamicsPtr_->initializeModel(
				logicRulesMachine,
				partitionIndex,
				BASE::algorithmName());

		state_vector_t beginState = initState;
		scalar_t beginTime, endTime;
		size_t k_u = 0;  // control input iterator
		for (size_t i=beginItr; i<=finalItr; i++) {

			beginTime = i==beginItr ? initTime  : switchingTimes[i];
			endTime   = i==finalItr ? finalTime : switchingTimes[i+1];

			// in order to correctly detect the next subsystem (right limit)
			beginTime += 10*OCS2NumericTraits<scalar_t>::week_epsilon();

			// integrate controlled system
			dynamicsIntegratorsPtr_->integrate(
					beginState, beginTime, endTime,
					stateTrajectory,
					timeTrajectory,
					BASE::settings().minTimeStep_,
					BASE::settings().absTolODE_,
					BASE::settings().relTolODE_,
					maxNumSteps,
					true);

			// compute control input trajectory and concatenate to inputTrajectory
			for ( ; k_u<timeTrajectory.size(); k_u++) {
				inputTrajectory.emplace_back( systemDynamicsPtr_->computeInput(
						timeTrajectory[k_u], stateTrajectory[k_u]) );
			} // end of k loop

			if (i<finalItr) {
				eventsPastTheEndIndeces.push_back( stateTrajectory.size() );
				systemDynamicsPtr_->computeJumpMap(
						timeTrajectory.back(), stateTrajectory.back(), beginState);
			}

		}  // end of i loop

		// If an event has happened at the final time push it to the eventsPastTheEndIndeces
		// numEvents>finalItr means that there the final active subsystem is before an event time.
		// Note: we don't push the state because the input is not yet defined since the next control
		// policy is available)
		bool eventAtFinalTime = numEvents>finalItr &&
				logicRulesMachine.getEventTimes(partitionIndex)[finalItr]<finalTime+OCS2NumericTraits<scalar_t>::limit_epsilon();

		if (eventAtFinalTime) {
			eventsPastTheEndIndeces.push_back( stateTrajectory.size() );
			systemDynamicsPtr_->computeJumpMap(
					timeTrajectory.back(), stateTrajectory.back(), beginState);
			return beginState;

		} else {
			return stateTrajectory.back();
		}
	}

private:
	std::shared_ptr<controlled_system_base_t> systemDynamicsPtr_;

	std::shared_ptr<event_handler_t> systemEventHandlersPtr_;

	std::unique_ptr<ode_base_t> dynamicsIntegratorsPtr_;

};

} // namespace ocs2

#endif /* TIMETRIGGERED_ROLLOUT_OCS2_H_ */
