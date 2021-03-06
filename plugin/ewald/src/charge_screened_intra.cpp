#include <cmath>
#include "ewald/include/charge_screened_intra.h"
#include "utils/include/serialize.h"

namespace feasst {

class MapChargeScreenedIntra {
 public:
  MapChargeScreenedIntra() {
    ChargeScreenedIntra().deserialize_map()["ChargeScreenedIntra"] = MakeChargeScreenedIntra();
  }
};

static MapChargeScreenedIntra map_charge_screened_intra_ = MapChargeScreenedIntra();

void ChargeScreenedIntra::serialize(std::ostream& ostr) const {
  ostr << class_name_ << " ";
  feasst_serialize_version(304, ostr);
  feasst_serialize(alpha_, ostr);
  feasst_serialize(conversion_factor_, ostr);
}

ChargeScreenedIntra::ChargeScreenedIntra(std::istream& istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 304, "unrecognized verison: " << version);
  feasst_deserialize(&alpha_, istr);
  feasst_deserialize(&conversion_factor_, istr);
}

double ChargeScreenedIntra::energy(
    const double squared_distance,
    const int type1,
    const int type2,
    const ModelParams& model_params) const {
  const double mixed_charge = model_params.mixed_charge()[type1][type2];
  const double distance = std::sqrt(squared_distance);
  return -mixed_charge*conversion_factor_*erf(alpha_*distance)/distance;
}

void ChargeScreenedIntra::precompute(const ModelParams& existing) {
  alpha_ = existing.property("alpha");
  conversion_factor_ = existing.constants().charge_conversion();
}

}  // namespace feasst
