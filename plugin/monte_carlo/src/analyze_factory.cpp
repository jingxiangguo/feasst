#include "monte_carlo/include/analyze_factory.h"
#include "utils/include/serialize.h"

namespace feasst {

class MapAnalyzeFactory {
 public:
  MapAnalyzeFactory() {
    AnalyzeFactory().deserialize_map()["AnalyzeFactory"] =
      std::make_shared<AnalyzeFactory>();
  }
};

static MapAnalyzeFactory mapper_ = MapAnalyzeFactory();

void AnalyzeFactory::initialize(Criteria * criteria,
    System * system,
    TrialFactory * trial_factory) {
  for (std::shared_ptr<Analyze> analyze : analyzers_) {
    analyze->initialize(criteria, system, trial_factory);
  }
}

void AnalyzeFactory::trial(const Criteria& criteria,
    const System& system,
    const TrialFactory& trial_factory) {
  DEBUG("multistate? " << is_multistate() << " class? " << class_name());
  if (is_multistate()) {
    DEBUG("state? " << criteria.state());
    trial_(criteria, system, trial_factory, criteria.state());
  } else {
    for (int index = 0; index < static_cast<int>(analyzers_.size()); ++index) {
//    for (const std::shared_ptr<Analyze> analyze : analyzers_) {
      DEBUG("index " << index);
      trial_(criteria, system, trial_factory, index);
    }
  }
}

void AnalyzeFactory::trial_(const Criteria& criteria,
    const System& system,
    const TrialFactory& trial_factory,
    const int index) {
  // timer_.start(index + 1);
  DEBUG("index " << index << " sz " << analyzers_.size());
  ASSERT(index < static_cast<int>(analyzers_.size()),
    "index: " << index << " too large when " << analyzers_.size() << " ans");
  DEBUG(analyzers_[index]->class_name());
  analyzers_[index]->trial(criteria, system, trial_factory);
  // timer_.end();
}

AnalyzeFactory::AnalyzeFactory(std::istream& istr) : Analyze(istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 1640, "unrecognized verison: " << version);
  // feasst_deserialize_fstdr(&analyzers_, istr);
  // HWH for unknown reasons, function template doesn't work
  int dim1;
  istr >> dim1;
  analyzers_.resize(dim1);
  for (int index = 0; index < dim1; ++index) {
    int existing;
    istr >> existing;
    if (existing != 0) {
      analyzers_[index] = analyzers_[index]->deserialize(istr);
    }
  }
}

void AnalyzeFactory::serialize(std::ostream& ostr) const {
  Stepper::serialize(ostr);
  feasst_serialize_version(1640, ostr);
  feasst_serialize_fstdr(analyzers_, ostr);
}

}  // namespace feasst
