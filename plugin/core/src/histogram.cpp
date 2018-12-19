
#include "core/include/histogram.h"
#include "core/include/debug.h"
#include "core/include/formula_polynomial.h"
#include "core/include/utils_math.h"

namespace feasst {

void Histogram::set_bin_size(const std::shared_ptr<Formula> bin_size) {
  set_expandable_();
  bin_size_ = bin_size;
  ASSERT(edges_.size() == 0 && size() == 0, "formula cannot be changed");
  histogram_.push_back(0.);
  const double fabove = bin_size_->evaluate(1);
  const double f0 = bin_size_->evaluate(0);
  const double fbelow = bin_size_->evaluate(-1);
  edges_.push_back(0.5*(fbelow + f0));
  edges_.push_back(0.5*(fabove + f0));
}

void Histogram::set_width_center(const double width, const double center) {
  auto bin_size = std::make_shared<FormulaPolynomial>();
  bin_size->set_x0(0);
  bin_size->set_A(0, center).set_A(1, width);
  set_bin_size(bin_size);
  is_constant_width_ = 1;
}

void Histogram::set_edges(const std::deque<double> edges) {
  edges_ = edges;
  set_not_expandable();
}
int Histogram::bin(const double value) const {
  ASSERT(value <= max() && value >= min(), "out of range");
  if (is_constant_width_ == 1) {
    const double kWidth = edges_[1] - edges_[0];
    return round((value - center_of_bin(0))/kWidth);
  }
  int bin = 0;
  bool found = false;
  while (!found && bin < size()) {
    if (value < edges_[bin + 1]) {
      found = true;
    }
    ++bin;
  }
  ASSERT(found == true, "out of range");
  return bin;
}

double Histogram::center_of_bin(const int bin) const {
  ASSERT(bin < size(), "size error");
  return 0.5*(edges_[bin] + edges_[bin + 1]);
}

void Histogram::add(const double value) {
  // initialize histogram if not already and formula is set
  ASSERT(edges_.size() != 0, "size error");
  if ( (value <= max()) && (value >= min()) ) {
    ++histogram_[bin(value)];
  } else {
    // expand histogram if allowed and formula is available.
    ASSERT(bin_size_ != NULL && expandable_, "out of range");
    int increment = 0;
    const int kMaxIncrements = 1e3;
    int found = 0;
    if (value > max()) {
      while (found == 0 && increment < kMaxIncrements) {
        const double new_width = 2*(bin_size_->evaluate(size()) - edges_.back());
        edges_.push_back(edges_.back() + new_width);
        if (value < max()) {
          found = 1;
          histogram_.push_back(1);
        } else {
          histogram_.push_back(0);
        }
        ++increment;
      }
      ASSERT(increment != kMaxIncrements, "size error");
    } else if (value < min()) {
      while (found == 0 && increment < kMaxIncrements) {
        const double new_width = 2*(edges_.front() - bin_size_->evaluate(-1));
        edges_.push_front(edges_.front() - new_width);
        if (value > min()) {
          found = 1;
          histogram_.push_front(1);
        } else {
          histogram_.push_front(0);
        }
        bin_size_->set_x0(bin_size_->x0() + 1);
        ++increment;
      }
      ASSERT(increment != kMaxIncrements, "size error");
    } else {
      ASSERT(increment != -1, "strange range");
    }
  }
}

}  // namespace feasst