// Copyright (c) 2019, Map IV, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the Map IV, Inc. nor the names of its contributors
//   may be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/*
 * slip_angle.cpp
 * Author MapIV Sekino
 */

#include "ros/ros.h"
#include "coordinate/coordinate.hpp"
#include "navigation/navigation.hpp"

static sensor_msgs::Imu imu;
static eagleye_msgs::VelocityScaleFactor velocity_scale_factor;
static eagleye_msgs::YawrateOffset yawrate_offset_stop;
static eagleye_msgs::YawrateOffset yawrate_offset_2nd;

static ros::Publisher pub;
static eagleye_msgs::SlipAngle slip_angle;

struct SlipangleParameter slip_angle_parameter;


void velocity_scale_factor_callback(const eagleye_msgs::VelocityScaleFactor::ConstPtr& msg)
{
  velocity_scale_factor.header = msg->header;
  velocity_scale_factor.scale_factor = msg->scale_factor;
  velocity_scale_factor.correction_velocity = msg->correction_velocity;
  velocity_scale_factor.status = msg->status;
}

void yawrate_offset_stop_callback(const eagleye_msgs::YawrateOffset::ConstPtr& msg)
{
  yawrate_offset_stop.header = msg->header;
  yawrate_offset_stop.yawrate_offset = msg->yawrate_offset;
  yawrate_offset_stop.status = msg->status;
}

void yawrate_offset_2nd_callback(const eagleye_msgs::YawrateOffset::ConstPtr& msg)
{
  yawrate_offset_2nd.header = msg->header;
  yawrate_offset_2nd.yawrate_offset = msg->yawrate_offset;
  yawrate_offset_2nd.status = msg->status;
}

void imu_callback(const sensor_msgs::Imu::ConstPtr& msg)
{
  imu.header = msg->header;
  imu.orientation = msg->orientation;
  imu.orientation_covariance = msg->orientation_covariance;
  imu.angular_velocity = msg->angular_velocity;
  imu.angular_velocity_covariance = msg->angular_velocity_covariance;
  imu.linear_acceleration = msg->linear_acceleration;
  imu.linear_acceleration_covariance = msg->linear_acceleration_covariance;
  slip_angle.header = msg->header;
  slip_angle.header.frame_id = "base_link";
  slip_angle_estimate(imu,velocity_scale_factor,yawrate_offset_stop,yawrate_offset_2nd,slip_angle_parameter,&slip_angle);
  pub.publish(slip_angle);
  slip_angle.status.estimate_status = false;
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "slip_angle");

  ros::NodeHandle n;

  std::string subscribe_imu_topic_name = "/imu/data_raw";

  n.getParam("imu_topic",subscribe_imu_topic_name);
  n.getParam("reverse_imu", slip_angle_parameter.reverse_imu);
  n.getParam("slip_angle/manual_coefficient", slip_angle_parameter.manual_coefficient);
  n.getParam("slip_angle/stop_judgment_velocity_threshold", slip_angle_parameter.stop_judgment_velocity_threshold);
  std::cout<< "subscribe_imu_topic_name "<<subscribe_imu_topic_name<<std::endl;
  std::cout<< "reverse_imu "<<slip_angle_parameter.reverse_imu<<std::endl;
  std::cout<< "manual_coefficient "<<slip_angle_parameter.manual_coefficient<<std::endl;
  std::cout<< "stop_judgment_velocity_threshold "<<slip_angle_parameter.stop_judgment_velocity_threshold<<std::endl;

  ros::Subscriber sub1 = n.subscribe(subscribe_imu_topic_name, 1000, imu_callback, ros::TransportHints().tcpNoDelay());
  ros::Subscriber sub2 = n.subscribe("velocity_scale_factor", 1000, velocity_scale_factor_callback, ros::TransportHints().tcpNoDelay());
  ros::Subscriber sub3 = n.subscribe("yawrate_offset_stop", 1000, yawrate_offset_stop_callback, ros::TransportHints().tcpNoDelay());
  ros::Subscriber sub4 = n.subscribe("yawrate_offset_2nd", 1000, yawrate_offset_2nd_callback, ros::TransportHints().tcpNoDelay());
  pub = n.advertise<eagleye_msgs::SlipAngle>("slip_angle", 1000);

  ros::spin();

  return 0;
}
