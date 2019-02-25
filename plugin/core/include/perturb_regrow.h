
#ifndef FEASST_CORE_PERTURB_REGROW_H_
#define FEASST_CORE_PERTURB_REGROW_H_

#include "core/include/perturb_translate.h"

namespace feasst {

/**
 */
class PerturbRegrow : public PerturbSelectMove {
 public:
  // assumes selection ends with site which is connected to the rest of the
  // particle that isn't being regrown, or and end-site, if regrowing entire
  // segment.
  void regrow(System * system) {
    Configuration * config = get_config_before_move(system);
    SelectList bonded = selection();
    int bonded_to = static_cast<int>(bonded.site_indices()[0].size()) - 1;
    DEBUG("regrowing sites " << bonded.str());
    for (int select_index = bonded_to - 1;
         select_index >= 0;
         --select_index) {
      rebond_(select_index, bonded_to, system, &bonded);
      bonded_to = select_index;
    }

    // recenter particle position
    bonded.set_particle_position(0,
      system->configuration().select_particle(0).average_site_position());

    config->update_positions(bonded);
    after_move();
  }

  ~PerturbRegrow() {}

 private:
  Random random_;

  void rebond_(const int site_to_update, const int site_bonded_to, System * system, SelectList * bonded) {
    // obtain the bond length
    const Configuration& config = system->configuration();
    const Bond& bond = selection().bond(0, site_to_update, site_bonded_to, config);
    const double l0 = bond.property("l0");

    // obtain a new position for the site to update
    const Position& anchor = bonded->site_positions()[0][site_bonded_to];
    Position rebond = anchor;
    random_.unit_sphere_surface(&rebond);
    rebond.multiply(l0);
    rebond.add(anchor);

    bonded->set_site_position(0, site_to_update, rebond);
  }
};

}  // namespace feasst

#endif  // FEASST_CORE_PERTURB_REGROW_H_
