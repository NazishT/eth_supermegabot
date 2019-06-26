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

#ifndef EXP2_OCS2_H_
#define EXP2_OCS2_H_

#include <ocs2_core/logic/rules/LogicRulesBase.h>
#include <ocs2_core/dynamics/ControlledSystemBase.h>
#include <ocs2_core/dynamics/DerivativesBase.h>
#include <ocs2_core/constraint/ConstraintBase.h>
#include <ocs2_core/cost/CostFunctionBase.h>
#include <ocs2_core/misc/FindActiveIntervalIndex.h>
#include <ocs2_core/initialization/SystemOperatingPoint.h>

namespace ocs2 {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_LogicRules : public LogicRulesBase
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef LogicRulesBase BASE;

	EXP2_LogicRules() = default;

	~EXP2_LogicRules() = default;

	EXP2_LogicRules(const scalar_array_t& eventTimes)
	: BASE(eventTimes)
	{}

	void rewind(const scalar_t& lowerBoundTime,
			const scalar_t& upperBoundTime) override
	{}

	void update() override
	{}

private:

};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_Sys1 : public ControlledSystemBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_Sys1() = default;
	~EXP2_Sys1() = default;

	void computeFlowMap(const scalar_t& t, const state_vector_t& x, const input_vector_t& u,
			state_vector_t& dxdt) final {
		dxdt(0) = x(0) + u(0)*sin(x(0));
		dxdt(1) = -x(1) - u(0)*cos(x(1));
	}

	EXP2_Sys1* clone() const final {
		return new EXP2_Sys1(*this);
	}
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_Sys2 : public ControlledSystemBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_Sys2() = default;
	~EXP2_Sys2() = default;

	void computeFlowMap(const scalar_t& t, const state_vector_t& x, const input_vector_t& u,
			state_vector_t& dxdt) final {
		dxdt(0) = x(1) + u(0)*sin(x(1));
		dxdt(1) = -x(0) - u(0)*cos(x(0));
	}

	EXP2_Sys2* clone() const final {
		return new EXP2_Sys2(*this);
	}
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_Sys3 : public ControlledSystemBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_Sys3() = default;
	~EXP2_Sys3() = default;

	void computeFlowMap(const scalar_t& t, const state_vector_t& x, const input_vector_t& u,
			state_vector_t& dxdt) final {
		dxdt(0) = -x(0) - u(0)*sin(x(0));
		dxdt(1) = x(1) + u(0)*cos(x(1));
	}

	EXP2_Sys3* clone() const final {
		return new EXP2_Sys3(*this);
	}
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_System : public ControlledSystemBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef ControlledSystemBase<2,2,EXP2_LogicRules> Base;

	EXP2_System()
	: activeSubsystem_(0),
	  subsystemDynamicsPtr_(3)
	{
		subsystemDynamicsPtr_[0].reset( new EXP2_Sys1 );
		subsystemDynamicsPtr_[1].reset( new EXP2_Sys2 );
		subsystemDynamicsPtr_[2].reset( new EXP2_Sys3 );
	}

	~EXP2_System() = default;

	EXP2_System(const EXP2_System& other)
	: activeSubsystem_(other.activeSubsystem_),
	  subsystemDynamicsPtr_(3)
	{
		subsystemDynamicsPtr_[0].reset(other.subsystemDynamicsPtr_[0]->clone());
		subsystemDynamicsPtr_[1].reset(other.subsystemDynamicsPtr_[1]->clone());
		subsystemDynamicsPtr_[2].reset(other.subsystemDynamicsPtr_[2]->clone());
	}

	EXP2_System* clone() const final {
		return new EXP2_System(*this);
	}

	void initializeModel(
			LogicRulesMachine<EXP2_LogicRules>& logicRulesMachine,
			const size_t& partitionIndex,
			const char* algorithmName=NULL) final {

		Base::initializeModel(logicRulesMachine, partitionIndex, algorithmName);

		findActiveSubsystemFnc_ = std::move( logicRulesMachine.getHandleToFindActiveEventCounter(partitionIndex) );
	}

	void computeFlowMap(const scalar_t& t, const state_vector_t& x, const input_vector_t& u,
			state_vector_t& dxdt) final {

		activeSubsystem_ = findActiveSubsystemFnc_(t);

		subsystemDynamicsPtr_[activeSubsystem_]->computeFlowMap(t, x, u, dxdt);
	}

private:
	int activeSubsystem_;
	std::function<size_t(scalar_t)> findActiveSubsystemFnc_;
	std::vector<Base::Ptr> subsystemDynamicsPtr_;
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_SysDerivative1 : public DerivativesBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_SysDerivative1() {};
	~EXP2_SysDerivative1() {};

	void getFlowMapDerivativeState(state_matrix_t& A) override {
		A << u_(0)*cos(x_(0))+1, 0, 0, u_(0)*sin(x_(1))-1;
	}
	void getFlowMapDerivativeInput(state_input_matrix_t& B) override {
		B << sin(x_(0)), 0, -cos(x_(1)), 0;
	}

	EXP2_SysDerivative1* clone() const override {
		return new EXP2_SysDerivative1(*this);
	}
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_SysDerivative2 : public DerivativesBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_SysDerivative2() {};
	~EXP2_SysDerivative2() {};

	void getFlowMapDerivativeState(state_matrix_t& A) override {
		A << 0, u_(0)*cos(x_(1))+1, u_(0)*sin(x_(0))-1, 0;
	}
	void getFlowMapDerivativeInput(state_input_matrix_t& B) override {
		B << sin(x_(1)), 0, -cos(x_(0)), 0;
	}

	EXP2_SysDerivative2* clone() const override {
		return new EXP2_SysDerivative2(*this);
	}
};


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_SysDerivative3 : public DerivativesBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_SysDerivative3() {};
	~EXP2_SysDerivative3() {};

	void getFlowMapDerivativeState(state_matrix_t& A) override {
		A << -u_(0)*cos(x_(0))-1, 0, 0, 1-u_(0)*sin(x_(1));
	}
	void getFlowMapDerivativeInput(state_input_matrix_t& B) override {
		B << -sin(x_(0)), 0, cos(x_(1)), 0;
	}

	EXP2_SysDerivative3* clone() const override {
		return new EXP2_SysDerivative3(*this);
	}
};


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_SystemDerivative : public DerivativesBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef DerivativesBase<2,2,EXP2_LogicRules> Base;

	EXP2_SystemDerivative()
	: activeSubsystem_(0),
	  subsystemDerivativesPtr_(3)
	{
		subsystemDerivativesPtr_[0].reset( new EXP2_SysDerivative1 );
		subsystemDerivativesPtr_[1].reset( new EXP2_SysDerivative2 );
		subsystemDerivativesPtr_[2].reset( new EXP2_SysDerivative3 );
	}

	~EXP2_SystemDerivative() {}

	EXP2_SystemDerivative(const EXP2_SystemDerivative& other)
	: activeSubsystem_(other.activeSubsystem_),
	  subsystemDerivativesPtr_(3)
	{
		subsystemDerivativesPtr_[0].reset(other.subsystemDerivativesPtr_[0]->clone());
		subsystemDerivativesPtr_[1].reset(other.subsystemDerivativesPtr_[1]->clone());
		subsystemDerivativesPtr_[2].reset(other.subsystemDerivativesPtr_[2]->clone());
	}


	void initializeModel(
			LogicRulesMachine<EXP2_LogicRules>& logicRulesMachine,
			const size_t& partitionIndex,
			const char* algorithmName=NULL) override {

		Base::initializeModel(logicRulesMachine, partitionIndex, algorithmName);

		findActiveSubsystemFnc_ = std::move( logicRulesMachine.getHandleToFindActiveEventCounter(partitionIndex) );
	}

	EXP2_SystemDerivative* clone() const override {
		return new EXP2_SystemDerivative(*this);
	}

	void setCurrentStateAndControl(const scalar_t& t, const state_vector_t& x, const input_vector_t& u) override {

		Base::setCurrentStateAndControl(t, x, u);
		activeSubsystem_ = findActiveSubsystemFnc_(t);
		subsystemDerivativesPtr_[activeSubsystem_]->setCurrentStateAndControl(t, x, u);
	}

	void getFlowMapDerivativeState(state_matrix_t& A) override {
		subsystemDerivativesPtr_[activeSubsystem_]->getFlowMapDerivativeState(A);
	}

	void getFlowMapDerivativeInput(state_input_matrix_t& B) override {
		subsystemDerivativesPtr_[activeSubsystem_]->getFlowMapDerivativeInput(B);
	}

private:
	int activeSubsystem_;
	std::function<size_t(scalar_t)> findActiveSubsystemFnc_;
	std::vector<Base::Ptr> subsystemDerivativesPtr_;

};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_constraint1 : public ConstraintBase<2, 2, EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_constraint1() = default;
	~EXP2_constraint1() = default;

	void getConstraint1(constraint1_vector_t& e)  final {
		e(0) = u_(1)*sin(x_(0)) - u_(1)*cos(x_(1)) + 0.1*u_(1) - 1;
	}

	size_t numStateInputConstraint(const scalar_t& time) final {
		return 1;
	}

	void getConstraint1DerivativesState(constraint1_state_matrix_t& C) final {
		C.topRows<1>() << u_(1)*cos(x_(0)), u_(1)*sin(x_(1));
	}

	void getConstraint1DerivativesControl(constraint1_input_matrix_t& D) final {
		D.topRows<1>() << 0.0, sin(x_(0))-cos(x_(1))+0.1;
	}

	EXP2_constraint1* clone() const final {
		return new EXP2_constraint1(*this);
	}
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_constraint2 : public ConstraintBase<2, 2, EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_constraint2() = default;
	~EXP2_constraint2() = default;

	void getConstraint1(constraint1_vector_t& e)  final {
		e(0) = u_(1)*sin(x_(1)) - u_(1)*cos(x_(0)) + 0.1*u_(1) - 1;
	}

	size_t numStateInputConstraint(const scalar_t& time) final {
		return 1;
	}

	void getConstraint1DerivativesState(constraint1_state_matrix_t& C) final {
		C.topRows<1>() << u_(1)*sin(x_(1)), -u_(1)*cos(x_(1));
	}

	void getConstraint1DerivativesControl(constraint1_input_matrix_t& D) final {
		D.topRows<1>() << 0.0, sin(x_(1))-cos(x_(0))+0.1;
	}

	EXP2_constraint2* clone() const final {
		return new EXP2_constraint2(*this);
	}
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_constraint3 : public ConstraintBase<2, 2, EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_constraint3() = default;
	~EXP2_constraint3() = default;

	void getConstraint1(constraint1_vector_t& e)  final {
		e(0) = -u_(1)*sin(x_(0)) + u_(1)*cos(x_(1)) + 0.1*u_(1) - 1;
	}

	size_t numStateInputConstraint(const scalar_t& time) final {
		return 1;
	}

	void getConstraint1DerivativesState(constraint1_state_matrix_t& C) final {
		C.topRows<1>() << u_(1)*sin(x_(1)), -u_(1)*cos(x_(1));
	}

	void getConstraint1DerivativesControl(constraint1_input_matrix_t& D) final {
		D.topRows<1>() << 0.0, sin(x_(1))-cos(x_(0))+0.1;
	}

	EXP2_constraint3* clone() const final {
		return new EXP2_constraint3(*this);
	}
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_constraint final : public ConstraintBase<2, 2, EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef ConstraintBase<2, 2, EXP2_LogicRules> Base;

	EXP2_constraint()
	: activeSubsystem_(0),
	  subsystemConstraintPtr_(3)
	{
		subsystemConstraintPtr_[0].reset( new EXP2_constraint1 );
		subsystemConstraintPtr_[1].reset( new EXP2_constraint2 );
		subsystemConstraintPtr_[2].reset( new EXP2_constraint3 );
	}

	~EXP2_constraint() = default;

	EXP2_constraint(const EXP2_constraint& other)
	: activeSubsystem_(other.activeSubsystem_),
	  subsystemConstraintPtr_(3)
	{
		subsystemConstraintPtr_[0].reset(other.subsystemConstraintPtr_[0]->clone());
		subsystemConstraintPtr_[1].reset(other.subsystemConstraintPtr_[1]->clone());
		subsystemConstraintPtr_[2].reset(other.subsystemConstraintPtr_[2]->clone());
	}

	EXP2_constraint* clone() const override {
		return new EXP2_constraint(*this);
	}

	void initializeModel(
			LogicRulesMachine<EXP2_LogicRules>& logicRulesMachine,
			const size_t& partitionIndex,
			const char* algorithmName=NULL) override {

		Base::initializeModel(logicRulesMachine, partitionIndex, algorithmName);

		findActiveSubsystemFnc_ = std::move( logicRulesMachine.getHandleToFindActiveEventCounter(partitionIndex) );
	}

	void setCurrentStateAndControl(
			const scalar_t& t,
			const state_vector_t& x,
			const input_vector_t& u) final {

		Base::setCurrentStateAndControl(t, x, u);
		activeSubsystem_ = findActiveSubsystemFnc_(t);
		subsystemConstraintPtr_[activeSubsystem_]->setCurrentStateAndControl(t, x, u);
	}


	void getConstraint1(constraint1_vector_t& e) final {

		subsystemConstraintPtr_[activeSubsystem_]->getConstraint1(e);
	}

	size_t numStateInputConstraint(const scalar_t& time) final {
		return subsystemConstraintPtr_[activeSubsystem_]->numStateInputConstraint(time);
	}

	void getConstraint1DerivativesState(constraint1_state_matrix_t& C) final {
		subsystemConstraintPtr_[activeSubsystem_]->getConstraint1DerivativesState(C);
	}

	void getConstraint1DerivativesControl(constraint1_input_matrix_t& D) final {
		subsystemConstraintPtr_[activeSubsystem_]->getConstraint1DerivativesControl(D);
	}

private:
	int activeSubsystem_;
	std::function<size_t(scalar_t)> findActiveSubsystemFnc_;
	std::vector<Base::Ptr> subsystemConstraintPtr_;
};


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_CostFunction1 : public CostFunctionBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_CostFunction1() = default;
	~EXP2_CostFunction1() = default;

	void getIntermediateCost(scalar_t& L) final {
		L = 0.5*pow(x_(0)-1.0, 2) + 0.5*pow(x_(1)+1.0, 2) + 0.5*pow(u_(0), 2) + + 0.5*alpha_*pow(u_(1), 2);
	}

	void getIntermediateCostDerivativeState(state_vector_t& dLdx) final { dLdx << (x_(0)-1.0), (x_(1)+1.0); }
	void getIntermediateCostSecondDerivativeState(state_matrix_t& dLdxx) final { dLdxx << 1.0, 0.0, 0.0, 1.0; }
	void getIntermediateCostDerivativeInput(input_vector_t& dLdu) final { dLdu << u_(0), alpha_*u_(1); }
	void getIntermediateCostSecondDerivativeInput(input_matrix_t& dLduu) final { dLduu << 1.0, 0.0, 0.0, alpha_; }

	void getIntermediateCostDerivativeInputState(input_state_matrix_t& dLdxu) final { dLdxu.setZero(); }

	void getTerminalCost(scalar_t& Phi) final { Phi = 0; }
	void getTerminalCostDerivativeState(state_vector_t& dPhidx) final { dPhidx.setZero(); }
	void getTerminalCostSecondDerivativeState(state_matrix_t& dPhidxx) final { dPhidxx.setZero(); }

	EXP2_CostFunction1* clone() const final {
		return new EXP2_CostFunction1(*this);
	};

private:
	double alpha_ = 0.1;
};


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_CostFunction2 : public CostFunctionBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_CostFunction2() = default;
	~EXP2_CostFunction2() = default;

	void getIntermediateCost(scalar_t& L) final {
		L = 0.5*pow(x_(0)-1.0, 2) + 0.5*pow(x_(1)+1.0, 2) + 0.5*pow(u_(0), 2) + 0.5*alpha_*pow(u_(1), 2);
	}

	void getIntermediateCostDerivativeState(state_vector_t& dLdx) final { dLdx << (x_(0)-1.0), (x_(1)+1.0); }
	void getIntermediateCostSecondDerivativeState(state_matrix_t& dLdxx) final { dLdxx << 1.0, 0.0, 0.0, 1.0; }
	void getIntermediateCostDerivativeInput(input_vector_t& dLdu) final { dLdu << u_(0), alpha_*u_(1); }
	void getIntermediateCostSecondDerivativeInput(input_matrix_t& dLduu) final { dLduu << 1.0, 0.0, 0.0, alpha_; }

	void getIntermediateCostDerivativeInputState(input_state_matrix_t& dLdxu) final { dLdxu.setZero(); }

	void getTerminalCost(scalar_t& Phi) final { Phi = 0; }
	void getTerminalCostDerivativeState(state_vector_t& dPhidx) final { dPhidx.setZero(); }
	void getTerminalCostSecondDerivativeState(state_matrix_t& dPhidxx) final { dPhidxx.setZero(); }

	EXP2_CostFunction2* clone() const final {
		return new EXP2_CostFunction2(*this);
	};

private:
	double alpha_ = 0.1;
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_CostFunction3 : public CostFunctionBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EXP2_CostFunction3() = default;
	~EXP2_CostFunction3() = default;

	void getIntermediateCost(scalar_t& L) final {
		L = 0.5*pow(x_(0)-1.0, 2) + 0.5*pow(x_(1)+1.0, 2) + 0.5*pow(u_(0), 2) + 0.5*alpha_*pow(u_(1), 2);
	}

	void getIntermediateCostDerivativeState(state_vector_t& dLdx) final { dLdx << (x_(0)-1.0), (x_(1)+1.0); }
	void getIntermediateCostSecondDerivativeState(state_matrix_t& dLdxx) final { dLdxx << 1.0, 0.0, 0.0, 1.0; }
	void getIntermediateCostDerivativeInput(input_vector_t& dLdu) final { dLdu << u_(0), alpha_*u_(1); }
	void getIntermediateCostSecondDerivativeInput(input_matrix_t& dLduu) final { dLduu << 1.0, 0.0, 0.0, alpha_; }

	void getIntermediateCostDerivativeInputState(input_state_matrix_t& dLdxu) final { dLdxu.setZero(); }

	void getTerminalCost(scalar_t& Phi) final { Phi = 0.5*pow(x_(0)-1.0, 2) + 0.5*pow(x_(1)+1.0, 2); }
	void getTerminalCostDerivativeState(state_vector_t& dPhidx) final { dPhidx << (x_(0)-1.0), (x_(1)+1.0); }
	void getTerminalCostSecondDerivativeState(state_matrix_t& dPhidxx) final { dPhidxx << 1.0, 0.0, 0.0, 1.0; }

	EXP2_CostFunction3* clone() const final {
		return new EXP2_CostFunction3(*this);
	};

private:
	double alpha_ = 0.1;
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP2_CostFunction final : public CostFunctionBase<2,2,EXP2_LogicRules>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef CostFunctionBase<2,2,EXP2_LogicRules> Base;

	EXP2_CostFunction()
	: activeSubsystem_(0),
	  subsystemCostsPtr_(3)
	{
		subsystemCostsPtr_[0].reset( new EXP2_CostFunction1 );
		subsystemCostsPtr_[1].reset( new EXP2_CostFunction2 );
		subsystemCostsPtr_[2].reset( new EXP2_CostFunction3 );
	}

	~EXP2_CostFunction() {}

	EXP2_CostFunction(const EXP2_CostFunction& other)
	: activeSubsystem_(other.activeSubsystem_),
	  subsystemCostsPtr_(3)
	{
		subsystemCostsPtr_[0].reset(other.subsystemCostsPtr_[0]->clone());
		subsystemCostsPtr_[1].reset(other.subsystemCostsPtr_[1]->clone());
		subsystemCostsPtr_[2].reset(other.subsystemCostsPtr_[2]->clone());
	}

	void initializeModel(
			LogicRulesMachine<EXP2_LogicRules>& logicRulesMachine,
			const size_t& partitionIndex,
			const char* algorithmName=NULL) override {

		Base::initializeModel(logicRulesMachine, partitionIndex, algorithmName);

		findActiveSubsystemFnc_ = std::move( logicRulesMachine.getHandleToFindActiveEventCounter(partitionIndex) );
	}

	EXP2_CostFunction* clone() const override {
		return new EXP2_CostFunction(*this);
	}

	void setCurrentStateAndControl(const scalar_t& t, const state_vector_t& x, const input_vector_t& u) override {

		Base::setCurrentStateAndControl(t, x, u);
		activeSubsystem_ = findActiveSubsystemFnc_(t);
		subsystemCostsPtr_[activeSubsystem_]->setCurrentStateAndControl(t, x, u);
	}

	void getIntermediateCost(scalar_t& L) {
		subsystemCostsPtr_[activeSubsystem_]->getIntermediateCost(L);
	}

	void getIntermediateCostDerivativeState(state_vector_t& dLdx) {
		subsystemCostsPtr_[activeSubsystem_]->getIntermediateCostDerivativeState(dLdx);
	}
	void getIntermediateCostSecondDerivativeState(state_matrix_t& dLdxx)  {
		subsystemCostsPtr_[activeSubsystem_]->getIntermediateCostSecondDerivativeState(dLdxx);
	}
	void getIntermediateCostDerivativeInput(input_vector_t& dLdu)  {
		subsystemCostsPtr_[activeSubsystem_]->getIntermediateCostDerivativeInput(dLdu);
	}
	void getIntermediateCostSecondDerivativeInput(input_matrix_t& dLduu)  {
		subsystemCostsPtr_[activeSubsystem_]->getIntermediateCostSecondDerivativeInput(dLduu);
	}

	void getIntermediateCostDerivativeInputState(input_state_matrix_t& dLdxu) {
		subsystemCostsPtr_[activeSubsystem_]->getIntermediateCostDerivativeInputState(dLdxu);
	}

	void getTerminalCost(scalar_t& Phi) {
		subsystemCostsPtr_[activeSubsystem_]->getTerminalCost(Phi);
	}
	void getTerminalCostDerivativeState(state_vector_t& dPhidx)  {
		subsystemCostsPtr_[activeSubsystem_]->getTerminalCostDerivativeState(dPhidx);
	}
	void getTerminalCostSecondDerivativeState(state_matrix_t& dPhidxx)  {
		subsystemCostsPtr_[activeSubsystem_]->getTerminalCostSecondDerivativeState(dPhidxx);
	}

public:
	int activeSubsystem_;
	std::function<size_t(scalar_t)> findActiveSubsystemFnc_;
	std::vector<std::shared_ptr<Base>> subsystemCostsPtr_;

};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
using EXP2_SystemOperatingTrajectories = SystemOperatingPoint<2,2,EXP2_LogicRules>;


} // namespace ocs2


#endif /* EXP2_OCS2_H_ */
