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

#ifndef MRT_ROS_DUMMY_BALLBOT_OCS2_H_
#define MRT_ROS_DUMMY_BALLBOT_OCS2_H_

#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/tf.h>
#include <tf/transform_broadcaster.h>

#include <ocs2_comm_interfaces/test/MRT_ROS_Dummy_Loop.h>
#include "ocs2_ballbot_example/definitions.h"

namespace ocs2 {
namespace ballbot {

class MRT_ROS_Dummy_Ballbot : public MRT_ROS_Dummy_Loop<ballbot::STATE_DIM_, ballbot::INPUT_DIM_>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef MRT_ROS_Dummy_Loop<ballbot::STATE_DIM_, ballbot::INPUT_DIM_> BASE;

	/**
	 * Constructor.
	 *
	 * @param [in] mrtPtr
	 * @param [in] mrtDesiredFrequency: MRT loop frequency in Hz. This should always set to a positive number.
	 * @param [in] mpcDesiredFrequency: MPC loop frequency in Hz. If set to a positive number, MPC loop
	 * will be simulated to run by this frequency. Note that this might not be the MPC's realtime frequency.
	 */
	MRT_ROS_Dummy_Ballbot(
			const mrt_ptr_t& mrtPtr,
			const scalar_t& mrtDesiredFrequency,
			const scalar_t& mpcDesiredFrequency)
	: BASE(mrtPtr, mrtDesiredFrequency, mpcDesiredFrequency)
	{}

	/**
	 * Destructor.
	 */
	virtual ~MRT_ROS_Dummy_Ballbot() = default;

	/**
	 * The initialization of the observation
	 *
	 * @param [in] initObservation: The initial observation.
	 */
	virtual void init(const system_observation_t& initObservation) override {

		BASE::init(initObservation);
	}

	void updateTfPublisher(const system_observation_t& observation){

		//compute positions of the markers based on system observation
		Eigen::Vector3d positionWorldToBall, positionWorldToBase, positionWorldToArmBase;
		Eigen::Quaterniond quaternionBaseToWorld;
		Eigen::Matrix3d rotationMatrixBaseToWorld;

		quaternionBaseToWorld = Eigen::AngleAxisd{observation.state()(2), Eigen::Vector3d{0, 0, 1}}*
								Eigen::AngleAxisd{observation.state()(3), Eigen::Vector3d{0, 1, 0}}*
								Eigen::AngleAxisd{observation.state()(4), Eigen::Vector3d{1, 0, 0}};



		rotationMatrixBaseToWorld = quaternionBaseToWorld.normalized().toRotationMatrix();

		positionWorldToBall << observation.state()(0), observation.state()(1), 0.125;


		positionWorldToBase = positionWorldToBall + rotationMatrixBaseToWorld*Eigen::Vector3d(0.0, 0.0, 0.317);

		positionWorldToArmBase = positionWorldToBall + rotationMatrixBaseToWorld*Eigen::Vector3d(0.0, 0.0, 0.317 + 0.266);


		// Broadcast transformation from rezero observation to robot base.
		geometry_msgs::TransformStamped base_transform;
		base_transform.header.frame_id = "odom";
		base_transform.child_frame_id = "base";
		base_transform.transform.translation.x = positionWorldToBase.x();
		base_transform.transform.translation.y = positionWorldToBase.y();
		base_transform.transform.translation.z = positionWorldToBase.z();
		// RViz wants orientationBaseToWorld:
		base_transform.transform.rotation.w = quaternionBaseToWorld.w();
		base_transform.transform.rotation.x = quaternionBaseToWorld.x();
		base_transform.transform.rotation.y = quaternionBaseToWorld.y();
		base_transform.transform.rotation.z = quaternionBaseToWorld.z();

		tfBroadcasterPtr_->sendTransform(base_transform);

		// Broadcast transformation from rezero observation to robot ball
		geometry_msgs::TransformStamped ball_transform;
		ball_transform.header.frame_id = "base";
		ball_transform.child_frame_id = "ball";
		ball_transform.transform.translation.x = 0.0;
		ball_transform.transform.translation.y = 0.0;
		ball_transform.transform.translation.z = -0.317;
		ball_transform.transform.rotation.w = 1.0;
		ball_transform.transform.rotation.x = 0.0;
		ball_transform.transform.rotation.y = 0.0;
		ball_transform.transform.rotation.z = 0.0;

		tfBroadcasterPtr_->sendTransform(ball_transform);
	}

protected:
	/**
	 * Launches the visualization node
	 *
	 * @param [in] argc: command line number of inputs.
	 * @param [in] argv: command line inputs' value.
	 */
	virtual void launchVisualizerNode(int argc, char* argv[]) override {

		ros::init(argc, argv, "ballbot_visualization_node");

		ros::NodeHandle n;
		visualizationPublisher_ = n.advertise<visualization_msgs::MarkerArray>("ballbot_vis", 10);
		ROS_INFO_STREAM("Waiting for visualization subscriber ...");
		while(ros::ok() && visualizationPublisher_.getNumSubscribers() == 0)
			ros::Rate(100).sleep();
		ROS_INFO_STREAM("Visualization subscriber is connected.");

		tfBroadcasterPtr_.reset(new tf::TransformBroadcaster);
	}

	/**
	 * Visualizes the current observation.
	 *
	 * @param [in] observation: The current observation.
	 */
	virtual void publishVisualizer(const system_observation_t& observation) override {

		updateTfPublisher(observation);

		visualization_msgs::MarkerArray markerArray;
		// Marker for Base
		visualization_msgs::Marker baseMarker;
		baseMarker.header.frame_id = "base";
		baseMarker.ns = "";
		baseMarker.id = 0;
		baseMarker.type = visualization_msgs::Marker::MESH_RESOURCE;
		baseMarker.mesh_resource = "package://ocs2_ballbot_example/urdf/meshes/complete_robot_june.dae";
		baseMarker.action = visualization_msgs::Marker::ADD;
		baseMarker.pose.position.x = 0.0;
		baseMarker.pose.position.y = 0.0;
		// the mesh has its origin in the center of the ball
		baseMarker.pose.position.z = -0.317;
		baseMarker.pose.orientation.x = 0.0;
		baseMarker.pose.orientation.y = 0.0;
		baseMarker.pose.orientation.z = 0.0;
		baseMarker.pose.orientation.w = 1.0;
		baseMarker.scale.x = 1.0;
		baseMarker.scale.y = 1.0;
		baseMarker.scale.z = 1.0;
		baseMarker.color.a = 1.0; // Don't forget to set the alpha!
		baseMarker.color.r = 0.5;
		baseMarker.color.g = 0.5;
		baseMarker.color.b = 0.5;
		markerArray.markers.push_back(baseMarker);

		// Marker for Ball
		visualization_msgs::Marker ballMarker;
		ballMarker.header.frame_id = "base";
		ballMarker.header.stamp = ros::Time();
		ballMarker.ns = "";
		ballMarker.id = 1;
		ballMarker.type = visualization_msgs::Marker::SPHERE;
		ballMarker.action = visualization_msgs::Marker::ADD;
		ballMarker.pose.position.x = 0.0;
		ballMarker.pose.position.y = 0.0;
		ballMarker.pose.position.z = -0.317;
		ballMarker.pose.orientation.x = 0.0;
		ballMarker.pose.orientation.y = 0.0;
		ballMarker.pose.orientation.z = 0.0;
		ballMarker.pose.orientation.w = 1.0;
		ballMarker.scale.x = 0.25;
		ballMarker.scale.y = 0.25;
		ballMarker.scale.z = 0.25;
		ballMarker.color.a = 1.0; // Don't forget to set the alpha!
		ballMarker.color.r = 0.0;
		ballMarker.color.g = 0.0;
		ballMarker.color.b = 0.0;
		markerArray.markers.push_back(ballMarker);

		//visualizationPublisher_.publish(baseMarker);
		visualizationPublisher_.publish(markerArray);
	}

	/************
	 * Variables
	 ************/
	ros::Publisher visualizationPublisher_;
	ros::Publisher posePublisher_;
	std::unique_ptr<tf::TransformBroadcaster> tfBroadcasterPtr_;

};

} // namespace ballbot
} // namespace ocs2

#endif /* MRT_ROS_DUMMY_BALLBOT_OCS2_H_ */
