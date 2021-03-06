
#ifndef FEASST_CLUSTER_TRIAL_REMOVE_AVB_H_
#define FEASST_CLUSTER_TRIAL_REMOVE_AVB_H_

#include <memory>
#include "utils/include/arguments.h"
#include "monte_carlo/include/trial.h"
#include "system/include/neighbor_criteria.h"

namespace feasst {

/// Attempt to remove a particle.
class TrialRemoveAVB : public Trial {
 public:
  TrialRemoveAVB(
    std::shared_ptr<NeighborCriteria> neighbor_criteria,
    const argtype& args = argtype());
  std::shared_ptr<Trial> create(std::istream& istr) const override;
  void serialize(std::ostream& ostr) const override;
  explicit TrialRemoveAVB(std::istream& istr);
  virtual ~TrialRemoveAVB() {}

 protected:
  void serialize_trial_remove_avb_(std::ostream& ostr) const;
};

inline std::shared_ptr<TrialRemoveAVB> MakeTrialRemoveAVB(
    std::shared_ptr<NeighborCriteria> neighbor_criteria,
    const argtype &args = argtype()) {
  return std::make_shared<TrialRemoveAVB>(neighbor_criteria, args);
}

}  // namespace feasst

#endif  // FEASST_CLUSTER_TRIAL_REMOVE_AVB_H_
