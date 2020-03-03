
#ifndef FEASST_EWALD_SYSTEM_EXAMPLE_H_
#define FEASST_EWALD_SYSTEM_EXAMPLE_H_

#include "configuration/test/configuration_test.h"
#include "system/include/system.h"
#include "system/include/lennard_jones.h"
#include "system/include/long_range_corrections.h"
#include "system/include/potential.h"
#include "system/include/model_two_body_factory.h"
#include "system/include/visit_model_intra.h"
#include "system/include/visit_model_bond.h"
#include "ewald/include/ewald.h"
#include "ewald/include/charge_screened.h"
#include "ewald/include/charge_screened_intra.h"
#include "ewald/include/charge_self.h"

namespace feasst {

inline System spce(const std::string physical_constants = "") {
  Configuration config = spce_sample();
//  config.add_model_param("alpha", 5.6/config.domain()->min_side_length());
  if (!physical_constants.empty()) {
    if (physical_constants == "CODATA2010") {
      config.set_physical_constants(MakeCODATA2010());
    } else if (physical_constants == "CODATA2018") {
      config.set_physical_constants(MakeCODATA2018());
    } else {
      ERROR("unrecognized");
    }
  }
  System sys;
  sys.add(config);
  sys.add(Potential(
    MakeEwald({{"kmax_squared", "27"},
               {"alpha", str(5.6/config.domain()->min_side_length())}}),
                     {{"prevent_cache", "true"}}));
  sys.add(Potential(MakeModelTwoBodyFactory({MakeLennardJones(),
                                             MakeChargeScreened()})));
  //sys.add(Potential(MakeChargeScreenedIntra(),
  //                   MakeVisitModelIntra({{"cutoff", "0"}})));
  sys.add(Potential(MakeChargeScreenedIntra(), MakeVisitModelBond()));
  sys.add(Potential(MakeChargeSelf()));
  sys.add(Potential(MakeLongRangeCorrections()));
  sys.precompute(); // HWH don't need this precompute?
  return sys;
}

inline System chain(const double alpha,
                    const int kmax_squared) {
  System system;
  { Configuration config(MakeDomain({{"cubic_box_length", "20"}}), {
      {"particle_type0", "../forcefield/data.chain10titratable"},
      {"particle_type1", "../plugin/ewald/forcefield/data.rpm_minus"},
      {"particle_type2", "../plugin/ewald/forcefield/data.rpm_plus"},
    });
//    config.add_particle_of_type(0);
    config.add_particle_of_type(1);
    config.add_particle_of_type(2);
    config.update_positions({{0, 0, 0}, {2, 0, 0}});
    double alpha2;
    int kxmax, kymax, kzmax;
    Ewald().tolerance_to_alpha_ks(0.0001, config, &alpha2, &kxmax, &kymax, &kzmax);
    DEBUG("alpha2 " << alpha2);
//    const double rms = Ewald().fourier_rms(alpha2, 3, config, 0);
//    DEBUG("rms0 " << rms);
    DEBUG("kxmax " << kxmax);
    config.add_model_param("alpha", alpha);
    system.add(config);
  }
  auto ewald= MakeEwald({{"kmax_squared", "27"},
               {"alpha", str(5.6/system.configuration().domain()->min_side_length())}});
  system.add(Potential(ewald,
                     {{"prevent_cache", "true"}}));
  system.add(Potential(MakeModelTwoBodyFactory({MakeLennardJones(),
                                                MakeChargeScreened()})));
  //system.add(Potential(MakeChargeScreenedIntra(),
  //                     MakeVisitModelIntra({{"cutoff", "0"}})));
  system.add(Potential(MakeChargeScreenedIntra(), MakeVisitModelBond()));
  system.add(Potential(MakeChargeSelf()));
//  system.add(Potential(MakeLongRangeCorrections()));
//  auto ewald = add_ewald_with(MakeLennardJones(), &system, kmax_squared);
  system.add(Potential(MakeLennardJones(), MakeVisitModelIntra({{"cutoff", "1"}})));
  DEBUG("kxmax " << ewald->kxmax());
  DEBUG("kymax " << ewald->kymax());
  DEBUG("kzmax " << ewald->kzmax());
  DEBUG("num_vectors " << ewald->num_vectors());
  return system;
}

inline void test_cases(
    /// tuple of constants and expected energy values
    std::vector<std::tuple<std::shared_ptr<PhysicalConstants>, double> > cases,
    std::shared_ptr<Model> model,
    std::shared_ptr<VisitModel> visitor = MakeVisitModel()) {
  for (auto cse : cases) {
    INFO(std::get<0>(cse)->class_name());
    Configuration config = spce_sample();
    config.add_model_param("alpha", 5.6/config.domain()->min_side_length());
    config.set_physical_constants(std::get<0>(cse));
    Potential potential(model, visitor);
    potential.precompute(&config);
    EXPECT_NEAR(std::get<1>(cse), potential.energy(&config), 1e-10);
    Potential potential2 = test_serialize(potential);
    EXPECT_NEAR(std::get<1>(cse), potential2.energy(&config), 1e-10);
  }
}

}  // namespace feasst

#endif  // FEASST_EWALD_SYSTEM_EXAMPLE_H_
