/*********************************************************************
 Computational Learning and Motor Control Lab
 University of Southern California
 Prof. Stefan Schaal
 *********************************************************************
 \remarks      ...

 \file         template_matching.cpp

 \author       Alexander Herzog
 \date         April 1, 2012

 *********************************************************************/

#include <grasp_template_planning/template_matching.h>

using namespace std;
using namespace grasp_template;

namespace grasp_template_planning
{

TemplateMatching::TemplateMatching(GraspCreatorInterface const* grasp_creator, boost::shared_ptr<const vector<
    GraspTemplate> > candidates, boost::shared_ptr<const vector<GraspAnalysis> > lib_grasps,
    boost::shared_ptr<const vector<vector<GraspAnalysis> > > lib_failures)
{
  grasp_creator_ = grasp_creator;
  candidates_ = candidates;
  lib_grasps_ = lib_grasps;
  lib_failures_ = lib_failures;

  lib_scores_.resize((*candidates_).size());
  fail_scores_.resize((*candidates_).size());
  lib_qualities_.resize((*lib_grasps_).size());
  candidate_to_lib_.resize((*candidates_).size());
  candidate_to_fail_.resize((*candidates_).size());
  lib_to_fail_.resize((*lib_grasps_).size());

  lib_match_handler_.clear();
  for (unsigned int i = 0; i < (*lib_grasps_).size(); i++)
  {
    const GraspAnalysis& lib_templt = (*lib_grasps_)[i];
    lib_match_handler_.push_back(DismatchMeasure(lib_templt.grasp_template, lib_templt.template_pose.pose,
                                                 lib_templt.gripper_pose.pose));
  }
}

GraspAnalysis TemplateMatching::getGrasp(unsigned int rank) const
{
  const GraspTemplate& templt = (*candidates_)[ranking_[rank]];
  const GraspAnalysis& lib = (*lib_grasps_)[candidate_to_lib_[ranking_[rank]]];
  GraspAnalysis res;
  grasp_creator_->createGrasp(templt, lib, res);
  return res;
}

double TemplateMatching::getScore(unsigned int rank) const
{
  return computeScore(ranking_[rank]);
}

const GraspAnalysis& TemplateMatching::getLib(unsigned int rank) const
{
  return (*lib_grasps_)[candidate_to_lib_[ranking_[rank]]];
}

string TemplateMatching::getScoreFormula() const
{
  string first = "a/[(1 - exp(- ";
  stringstream ss;
  ss << learningLibQualFac();
  string second;
  ss >> second;
  ss.clear();
  string thirdt = " * b))(1 - exp(- ";
  ss << learningFailDistFac();
  string fourth;
  ss >> fourth;
  ss.clear();
  string fifth = " * c))]";

  string result = first;
  result.append(second);
  result.append(thirdt);
  result.append(fourth);
  result.append(fifth);

  return result;
}

void TemplateMatching::create()
{
  unsigned int lib_index = 0;
#pragma omp parallel for private(lib_index)
  for (lib_index = 0; lib_index < (*lib_grasps_).size(); lib_index++)
  {
    computeLibQuality(lib_index);
  }

  unsigned int cand = 0;
#pragma omp parallel for private(cand)
  for (cand = 0; cand < (*candidates_).size(); cand++)
  {
    TemplateDissimilarity best_cf, best_cl, best_lf;
    double best_m = numeric_limits<double>::max();
    int best_fail_ind = -1;
    unsigned int best_lib_id = 0;
    for (unsigned int lib = 0; lib < (*lib_grasps_).size(); lib++)
    {
      TemplateDissimilarity cur_cf, cur_cl, cur_lf;
      double a, b, c;

      //compute m(c, f)
      int cur_fail_index = -1;
      computeFailScore(cand, lib, cur_cf, cur_fail_index);
      if (cur_fail_index >= 0)
      {
        b = cur_cf.getScore();
      }
      else
      {
        b = -1;
      }

      //compute m(c, l)
      {
        GraspTemplate sample((*candidates_)[cand]);
        const DismatchMeasure& mh = lib_match_handler_[lib];

        mh.applyDcMask(sample);
        cur_cl = mh.getScore(sample);

        a = cur_cl.getScore();
      }

      //compute m(l, f)
      if (lib_to_fail_[lib] >= 0)
      {
        cur_lf = lib_qualities_[lib];
        c = cur_lf.getScore();
      }
      else
      {
        c = -1;
      }

      double m = computeScore(a, b, c);

      if (m < best_m)
      {
        best_m = m;
        best_cl = cur_cl;
        best_cf = cur_cf;
        best_lf = cur_lf;
        best_fail_ind = cur_fail_index;
        best_lib_id = lib;
      }
    }

    candidate_to_lib_[cand] = best_lib_id;
    candidate_to_fail_[cand] = best_fail_ind;
    lib_scores_[cand] = best_cl;
    fail_scores_[cand] = best_cf;
  }

  map<double, unsigned int> ranking_map;
  for (unsigned int i = 0; i < (*candidates_).size(); i++)
  {
    double score = computeScore(i);
    //map does not store replicants!!!!!! -> candidates.size() != ranking_.size()
    ranking_map.insert(make_pair<double, unsigned int> (score, i));
  }

  ranking_.clear();
  for (map<double, unsigned int>::const_iterator it = ranking_map.begin(); it != ranking_map.end(); it++)
  {
    ranking_.push_back(it->second);
  }
}

bool TemplateMatching::exceedsDissimilarityThreshold(const GraspAnalysis& succ_demo) const
{
  GraspTemplate sample(succ_demo.grasp_template, succ_demo.template_pose.pose);
  TemplateDissimilarity score;
  unsigned int lib_index = -1;
  computeLibScore(sample, score, lib_index);

  return score.getScore() > learningSuccAddDist();
}

void TemplateMatching::writeScores(ostream& stream, unsigned int max_scores) const
{
  stream << "filename" << "\t" << "lib_score" << "\t" << "first_s" << "\t" << "second_s" << "\t" << "ss" << endl;
  for (unsigned int i = 0; i < max_scores && i < size(); i++)
  {
    stream << getLib(i).demo_filename << "\t";
    stream << getScore(i) << "\t";
    stream << getLibOverlay(i) << "\t";
    stream << lib_scores_[ranking_[i]].sd_ + lib_scores_[ranking_[i]].sf_ + lib_scores_[ranking_[i]].ss_
        + lib_scores_[ranking_[i]].st_ << "\t";
    stream << lib_scores_[ranking_[i]].ds_ + lib_scores_[ranking_[i]].fs_ + lib_scores_[ranking_[i]].ss_
        + lib_scores_[ranking_[i]].ts_ << "\t";
    stream << lib_scores_[ranking_[i]].ss_;
    stream << endl;
  }
}

void TemplateMatching::computeLibScore(GraspTemplate& candidate, TemplateDissimilarity& score, unsigned int index) const
{
  for (unsigned int i = 0; i < (*lib_grasps_).size(); i++)
  {
    const DismatchMeasure& mh = lib_match_handler_[i];

    mh.applyDcMask(candidate);
    TemplateDissimilarity cur = mh.getScore(candidate);

    if (i == 0 || cur.isBetter(cur, score))
    {
      score = cur;
      index = i;
    }
  }
}

void TemplateMatching::computeLibQuality(unsigned int lib_index)
{
  TemplateDissimilarity closest;
  int fail_index = -1;

  if (lib_failures_ != NULL)
  {
    for (unsigned int i = 0; i < (*lib_failures_)[lib_index].size(); i++)
    {
      GraspTemplate f_templt((*lib_failures_)[lib_index][i].grasp_template,
                             (*lib_failures_)[lib_index][i].template_pose.pose);
      lib_match_handler_[lib_index].applyDcMask(f_templt);

      TemplateDissimilarity cur = lib_match_handler_[lib_index].getScore(f_templt);
      if (i == 0 || cur.isBetter(cur, closest))
      {
        closest = cur;
        fail_index = i;
      }
    }
  }

  lib_qualities_[lib_index] = closest;
  lib_to_fail_[lib_index] = fail_index;
}

void TemplateMatching::computeFailScore(unsigned int candidate, unsigned int lib_index, TemplateDissimilarity& score,
                                        int& fail_index) const
{
  GraspTemplate sample((*candidates_)[candidate]);
  lib_match_handler_[lib_index].applyDcMask(sample);

  fail_index = -1;

  if (lib_failures_ != NULL)
  {
    for (unsigned int i = 0; i < (*lib_failures_)[lib_index].size(); i++)
    {
      GraspTemplate f_templt((*lib_failures_)[lib_index][i].grasp_template,
                             (*lib_failures_)[lib_index][i].template_pose.pose);
      lib_match_handler_[lib_index].applyDcMask(f_templt);

      TemplateDissimilarity cur = lib_match_handler_[lib_index].getScore(sample, f_templt);
      if (i == 0 || cur.isBetter(cur, score))
      {
        score = cur;
        fail_index = i;
      }
    }
  }
}

double TemplateMatching::computeScore(double a, double b, double c) const
{
  if (b >= 0)
  {
    b = 1 - exp(-learningFailDistFac() * b * b);
  }
  else
    b = 1;

  if (c >= 0)
  {
    c = 1 - exp(-learningLibQualFac() * c * c);
  }
  else
    c = 1;

  if (b < 0.0001)
    b = 0.0001;
  if (c < 0.0001)
    c = 0.0001;

  return a / b / c;
}

double TemplateMatching::computeScore(unsigned int cand) const
{
  const double a = lib_scores_[cand].getScore(); //m(c, l) is to minimize!

  double b; //m(c, f); is to be maximized!
  if (candidate_to_fail_[cand] >= 0)
  {
    b = fail_scores_[cand].getScore();
    b = 1 - exp(-learningFailDistFac() * b * b);
  }
  else
    b = 1;

  double c; //m(l, f); is to be maximized!
  if (lib_to_fail_[candidate_to_lib_[cand]] >= 0)
  {
    c = lib_qualities_[candidate_to_lib_[cand]].getScore();
    c = 1 - exp(-learningLibQualFac() * c * c);
  }
  else
    c = 1;

  return a / b / c;
}

double TemplateMatching::getLibOverlay(unsigned int rank) const
{
  return lib_scores_[ranking_[rank]].getMinOverlay();
}

} //namespace
