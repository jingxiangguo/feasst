#include <memory>
#include <gtest/gtest.h>
#include "core/include/trial_translate.h"
#include "core/include/trial_transfer.h"
#include "core/include/monte_carlo.h"
#include "core/include/criteria_metropolis.h"
#include "core/include/criteria_flat_histogram.h"
#include "core/include/macrostate_num_particles.h"
#include "core/include/bias_wang_landau.h"
#include "core/include/histogram.h"
#include "core/include/utils_io.h"
#include "core/include/accumulator.h"
#include "core/test/system_test.h"
#include "core/include/long_range_corrections.h"
#include "core/include/visit_model_intra.h"
#include "core/include/visit_model_cell.h"

namespace feasst {

class TestLJSystem {
 public:
  System system;
  TrialFactory trials;
  CriteriaMetropolis criteria;
  double add_probability = 0.5;
  double box_length = 8;
  double beta = 1./1.2;
  double activ = exp(-2.775);
  Configuration * config;
  MonteCarlo mc;

  void init() {
    system.add(Configuration());
    system.add_to_unoptimized(default_potential());
    //add_model(std::make_shared<ModelLJ>());
    config = system.get_configuration();
    config->add_particle_type("../forcefield/data.lj");
    auto transfer = std::make_shared<TrialTransfer>();
    transfer->set_weight(0.25);
    transfer->set_add_probability(add_probability);
    // trials.add(transfer);
    auto translate = std::make_shared<TrialTranslate>();
    translate->set_weight(0.75);
    trials.add(translate);
    config->set_domain(Domain().set_cubic(box_length));
    criteria.set_beta(beta);
    criteria.add_activity(activ);
    criteria.set_running_energy(system.energy());
    mc.set(std::make_shared<CriteriaMetropolis>(criteria));
    mc.set(system);
    // mc.add(transfer);
    mc.add(translate);
  }
};

TEST(MonteCarlo, NVTbenchmark) {
  seed_random_by_date();
  // seed_random(1346867550);
  TestLJSystem test;
  test.add_probability = 1.;
  test.activ = 100;
  test.init();
  test.mc.seek_num_particles(50);
  EXPECT_EQ(test.mc.system().configuration().num_particles(), 50);
  test.mc.attempt(1e4);
  // test.mc.attempt(1e6); // ~3-4 seconds
  EXPECT_NEAR(test.mc.get_system()->energy(), test.mc.get_criteria()->running_energy(), 1e-10);
}

TEST(MonteCarlo, NVTMC_SRSW) {
  seed_random_by_date();
  const double rho = 1e-3;
  const int nTrials = 1e3, nCheck = 1e3;
  //const int nTrials = 1e5, nCheck = 1e3;
  //const int nTrials = 1e7, nCheck = 1e3;
  const int nMol = 500;
  TestLJSystem test;
  test.beta = 1./0.9;
  test.add_probability = 1.;
  test.activ = 100;
  test.box_length = pow(double(nMol)/rho, 1./3.);
  test.init();
  Accumulator pe;
  test.mc.seek_num_particles(nMol);
  EXPECT_EQ(test.mc.system().configuration().num_particles(), 500);
  EXPECT_EQ(test.mc.trials().num_trials(), 1);
  for (int iTrial = 0; iTrial < 2*nTrials; ++iTrial) {
    test.mc.attempt();
    if (iTrial >= nTrials) {
      pe.accumulate(test.criteria.running_energy());
    }
    if (iTrial % nCheck == 0) {
      const int num = test.mc.system().configuration().num_particles();
      std::cout << "t" << iTrial
                << " pe " << pe.average()
                << " pe/n " << pe.average()/num
                << " n " << num
                << " vol " << test.config->domain().volume()
                << std::endl;
    }
  }
}

TEST(MonteCarlo, GCMCbenchmark) {
  // seed_random(1346867550);
  TestLJSystem test;
  test.activ = 100;
  test.init();
  const int nTrials = 1e3;
  // const int nTrials = 2e7;
  for (int iTrial = 0; iTrial < nTrials; ++iTrial) {
    test.trials.attempt(&test.criteria, &test.system);
    if (iTrial%100000==0) std::cout << "t" << iTrial << std::endl;
  }
}

TEST(MonteCarlo, MC) {
  seed_random_by_date();
  TestLJSystem test;
  const int nTrialsEq = 2e2, nTrials = 2e2;
  // const int nTrialsEq = 1e8, nTrials = 1e8;
  // const int nCheck = 1e2;
  const int nCheck = 1e4;
  const int nMol = 500;
  const double rho = 1e-3;
  test.box_length = pow(double(nMol)/rho, 1./3.);
  test.beta = 1./1.5;
  test.activ = exp(-2.775);
  test.init();
  EXPECT_EQ(1, test.config->num_particle_types());

  //system.set_running_energy(system.energy());
  int nTrans = 0, nIns = 0, nDel = 0;
  double peAcc = 0.;
  int inTrial = 0;
  for (int iTrial = 0; iTrial < nTrialsEq + nTrials; ++iTrial) {
    ++inTrial;
    peAcc += test.criteria.running_energy();
    if (iTrial == nTrialsEq) {
      peAcc = 0.;
      inTrial = 0;
    }
    if (iTrial % nCheck == 0) {
      std::cout << "n " << test.config->num_particles()
                << " pe " << test.criteria.running_energy()
                << " av " << peAcc/double(inTrial)
                << " peExpe " << -9.9165E-03*500 << std::endl;
      ASSERT(std::abs(test.system.energy() - test.criteria.running_energy()) < 1e-7, "mismatch");
    }
    test.trials.attempt(&test.criteria, &test.system);
  }
  std::cout << "nTrans " << nTrans << " nIns " << nIns << " nDel " << nDel << std::endl;
}

TEST(MonteCarlo, WLMC) {
  // seed_random();
  seed_random_by_date();
  System system;
  system.add(Configuration());
  system.add_to_unoptimized(default_potential());
  //system.add_model(std::make_shared<ModelLJ>());
  Configuration * config = system.get_configuration();
  config->add_particle_type("../forcefield/data.lj");

  // trials
  TrialFactory trials;
  auto transfer = std::make_shared<TrialTransfer>();
  transfer->set_weight(0.25);
  trials.add(transfer);
  //transfer.set_add_probability(1.);
  auto translate = std::make_shared<TrialTranslate>();
  translate->set_weight(1);
  trials.add(translate);

  const int nTrialsEq = 2e2, nTrials = 2e2;
  //const int nTrialsEq = 1e8, nTrials = 1e8;
  //const int nCheck = 1e6;
  const int nCheck = 1e2;
  const double boxl = 8;
  //const double rho = 1e-3, boxl = pow(double(nMol)/rho, 1./3.);
  config->set_domain(Domain().set_cubic(boxl));
  std::cout << "boxl " << boxl << std::endl;
  CriteriaFlatHistogram criteria;
  criteria.set_beta(1./1.5);
  criteria.add_activity(exp(-2.775));
  //criteria.add_activity(exp(-2.775));
  Histogram histogram;
  const int nMol = 5;
  histogram.set_width_center(1., 0.);
  for (int i = 0; i <= nMol; ++i) {
    histogram.add(i);
  }
  auto macrostate = std::make_shared<MacrostateNumParticles>();
  macrostate->set_histogram(histogram);
  criteria.set_macrostate(macrostate);
  auto bias = std::make_shared<BiasWangLandau>();
  bias->resize(histogram);
  criteria.set_bias(bias);

  criteria.set_running_energy(system.energy());
  double peAcc = 0.;
  int inTrial = 0;
  for (int iTrial = 0; iTrial < nTrialsEq + nTrials; ++iTrial) {
    ++inTrial;
    peAcc += criteria.running_energy();
    if (iTrial == nTrialsEq) {
      peAcc = 0.;
      inTrial = 0;
    }
    if (iTrial % nCheck == 0) {
      std::cout << "n " << config->num_particles() << " pe " << criteria.running_energy() << " av " << peAcc/double(inTrial) << " peExpe " << -9.9165E-03*500 << std::endl;
      ASSERT(std::abs(system.energy() - criteria.running_energy()) < 1e-7, "mismatch" <<
        system.energy() << " " << criteria.running_energy());
    }
    trials.attempt(&criteria, &system);
  }
  std::cout << "nTrans " << translate->num_attempts() << " nTransfer " << transfer->num_attempts() << std::endl;
  std::cout << str(bias->ln_macro_prob()) << std::endl;
}

TEST(MonteCarlo, Analyze) {
  seed_random_by_date();
  seed_random(1550461468);
  MonteCarlo mc;
  const double cutoff = 2.;

  { System system;
    { Configuration config;
      config.set_domain(Domain().set_cubic(12));
      // config.add_particle_type("../forcefield/data.lj");
      // config.add_particle_type("../forcefield/data.dimer");
      config.add_particle_type("../forcefield/data.chain10");
      system.add(config); }

    { Potential potential;
      potential.set_model(std::make_shared<ModelLJ>());
      potential.set_visit_model(std::make_shared<VisitModel>());
      potential.set_model_params(system.configuration());
      potential.set_model_param("cutoff", 0, cutoff);
      EXPECT_NEAR(potential.model_params().mixed_cutoff()[0][0], cutoff, NEAR_ZERO);
      system.add_to_unoptimized(potential); }

    { Potential potential;
      potential.set_model(std::make_shared<ModelHardSphere>());
      auto visitor = std::make_shared<VisitModelIntra>();
      visitor->set_intra_cut(1);
      potential.set_visit_model(visitor);
      potential.set_model_params(system.configuration());
      potential.set_model_param("cutoff", 0, 1.);
      system.add_to_unoptimized(potential); }

//    { Potential lrc;
//      lrc.set_visit_model(std::make_shared<LongRangeCorrections>());
//      lrc.set_model_params(system.configuration());
//      lrc.set_model_param("cutoff", 0, cutoff);
//      EXPECT_NEAR(lrc.model_params().mixed_cutoff()[0][0], cutoff, NEAR_ZERO);
//      system.add_to_unoptimized(lrc); }

    mc.set(system);
  }

  { auto criteria = std::make_shared<CriteriaMetropolis>();
    criteria->set_beta(1.2);
    criteria->add_activity(1.);
    mc.set(criteria); }

  { auto trial = std::make_shared<TrialTranslate>();
    trial->set_weight(1.);
    trial->set_max_move(1.);
    trial->set_max_move_bounds(mc.system().configuration().domain());
    mc.add(trial); }

  { auto trial = std::make_shared<TrialRotate>();
    trial->set_weight(1.);
    trial->set_max_move(90.);
    mc.add(trial); }

  { auto trial = std::make_shared<TrialPivot>();
    trial->set_weight(1.);
    trial->set_max_move(90.);
    mc.add(trial); }

  { auto trial = std::make_shared<TrialCrankShaft>();
    trial->set_weight(1.);
    trial->set_max_move(90.);
    trial->set_tunable_percent_change(0.1);
    mc.add(trial); }

  mc.seek_num_particles(1);
  const int num_periodic = 1e3;

  { auto log = std::make_shared<Log>();
    log->set_steps_per_write(num_periodic);
    log->set_file_name("tmp/log.txt");
    mc.add(log); }

  { auto movie = std::make_shared<Movie>();
    movie->set_steps_per(num_periodic);
    movie->set_file_name("tmp/chain10movie.xyz");
    mc.add(movie); }

  { auto checker = std::make_shared<EnergyCheck>();
    checker->set_steps_per_update(num_periodic);
    checker->set_tolerance(1e-10);
    mc.add(checker); }

  { auto tuner = std::make_shared<Tuner>();
    tuner->set_steps_per_update(num_periodic);
    mc.add(tuner); }

  { auto bondcheck = std::make_shared<RigidBondChecker>();
    bondcheck->set_steps_per(num_periodic);
    mc.add(bondcheck); }

  mc.attempt(1e4);
}

}  // namespace feasst