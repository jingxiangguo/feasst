/**
 * \file
 *
 * \brief ideal gas, no interactions
 *
 */

#include "./pair_ideal.h"

/**
 * Constructor for pair_lj class requires the following
 */
PairIdeal::PairIdeal(Space* space,
  const double rCut)  //!< interaction cut-off distance
  : Pair(space, rCut) {
  className_.assign("PairIdeal");
}

PairIdeal::~PairIdeal() {
}

/**
 */
int PairIdeal::initEnergy() {

  // zero accumulators: potential energy, force, and virial
  std::fill(pe_.begin(), pe_.end(), 0.);
  myFill(0., f_);
  myFill(0., vr_);
  return 0;
}

/**
 * Lennard-Jones potential energy contribution due to one particle
 */
double PairIdeal::multiPartEner(const vector<int> mpart,    //!< number of single particle to calculate energy interactions
                             const int flag     //!<
  ) {

  if (flag == 0) {}; //remove unused parameter warning

  // only consider pair-wise distances if neighbor list needs to be updated
  if (neighOn_) {

    /// obtain neighList with cellList
    if (space_->cellType() == 1) space_->buildNeighListCell(space_->mol()[mpart.front()]);

    // shorthand for read-only space variables
    const vector<double> &l = space_->l();
    const vector<double> &x = space_->x();
    const vector<int> &mol = space_->mol();

    double r2, xi, yi, zi, dx, dy, dz, lx = l[0], ly = l[1], lz = l[2];
    double halflx = lx/2., halfly = ly/2., halflz = lz/2.;
    neighOne_.clear();
    neighOne_.resize(int(mpart.size()), vector<int>());

    for (int impart = 0; impart < int(mpart.size()); ++impart) {
      const int ipart = mpart[impart];
      const int iMol = mol[ipart];
      xi = x[dimen_*ipart+0];
      yi = x[dimen_*ipart+1];
      zi = x[dimen_*ipart+2];

      // loop through nearest neighbor atom pairs
      //for (int jpart = 0; jpart < natom; ++jpart) {
      // loop through molecules
      const vector<int> &neigh = space_->neighListChosen();
      for (int ineigh = 0; ineigh < int(neigh.size()); ++ineigh) {
        const int jpart = neigh[ineigh];
        if (iMol != mol[jpart]) {

          // separation vector, xij with periodic boundary conditions
          dx = xi - x[dimen_*jpart];
          dy = yi - x[dimen_*jpart+1];
          dz = zi - x[dimen_*jpart+2];
          if (dx >  halflx) dx -= lx;
          if (dx < -halflx) dx += lx;
          if (dy >  halfly) dy -= ly;
          if (dy < -halfly) dy += ly;
          if (dz >  halflz) dz -= lz;
          if (dz < -halflz) dz += lz;
          r2 = dx*dx + dy*dy + dz*dz;

          // no interaction beyond cut-off distance
          if (r2 < rCut_*rCut_) {

            // store new neighbor list
            if ( (r2 < neighAboveSq_) && (r2 > neighBelowSq_) ) {
              neighOne_[impart].push_back(jpart);
            }
          }
        }
      }
    }
  }
  return 0;
}
