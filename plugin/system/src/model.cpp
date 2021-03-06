#include "system/include/model.h"
#include "utils/include/serialize.h"

namespace feasst {

std::map<std::string, std::shared_ptr<Model> >& Model::deserialize_map() {
  static std::map<std::string, std::shared_ptr<Model> >* ans =
     new std::map<std::string, std::shared_ptr<Model> >();
  return *ans;
}

std::shared_ptr<Model> Model::deserialize(std::istream& istr) {
  return template_deserialize(deserialize_map(), istr);
}

double Model::energy(
  const double squared_distance,
  const int type1,
  const int type2,
  const ModelParams& model_params) const { FATAL("not implemented"); }

void Model::serialize(std::ostream& ostr) const { FATAL("not implemented"); }

std::shared_ptr<Model> Model::create(std::istream& istr) const {
  FATAL("not implemented"); }

}  // namespace feasst
