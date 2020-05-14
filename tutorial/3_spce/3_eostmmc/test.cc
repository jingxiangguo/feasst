/**
 * FEASST - Free Energy and Advanced Sampling Simulation Toolkit
 * http://pages.nist.gov/feasst, National Institute of Standards and Technology
 * Harold W. Hatch, harold.hatch@nist.gov
 *
 * Permission to use this data/software is contingent upon your acceptance of
 * the terms of this agreement (see LICENSE.txt) and upon your providing
 * appropriate acknowledgments of NIST's creation of the data/software.
 */

#include "feasst.h"

int main(int argc, char** argv) {  // SPCE, SRSW_EOSTMMC

  int openMP = 0;
  int nMolMax = 5, nMolMin = 0, nfreq = 1e4, ncfreq = 1e5;
  double rCut = 10., temp = 525, lnz = -8.14, boxl = 20.;
  std::stringstream molType;
  molType << "data.spce";

  // parse command-line arguments using getopt
  { int index, c; opterr = 0;
    while ((c = getopt(argc, argv, "t:r:z:l:x:n:f:c:p:m:o:")) != -1) {
      switch (c) {
        case 't': temp = atof(optarg); break;
        case 'r': rCut = atof(optarg); break;
        case 'z': lnz = atof(optarg); break;
        case 'l': boxl = atof(optarg); break;
        case 'x': nMolMax = atoi(optarg); break;
        case 'n': nMolMin = atoi(optarg); break;
        case 'f': nfreq = atoi(optarg); break;
        case 'c': ncfreq = atoi(optarg); break;
        case 'o': openMP = atoi(optarg); break;
        case 'm': molType.str(""); molType << optarg; break;
	      case '?':
          if (isprint(optopt))
            fprintf(stderr, "Unknown option `-%c'.\n", optopt);
          else
            fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
          return 1;
        default: abort();
      }
    }
    cout << "# -t temp " << temp << " -r rCut " << rCut << " -z lnz " << lnz << " -l boxl " << boxl << " -x nMolMax " << nMolMax << " -n nMolMin " << nMolMin << " -f nfreq " << nfreq << " -c ncfreq " << ncfreq << " -m molType " << molType.str() << " -o openMP " << openMP << endl;
    for (index = optind; index < argc; index++) printf("Non-option argument %s\n", argv[index]);
  }  // GETOPT

  // initialize simulation domain
  feasst::ranInitByDate();
  auto space = feasst::makeSpace(
    {{"dimen", "3"},
     {"boxLength", feasst::str(boxl)}});

  // initialize pair-wise interactions
  auto pair = feasst::makePairLJCoulEwald(space,
    {{"rCut", feasst::str(rCut)},
     {"molTypeInForcefield", molType.str()},
     {"alphaL", "5.6"},
     {"k2max", "38"}});
  // pair->initAtomCut(1);
  // pair->equateRcutForAllTypes();

  // acceptance criteria
  auto criteria = feasst::makeCriteriaWLTMMC(
    {{"beta", feasst::str(1./(temp*feasst::idealGasConstant/1e3))},
     {"activ", feasst::str(exp(lnz))},
     {"mType", "nmol"},
     {"nMin", feasst::str(nMolMin)},
     {"nMax", feasst::str(nMolMax)}});
  criteria->collectInit(15);  // begin collection matrix at 15 WL flatness
  criteria->tmmcInit(20);     // begin transition matrix at 20 WL flatness

  // initialize MC simulation object
  feasst::WLTMMC mc(pair, criteria);
  mc.weight = 0.4;
  feasst::transformTrial(&mc, "translate");
  feasst::transformTrial(&mc, "rotate");
  auto tdel = feasst::makeTrialDelete();
  tdel->numFirstBeads(10);
  mc.weight = 0.1;
  mc.initTrial(tdel);
  auto tadd = feasst::makeTrialAdd(space->addMolListType(0).c_str());
  tadd->numFirstBeads(10);
  mc.weight = 0.1;
  mc.initTrial(tadd);

  // output log, lnpi and movie
  mc.initLog("log", nfreq);
  mc.initColMat("colMat", ncfreq);
  mc.setNFreqCheckE(ncfreq,
                    2e-4);  // absolute (no-percentage) energy tolerance
  mc.setNFreqTune(nfreq);
  mc.initMovie("movie", nfreq);
  //mc.initXTC("movie", nfreq);
  mc.initRestart("tmp/rst", ncfreq);

  //production tmmc simulation
  if (openMP != 0) {
    mc.initWindows(1.75,  // exponent that determines size of windows
                   0);    // extra macrostate overlap between processors
  }
  mc.runNumSweeps(2,   // number of "sweeps"
                 -1);   // maximum number of trials. Infinite if "-1".
}


