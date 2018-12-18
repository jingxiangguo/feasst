
#ifndef FEASST_CORE_SELECT_LIST_H_
#define FEASST_CORE_SELECT_LIST_H_

#include "core/include/configuration.h"

namespace feasst {


/**
  Similar to Select, except includes configurations as argument.
    Selects are for modifications of a subset of the configuration.
    Note that the requested selection may not be possible.
    In this case the selection remains empty.
  HWH: contain SelectPosition instead of inheriting from it to hide selection interface
 */
class SelectList : public SelectPosition {
 public:
  void particle(const int index,
                const Configuration& config,
                // By default, if group_index is 0, consider all particles
                const int group_index = 0) {
    // HWH optimize this
    clear();
    const SelectGroup& select = config.group_selects()[group_index];
    ASSERT(index < select.num_particles(), "error");
    add_particle(select.particle_index(index), select.site_indices(index));
    resize();
    load_positions(config.particles());
  }

  /// Add random particle in group index.
  virtual void random_particle(const Configuration& config,
      // By default, if group_index is 0, consider all particles
      const int group_index = 0,
      // By default, load coordinates. Set to zero if not needed.
      const int load_coordinates = 1) {
    // HWH optimize this
    clear();
    ASSERT(group_index >= 0, "error");
    const int num = config.num_particles(group_index);
    if (num > 0) {
      const int index = random()->uniform(0, num - 1);
      const SelectGroup& select = config.group_selects()[group_index];
      DEBUG("index " << group_index << " " << index);
      DEBUG("num " << select.num_particles());
      add_particle(select.particle_index(index), select.site_indices(index));
      if (load_coordinates == 1) {
        resize();
        load_positions(config.particles());
      }
    }
  }

  /// Add random particle of given type.
  void random_particle_of_type(const int type,
                               Configuration * config,
      // By default, load coordinates. Set to zero if not needed.
      const int load_coordinates = 1) {
    ASSERT(type < config->num_particle_types(),
      "particle type(" << type << ") doesn't exist");

    // do not select if no particles
    if (config->num_particles() == 0) {
      clear();
      return;
    }

    const int group_index = config->particle_type_to_group(type);
    // HWH is *config slow here?
    random_particle(*config, group_index, load_coordinates);
  }

  /// Select the last particle that was added to configuration.
  void last_particle_added(const Configuration * config) {
    // HWH optimize for add delete trial
    clear();
    add_particle(config->newest_particle(), config->newest_particle_index());
    resize();
    load_positions(config->particles());
  }

  /// Select sites.
  void select_sites(const Configuration& config,
                    const int particle_index,
                    const std::vector<int> site_indices) {
    clear();
    // filter the particle index.
    const int index = config.selection_of_all().particle_index(particle_index);
    add_sites(index, site_indices);
    resize();
    load_positions(config.particles());
  }

  /// Select a site.
  void select_site(const Configuration& config,
                   const int particle_index,
                   const int site_index) {
    select_sites(config, particle_index, {site_index});
  }

  /// Note: this method is relatively unoptimized compared with other options.
  virtual void add(const Configuration& config, const Group& group) {
    for (int index = 0; index < config.num_particles(); ++index) {
      const Particle& part = config.particle(index);
      if (group.is_in(part)) {
        Particle filtered = group.remove_sites(part);
        add_particle(filtered, index);
      }
    }
  }

  /// add random particle that is in group.
  /// Note: this method is relatively unoptimized compared with other options.
  virtual void add_random_particle(const Configuration& config, const Group& group) {
    add(config, group);
    reduce_to_random_particle();
  }

  /// Return the selected particle.
  const Particle& particle(const Configuration& config) {
    ASSERT(num_particles() == 1, "requires one selected particle");
    return config.select_particle(particle_index(0));
  }

 private:
};

}  // namespace feasst

#endif  // FEASST_CORE_SELECT_LIST_H_
