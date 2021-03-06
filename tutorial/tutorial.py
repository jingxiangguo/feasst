import feasst

monte_carlo = feasst.MonteCarlo()
monte_carlo.set(feasst.MakeRandomMT19937(feasst.args({"seed" : "time"})))
monte_carlo.add(feasst.Configuration(feasst.MakeDomain(feasst.args({"cubic_box_length": "8"})),
     feasst.args({"particle_type": feasst.install_dir() + "/forcefield/data.lj"})))
monte_carlo.add(feasst.Potential(feasst.MakeLennardJones()))
monte_carlo.add(feasst.Potential(feasst.MakeLongRangeCorrections()))
monte_carlo.add(feasst.MakeMetropolis(feasst.args({"beta": "1.5"})))
monte_carlo.add(feasst.MakeTrialTranslate(feasst.args(
    {"tunable_param": "2.", "tunable_target_acceptance": "0.2"})))
steps_per = int(1e3)
monte_carlo.add(feasst.MakeTuner(feasst.args({"steps_per" : str(steps_per)})))
feasst.SeekNumParticles(50)\
    .with_metropolis(feasst.args({"beta": "0.1", "chemical_potential": "10"}))\
    .with_trial_add().run(monte_carlo)
monte_carlo.add(feasst.MakeLog(feasst.args({"steps_per" : str(steps_per)})))
monte_carlo.add(feasst.MakeMovie(feasst.args(
    {"steps_per" : str(steps_per), "file_name" : "movie.xyz"})))
monte_carlo.add(feasst.MakeCheckEnergy(feasst.args(
    {"steps_per" : str(steps_per), "tolerance" : "1e-8"})))
monte_carlo.attempt(int(1e5))
