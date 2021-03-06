#include <cmath>
#include <algorithm>
#include <memory>
#include "utils/include/serialize.h"
#include "confinement/include/shape.h"
#include "utils/include/debug.h"

namespace feasst {

std::map<std::string, std::shared_ptr<Shape> >& Shape::deserialize_map() {
  static std::map<std::string, std::shared_ptr<Shape> >* ans =
     new std::map<std::string, std::shared_ptr<Shape> >();
  return *ans;
}

bool Shape::is_inside(const Position& point) const {
  DEBUG(nearest_distance(point));
  if (nearest_distance(point) < 0) {
    return true;
  }
  return false;
}

bool Shape::is_inside(const Position& point, const double diameter) const {
  if (nearest_distance(point) + 0.5*diameter < 0) {
    return true;
  }
  return false;
}

void Shape::serialize(std::ostream& ostr) const { FATAL("not implemented"); }

std::shared_ptr<Shape> Shape::create(std::istream& istr) const {
  FATAL("not implemented");
}

std::shared_ptr<Shape> Shape::deserialize(std::istream& istr) {
  return template_deserialize(deserialize_map(), istr);
}

class MapShapeIntersect {
 public:
  MapShapeIntersect() {
    ShapeIntersect().deserialize_map()["ShapeIntersect"] = MakeShapeIntersect();
  }
};

static MapShapeIntersect mapper_ = MapShapeIntersect();

ShapeIntersect::ShapeIntersect(
    std::shared_ptr<Shape> shape1,
    std::shared_ptr<Shape> shape2) {
  shape1_ = shape1;
  shape2_ = shape2;
}

double ShapeIntersect::nearest_distance(const Position& point) const {
  const double dist1 = shape1_->nearest_distance(point),
               dist2 = shape2_->nearest_distance(point);
  if (dist1 > dist2) {
    return dist1;
  } else {
    return dist2;
  }
}

void ShapeIntersect::serialize(std::ostream& ostr) const {
  ostr << class_name_ << " ";
  feasst_serialize_version(822, ostr);
  feasst_serialize_fstdr(shape1_, ostr);
  feasst_serialize_fstdr(shape2_, ostr);
}

ShapeIntersect::ShapeIntersect(std::istream& istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(822 == version, version);

  // HWH for unknown reasons, below template isn't working in this case.
  // feasst_deserialize_fstdr(shape1, istr);
  // feasst_deserialize_fstdr(shape2, istr);
  int existing;
  istr >> existing;
  if (existing != 0) {
    shape1_ = shape1_->deserialize(istr);
  }
  istr >> existing;
  if (existing != 0) {
    shape2_ = shape2_->deserialize(istr);
  }
}

class MapShapeUnion {
 public:
  MapShapeUnion() {
    ShapeUnion().deserialize_map()["ShapeUnion"] = std::make_shared<ShapeUnion>();
  }
};

static MapShapeUnion mapper_shape_union_ = MapShapeUnion();

ShapeUnion::ShapeUnion(
    std::shared_ptr<Shape> shape1,
    std::shared_ptr<Shape> shape2) {
  shape1_ = shape1;
  shape2_ = shape2;
}

double ShapeUnion::nearest_distance(const Position& point) const {
  const double dist1 = shape1_->nearest_distance(point),
               dist2 = shape2_->nearest_distance(point);
  if (dist1 < dist2) {
    return dist1;
  } else {
    return dist2;
  }
}

void ShapeUnion::serialize(std::ostream& ostr) const {
  ostr << class_name_ << " ";
  feasst_serialize_version(172, ostr);
  feasst_serialize_fstdr(shape1_, ostr);
  feasst_serialize_fstdr(shape2_, ostr);
}

ShapeUnion::ShapeUnion(std::istream& istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(172 == version, version);

  // HWH for unknown reasons, below template isn't working in this case.
  // feasst_deserialize_fstdr(shape1, istr);
  // feasst_deserialize_fstdr(shape2, istr);
  int existing;
  istr >> existing;
  if (existing != 0) {
    shape1_ = shape1_->deserialize(istr);
  }
  istr >> existing;
  if (existing != 0) {
    shape2_ = shape2_->deserialize(istr);
  }
}

void ShapedEntity::serialize(std::ostream& ostr) const {
  feasst_serialize_version(9249, ostr);
  feasst_serialize_fstdr(shape_, ostr);
}

ShapedEntity::ShapedEntity(std::istream& istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 9249, "unrecognized verison: " << version);
  // feasst_deserialize_fstdr(shape_, istr);
  { // HWH for unknown reasons the above template function does not work
    int existing;
    istr >> existing;
    if (existing != 0) {
      shape_ = shape_->deserialize(istr);
    }
  }
}

}  // namespace feasst
