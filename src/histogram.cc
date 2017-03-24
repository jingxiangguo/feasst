/**
 * \file
 *
 * \brief
 */

#include "./histogram.h"

/**
 * Constructor
 */
Histogram::Histogram() {
  defaultConstruction();
}
Histogram::Histogram(const double binWidth) : binWidth_(binWidth) {
  defaultConstruction();
}
Histogram::Histogram(const double binWidth,
                     const int iType,
                     const int jType)
  : binWidth_(binWidth) {
  defaultConstruction();
  iType_ = iType;
  jType_ = jType;
}
Histogram::Histogram(const char* fileName) {
  ASSERT(myFileExists(fileName),
    "restart file(" << fileName << ") doesn't exist");
  defaultConstruction();
  binWidth_ = fstod("binWidth", fileName);
  max_ = fstod("max", fileName);
  min_ = fstod("min", fileName);
  nNorm_ = fstoll("nNorm", fileName);
  iType_ = fstoi("iType", fileName);
  jType_ = fstoi("jType", fileName);
  // centerZero_ = fstoi("centerZero", fileName);

  // compute the size of the histogram
  const int size = myRound((max_ - min_)/binWidth_);

  // cout << " open file and skip header lines" << endl;
  std::ifstream fs(fileName);
  string line;
  const int nLines = numLines(fileName);
  for (int i = 0; i < nLines - size; ++i) getline(fs, line);

  // read the histogram
  histogram_.resize(size);
  int tmp;
  for (int i = 0; i < size; ++i) {
    fs >> tmp >> tmp;
    histogram_[i] = tmp;
    getline(fs, line);
  }
}

/**
 * defaults in constructor
 */
void Histogram::defaultConstruction() {
  verbose_ = 0;
  className_.assign("Histogram");
  min_ = 0;
  max_ = 0;
  nNorm_ = 0;
  iType_ = 0;
  jType_ = 0;
  centerZero_ = 0;
}

/**
 * clone design pattern
 */
Histogram* Histogram::clone() const {
  Histogram* h = new Histogram(*this);
  return h;
}
shared_ptr<Histogram> Histogram::cloneShrPtr() const {
  shared_ptr<Histogram> h = make_shared<Histogram>(*this);
  return h;
}

/**
 * write restart file
 */
void Histogram::writeRestart(const char* fileName) {
  std::ofstream file(fileName);
  file << "# binWidth " << binWidth_ << endl;
  file << "# max " << max_ << endl;
  file << "# min " << min_ << endl;
  file << "# nNorm " << nNorm_ << endl;
  file << "# iType " << iType_ << endl;
  file << "# jType " << jType_ << endl;
//  file << "# centerZero " << centerZero_ << endl;
  for (int i = 0; i < size(); ++i) {
    file << i << " " << histogram_[i] << endl;
  }
}

/**
 * accumulate values
 */
void Histogram::accumulate(const double value) {
  // if first time accumulating, initialize
  //  two methods are available:
  //  1. center bins such that zero lies on boundary between two bins
  //  (centerZero = 0)
  //  2. center bin such that zero lies in middle of one bin (centerZero = 1)
  if ( (min_ == 0) && (max_ == 0) ) {
    if (value >= 0) {
      if (centerZero_ == 0) {
        min_ = static_cast<int>(value/binWidth_)*binWidth_;
        max_ = static_cast<int>(value/binWidth_ + 1)*binWidth_;
      } else {
        min_ = static_cast<int>(value/binWidth_)*binWidth_ - 0.5*binWidth_;
        max_ = static_cast<int>(value/binWidth_ + 1)*binWidth_ - 0.5*binWidth_;
      }
    } else {
      if (centerZero_ == 0) {
        min_ = static_cast<int>(value/binWidth_ - 1)*binWidth_;
        max_ = static_cast<int>(value/binWidth_)*binWidth_;
      } else {
        min_ = static_cast<int>(value/binWidth_ - 1)*binWidth_ + 0.5*binWidth_;
        max_ = static_cast<int>(value/binWidth_)*binWidth_ + 0.5*binWidth_;
      }
    }
    histogram_.clear();
    histogram_.push_back(1);

  // if value is above max, resize histogram
  } else if (value > max_) {
    const double diff = value - max_;
    const int binInc = static_cast<int>(diff/binWidth_+1);
    if (value >= 0) {
      if (centerZero_ == 0) {
        max_ = static_cast<int>(value/binWidth_ + 1)*binWidth_;
      } else {
        max_ = static_cast<int>(value/binWidth_ + 1)*binWidth_ - 0.5*binWidth_;
      }
    } else {
      if (centerZero_ == 0) {
        max_ = static_cast<int>(value/binWidth_)*binWidth_;
      } else {
        max_ = static_cast<int>(value/binWidth_)*binWidth_ + 0.5*binWidth_;
      }
    }
    for (int i = 0; i < binInc - 1; ++i) histogram_.push_back(0);
    histogram_.push_back(1);

  // if value is below min, resize histogram
  } else if (value < min_) {
    const double diff = min_ - value;
    const int binInc = static_cast<int>(diff/binWidth_+1);
    if (value >= 0) {
      if (centerZero_ == 0) {
        min_ = static_cast<int>(value/binWidth_)*binWidth_;
      } else {
        min_ = static_cast<int>(value/binWidth_)*binWidth_ - 0.5*binWidth_;
      }
    } else {
      if (centerZero_ == 0) {
        min_ = static_cast<int>(value/binWidth_ - 1)*binWidth_;
      } else {
        min_ = static_cast<int>(value/binWidth_ - 1)*binWidth_ + 0.5*binWidth_;
      }
    }
    for (int i = 0; i < binInc - 1; ++i) histogram_.push_front(0);
    histogram_.push_front(1);

  // otherwise, simply bin the histogram
  } else {
    ++histogram_[bin(value)];
  }
}

/**
 * return index of maximum element in histogram
 */
int Histogram::maxElementBin() const {
  return std::distance(histogram_.begin(), max_element(histogram_.begin(),
                       histogram_.end()));
}

/**
 * center the histogram on zero
 */
void Histogram::centerZero() {
  centerZero_ = 1;
  ASSERT(histogram_.empty(),
    "centerZero must be called before histogram is collected");
}

/**
 * print to file 
 */
void Histogram::print(const char* fileName) {
  std::ofstream outf(fileName);
  outf << "# " << nNorm() << endl;
  for (unsigned int bin = 0; bin < histogram_.size(); ++bin) {
    outf << bin2m(bin) << " " << histogram_[bin]/sum() << endl;
  }
}
