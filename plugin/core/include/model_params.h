
#ifndef FEASST_CORE_MODEL_PARAMS_H_
#define FEASST_CORE_MODEL_PARAMS_H_

#include <memory>
#include <vector>
#include <cmath>
#include <string>
#include "core/include/utils.h"
#include "core/include/particle.h"

namespace feasst {

class ModelParams;

/**
  Model parameters depend upon site types, such as epsilon, sigma, etc.
  These parameters may also be mixed between two different site types.
  Each model parameter has an assumed mixing behavior.
 */
class ModelParam {
 public:
  /// Add a new site type.
  void add(const double value);

  /// Set the value of the site type.
  void set(const int site_type, const double value) {
    values_[site_type] = value;
  }

  /// Add a new site type. If the site does not contain the model parameter,
  /// then add the default value.
  void add(const Site site, const double default_value = 0.);

  /// Add all site types in particle.
  void add(const Particle particle);

  /// Compute the mixed values.
  void mix();

  /// Set the mixed value of the site types.
  void set_mixed(const int site_type1, const int site_type2, const double value) {
    mixed_values_[site_type1][site_type2] = value;
    is_mixed_override_ = true;
  }

  /// Return the value.
  double value(const int type) const;

  /// Return the values.
  const std::vector<double>& values() const { return values_; }

  /// Return the mixed value.
  double mixed_value(const int type1, const int type2) const {
    return mixed_values_[type1][type2];
  }

  /// Return the mixed values.
  const std::vector<std::vector<double> >& mixed_values() const {
    return mixed_values_;
  }

  /// Return the number of values.
  int size() const { return static_cast<int>(values_.size()); }

  /// Return the name of the model parameter (e.g., epsilon, cutoff)
  std::string name() { return name_(); }

  /// Return the maximum.
  double max() const { return max_value_; }

  /// Return the mixed maximum.
  double mixed_max() const;

  virtual double compute(const int type1, const int type2,
    const ModelParams& model_params);

  /// Define new parameters modeled after the existing ones
  virtual void set_param(const ModelParams& existing);

  virtual ~ModelParam() {};

 private:
  std::vector<double> values_;
  std::vector<std::vector<double> > mixed_values_;
  double max_value_;
  double max_mixed_value_;

  /// Define mixing rules in the derived class.
  /// The default is a simple average.
  virtual double mix_(const double value1, const double value2) {
    return 0.5*(value1 + value2);
  }

  virtual std::string name_() { return std::string("generic"); }

  bool is_mixed_override_ = false;
};

/**
 The epsilon parameter is named "epsilon" in LMP-like data file Pair Coeffs.
 The epsilon parameter has the mixing rule:
 \f$ \epsilon_{ij} = \sqrt(\epsilon_i \epsilon_j) \f$
 */
class Epsilon : public ModelParam {
 private:
  double mix_(const double value1, const double value2) override {
    return std::sqrt(value1*value2);
  }
  std::string name_() override { return std::string("epsilon"); }
};

/**
 The sigma parameter is named "sigma" in LMP-like data file Pair Coeffs.
 The sigma parameter has the mixing rule:
 \f$ \sigma_{ij} = 0.5*(\sigma_i + \sigma_j) \f$
 */
class Sigma : public ModelParam {
 private:
  std::string name_() override { return std::string("sigma"); }
};

/**
 The cut off parameter is named "cutoff" in LMP-like data file Pair Coeffs.
 The cut off parameter has the mixing rule:
 \f$ r^c_{ij} = 0.5*(r^c_i + r^c_j) \f$
 */
class CutOff : public ModelParam {
 private:
  std::string name_() override { return std::string("cutoff"); }
};

/**
 The charge parameter is named "charge" in LMP-like data file Pair Coeffs.
 The charge parameter, q, has the mixing rule:
 \f$ q_{ij} = q_i q_j \f$
 */
class Charge : public ModelParam {
 private:
  double mix_(const double value1, const double value2) override {
    return value1*value2;
  }
  std::string name_() override { return std::string("charge"); }
};

/**
  Container for all model parameters.
 */
class ModelParams : public PropertiedEntity {
 public:
  ModelParams();

  /// Deep copy constructor
  ModelParams(const ModelParams& params);

  /// Add all site types in particle.
  void add(const Particle particle);

  /// Compute the mixed values.
  void mix();

  /// Return the number of values.
  int size() const;

  /// Modify model parameter of a given site type and name to value.
  void set(const char* name, const int site_type, const double value);

  /// Modify the mixed model parameter of a pair of given site types and name
  /// to value.
  void set(const char* name, const int site_type1, const int site_type2,
    const double value);

  /// Return model parameters of specific types.
  Epsilon epsilon() const { return *epsilon_; }
  Sigma sigma() const { return *sigma_; }
  CutOff cutoff() const { return *cutoff_; }
  Charge charge() const { return *charge_; }

  /// Return constant pointers for optimized model inner loops.
  const std::vector<std::vector<double> >& mixed_epsilon() const {
    return epsilon_->mixed_values();
  }
  const std::vector<std::vector<double> >& mixed_sigma() const {
    return sigma_->mixed_values();
  }
  const std::vector<std::vector<double> >& mixed_cutoff() const {
    return cutoff_->mixed_values();
  }
  const std::vector<std::vector<double> >& mixed_charge() const {
    return charge_->mixed_values();
  }

  /// Add a custom model parameter
  void add(std::shared_ptr<ModelParam> param) {
    params_.push_back(param);
  }

  /// Return the model parameter with the corresponding name.
  std::shared_ptr<ModelParam> select(const std::string name);

 private:
  /// All types of model parameters listed here.
  /// If adding new types, make sure to add them to params_ below.
  std::shared_ptr<Epsilon> epsilon_;
  std::shared_ptr<Sigma> sigma_;
  std::shared_ptr<CutOff> cutoff_;
  std::shared_ptr<Charge> charge_;

  /// Add all of the above to this list.
  std::vector<std::shared_ptr<ModelParam> > params_;

  /// Add built-in types to params
  void add_();
};

}  // namespace feasst

#endif  // FEASST_CORE_MODEL_PARAMS_H_
