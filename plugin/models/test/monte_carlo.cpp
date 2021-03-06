#include <memory>
#include "utils/test/utils.h"
#include "utils/include/utils_io.h"
#include "math/include/accumulator.h"
#include "math/include/random_mt19937.h"
#include "monte_carlo/include/trial.h"
#include "monte_carlo/include/trial_rotate.h"
#include "monte_carlo/include/monte_carlo.h"
#include "monte_carlo/test/monte_carlo_test.h"
#include "monte_carlo/include/metropolis.h"
#include "monte_carlo/include/utils.h"
#include "system/include/long_range_corrections.h"
#include "system/include/visit_model_intra.h"
#include "system/include/visit_model_cell.h"
#include "steppers/include/num_particles.h"
#include "models/include/lennard_jones_cut_shift.h"
#include "steppers/include/check_energy.h"

namespace feasst {

TEST(MonteCarlo, trimer) {
  MonteCarlo mc;
  mc.set(MakeRandomMT19937({{"seed", "123"}}));
  { { Configuration config(
        MakeDomain({{"cubic_box_length", "12"}}),
        {{"particle_type", "../forcefield/data.trimer"}});
      config.add_particle_of_type(0);
      config.add_particle_of_type(0);
      TrialSelectParticle sel;
      sel.select_particle(1, config);
      const Position disp = Position().set_vector({4, 4, 4});
      config.displace_particle(sel.mobile(), disp);
      mc.add(config);
    }

    auto lj_wca = MakeLennardJonesCutShift();
    ModelParams params = mc.system().configuration().model_params();
    lj_wca->set_wca(0, 1, &params);
    lj_wca->set_wca(1, 1, &params);
    //lj_wca->precompute(params);
    Potential potential(lj_wca);
    potential.set(params);
    mc.add(potential);
  }
  mc.set(MakeMetropolis({{"beta", "4"}, {"chemical_potential", "-1"}}));
  mc.add(MakeTrialTranslate({{"weight", "1."}, {"tunable_param", "1."}}));
  mc.add(MakeTrialRotate({{"weight", "1."}, {"tunable_param", "1."}}));
  add_trial_transfer(&mc, {{"particle_type", "0"}});
  mc.add(MakeCheckEnergy(
   {{"steps_per", "100"},
    {"tolerance", "1e-10"}}));
  mc.attempt(1e3);
}

}  // namespace feasst
