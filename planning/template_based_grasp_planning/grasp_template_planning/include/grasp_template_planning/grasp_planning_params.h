/*********************************************************************
 Computational Learning and Motor Control Lab
 University of Southern California
 Prof. Stefan Schaal
 *********************************************************************
 \remarks      ...

 \file         grasp_planning_params.h

 \author       Alexander Herzog
 \date         April 1, 2012

 *********************************************************************/

#ifndef GRASP_PLANNING_PARAMS_H_
#define GRASP_PLANNING_PARAMS_H_

#include <string>
#include <vector>
#include <Eigen/Eigen>
#include <boost/thread.hpp>

#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/Pose.h>
#include <grasp_template_planning/GraspAnalysis.h>

//#define GTP_SAFER_CREATE_ID_

namespace grasp_template_planning
{

class GraspPlanningParams
{
public:

  GraspPlanningParams();

  /* grasp demo labels */
  std::string labelGraspDemoHEO() const {return "hand enclosures object";};
  std::string labelGraspDemoODRSent() const {return "object detection request sent";};
  std::string labelGraspDemoODRReceived() const {return "object detection response received";};
  std::string labelGraspDemoId() const {return "demo_id:";};

  /* grasp demo topics
   * remember to add publish new variables to rosparam serverm
   */
  std::string topicGraspDemoEvents() const {return "/grasp_demo_events";};
  std::string topicGraspDemoObjectCluster() const {return "/grasp_demo_object_cluster";};
  std::string topicGraspDemoTable() const {return "/grasp_demo_table";};
  std::string topicGraspDemoGripperPose() const {return "/grasp_demo_gripper_pose";};
  std::string topicViewpointTransform() const {return "/grasp_demo_viewpoint_transform";};
  std::string topicFingerpositions() const {return "/grasp_demo_fingerpositions";};
  std::string topicTransformFrames() const {return "/tf";};
  std::string topicJointStates() const {return "/joint_states";};
  std::string topicAnalyzedGrasps() const {return "/analyzed_grasp_data";};
  std::string topicGraspMatchings() const {return "/grasp_matchings_data";};

  const std::string& frameGripper() const {return frame_gripper_;};
  double learningLibQualFac() const {return learning_lib_quality_factor_;}
  double learningFailDistFac() const {return learning_fail_distance_factor_;}
  double learningSuccAddDist() const {return learning_fail_distance_factor_;}
  void getTemplateExtractionPoint(Eigen::Vector3d& delta) const
      {delta = template_extraction_point_;};

  static std::string createId();
  bool getTransform(tf::StampedTransform& transform, const std::string& from,
      const std::string& to) const;
  std::string getRelatedFailureLib(const GraspAnalysis& grasp_lib_entry) const;
  std::string createNewDemoFilename(const GraspAnalysis& grasp_lib_entry) const;

private:
#ifdef GTP_SAFER_CREATE_ID_
  static boost::mutex mutex_;
  static unsigned int safe_id_counter_;
#endif

  double learning_lib_quality_factor_, learning_fail_distance_factor_,
      learning_success_add_dist_;
  std::string frame_gripper_;
  Eigen::Vector3d template_extraction_point_;
};

} //namespace
#endif /* GRASP_PLANNING_PARAMS_H_ */
