/*********************************************************************
 Computational Learning and Motor Control Lab
 University of Southern California
 Prof. Stefan Schaal
 *********************************************************************
 \remarks      ...

 \file         template_matching.h

 \author       Alexander Herzog
 \date         April 1, 2012

 *********************************************************************/

#ifndef TEMPLATE_MATCHING_H_
#define TEMPLATE_MATCHING_H_

#include <vector>
#include <map>

#include <grasp_template/grasp_template.h>
#include <grasp_template/dismatch_measure.h>
#include <grasp_template_planning/grasp_pool.h>
#include <grasp_template_planning/grasp_creator_interface.h>
#include <grasp_template_planning/grasp_planning_params.h>

namespace grasp_template_planning
{

class TemplateMatching : public GraspPlanningParams
{
public:
  TemplateMatching(GraspCreatorInterface const* grasp_creator, boost::shared_ptr<const std::vector<
      grasp_template::GraspTemplate> > candidates, boost::shared_ptr<const std::vector<GraspAnalysis> > lib_grasps,
                   boost::shared_ptr<const std::vector<std::vector<GraspAnalysis> > > lib_failures);

  boost::shared_ptr<const std::vector<grasp_template::GraspTemplate> > candidates_;

  unsigned int size() const {return ranking_.size();};
  double getLibScore(unsigned int rank) const {return lib_scores_[ranking_[rank]].getScore();};
  double getFailScore(unsigned int rank) const {return fail_scores_[ranking_[rank]].getScore();};
  double getLibQuality(unsigned int rank) const {return lib_qualities_[candidate_to_lib_[ranking_[rank]]].getScore();};
  double getHypothesisScore(unsigned int rank) const {return computeScore(ranking_[rank]);};
  GraspAnalysis getGrasp(unsigned int rank) const;
  double getScore(unsigned int rank) const;
  const GraspAnalysis& getLib(unsigned int rank) const;
  std::string getScoreFormula() const;

  void create();
  bool exceedsDissimilarityThreshold(const GraspAnalysis& succ_demo) const;
  void writeScores(std::ostream& stream, unsigned int max_scores) const;

private:
  GraspCreatorInterface const* grasp_creator_;
  boost::shared_ptr<const std::vector<GraspAnalysis> > lib_grasps_;
  boost::shared_ptr<const std::vector<std::vector<GraspAnalysis> > > lib_failures_;
  std::vector<grasp_template::DismatchMeasure> lib_match_handler_;
  std::vector<grasp_template::TemplateDissimilarity> lib_scores_;
  std::vector<grasp_template::TemplateDissimilarity> fail_scores_;
  std::vector<grasp_template::TemplateDissimilarity> lib_qualities_;
  std::vector<unsigned int> candidate_to_lib_;
  std::vector<int> candidate_to_fail_;
  std::vector<int> lib_to_fail_;
  std::vector<unsigned int> ranking_;

  void computeLibScore(grasp_template::GraspTemplate& candidate,
      grasp_template::TemplateDissimilarity& score, unsigned int index) const;
  void computeLibQuality(unsigned int lib_index);
  void computeFailScore(unsigned int candidate, unsigned int lib_index,
      grasp_template::TemplateDissimilarity& score, int& fail_index) const;
  double computeScore(double a, double b, double c) const;
  double computeScore(unsigned int cand) const;
  double getLibOverlay(unsigned int rank) const;
};

} //namespace
#endif /* TEMPLATE_MATCHING_H_ */
