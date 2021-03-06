/*********************************************************************
 Computational Learning and Motor Control Lab
 University of Southern California
 Prof. Stefan Schaal
 *********************************************************************
 \remarks      ...

 \file         object_detection_listener.h

 \author       Alexander Herzog
 \date         April 1, 2012

 *********************************************************************/

#ifndef OBJECT_DETECTION_LISTENER_H_
#define OBJECT_DETECTION_LISTENER_H_

#include <limits>
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Pose.h>
#include <tabletop_object_detector/TabletopSegmentation.h>

namespace grasp_template_planning
{

class ObjectDetectionListener
{
public:

  ObjectDetectionListener(){};

  const sensor_msgs::PointCloud& getCluster() const;
  const geometry_msgs::PoseStamped getTableFrame() const;

  void connectToObjectDetector(ros::NodeHandle& n);
  bool fetchClusterFromObjectDetector();

private:

  ros::ServiceClient cluster_client_;
  geometry_msgs::PoseStamped table_frame_;
  tabletop_object_detector::TabletopSegmentation tod_communication_;
  sensor_msgs::PointCloud object_cluster_; //closest object
};

}//  namespace
#endif /* OBJECT_DETECTION_LISTENER_H_ */
