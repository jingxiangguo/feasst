
#include <fstream>
#include <sstream>
#include "core/include/file_xyz.h"
#include "core/include/utils_io.h"
#include "core/include/debug.h"
#include "core/include/visit_configuration.h"

namespace feasst {

void FileXYZ::load(const char* file_name, Configuration * config) const {
  std::ifstream xyz_file(file_name);
  int num_sites;
  std::string line;
  std::getline(xyz_file, line);
  { std::stringstream iss(line);
    iss >> num_sites; }
  std::getline(xyz_file, line);
  std::vector<double> coord(3);
  { std::stringstream iss(line);
    double id;
    iss >> id >> coord[0] >> coord[1] >> coord[2]; }
  //cout << "line " << line << " is " << iss.str() << endl;
//      cout << "cord " << str(coord) << endl;
  Position position;
  position.set_vector(coord);
  DomainCuboid domain = config->domain();
  domain.set_side_length(position);
  config->set_domain(domain);
  //config->domain.set_side_length(position);
  if (config->num_sites() == 0) {
    if (config->num_particle_types() == 0) {
      Particle particle;
      particle.default_particle();
      config->add_particle_type("../forcefield/data.atom");
    }
    for (int site_index = 0; site_index < num_sites; ++site_index) {
      config->add_particle(0);
    }
  }

  /// read the coordinates into a 2d vector
  std::vector<std::vector<double> > coords;
  coords.resize(num_sites, std::vector<double>(config->dimension()));
  for (int i = 0; i < num_sites; ++i) {
    std::getline(xyz_file, line);
    std::istringstream iss(line);
    std::string tmp;
    iss >> tmp;
    for (int dim = 0; dim < config->dimension(); ++dim) {
      iss >> coords[i][dim];
    }
    DEBUG("coord " << coords[i][0]);
  }
  config->update_positions(coords);
}

// Utility class to print XYZ files from selection.
class PrinterXYZ : public LoopOneBody {
 public:
  PrinterXYZ(std::shared_ptr<std::ofstream> file) : file_(file) {}
  void work(const Site& site) const override {
    (*file_.get()) << "H " << site.position().str() << endl;
  }
 private:
  std::shared_ptr<std::ofstream> file_;
};

void FileXYZ::write(const char* file_name,
                    const Configuration& config,
                    int group_index) const {
  auto file = std::make_shared<std::ofstream>(file_name);
  (*file.get()) << config.num_sites() << endl
       << "-1" << endl;
  PrinterXYZ printer(file);
  VisitConfiguration().loop(config, &printer, group_index);
}

}  // namespace feasst