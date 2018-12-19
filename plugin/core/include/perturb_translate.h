
#ifndef FEASST_CORE_PERTURB_TRANSLATE_H_
#define FEASST_CORE_PERTURB_TRANSLATE_H_

#include "core/include/perturb.h"

namespace feasst {

class PerturbTranslate : public Perturb {
 public:
  const Select& selection() const override { return selection_; }

  void select_random_particle(const int group_index, const Configuration& config) {
    ASSERT(optimization_ == 1, "error");
    selection_.random_particle(config, group_index);
  }

  void translate_selected_particle(const Position &trajectory,
    System * system) {
    store_old(system);
    Configuration* config = system->configuration(0);
    config->displace_particles(selection_, trajectory);
    set_revert_possible();
  }

  void revert() {
    if (revert_possible()) {
      if (optimization_ == 0) {
        Perturb::revert();
      } else {
        Configuration* config = system()->configuration(0);
        config->update_positions(selection_);
      }
    }
  }

  ~PerturbTranslate() {}

 private:
  SelectList selection_;
};

}  // namespace feasst

#endif  // FEASST_CORE_PERTURB_TRANSLATE_H_