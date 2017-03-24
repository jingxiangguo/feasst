#include <gtest/gtest.h>
#include "pair_lj_multi.h"

TEST(Pair, WCAanalytical) {

  // WCA for sig=1 and 0.85
  for (double sig = 0.85; sig < 1.01; sig += 0.15) {
    Space s(3,0);
    for (int dim = 0; dim < s.dimen(); ++dim) s.lset(100, dim);
    s.addMolInit("../forcefield/data.atom");
    vector<double> x(s.dimen(), 0.);
    s.xAdd = x;
    s.addMol("../forcefield/data.atom");
    x[0] = 1.;
    s.xAdd = x;
    s.addMol("../forcefield/data.atom");
    EXPECT_NEAR(1, s.x(1,0) - s.x(0,0), 1e-15);
    EXPECT_NEAR(0, s.x(1,1) - s.x(0,1), 1e-15);
    EXPECT_NEAR(0, s.x(1,2) - s.x(0,2), 1e-15);

    PairLJMulti p(&s, 0.);
    stringstream ss;
    ss << "../forcefield/data.lj";
    if (sig == 0.85) ss << "s0.85";
    p.initLMPData(ss.str().c_str());
    p.initWCA(0,0);

    if (sig == 1) {
      s.xset(1, 1, 0);
      p.initEnergy();
      EXPECT_NEAR(1, p.peTot(), doubleTolerance);
      s.xset(pow(2, 1./6.), 1, 0);
      p.initEnergy();
      EXPECT_NEAR(0, p.peTot(), doubleTolerance);
      s.xset(1.01, 1, 0);
      p.initEnergy();
      EXPECT_NEAR(0.781615960043788000, p.peTot(), doubleTolerance);
    } else {
      s.xset(0.85, 1, 0);
      p.initEnergy();
      EXPECT_NEAR(1, p.peTot(), doubleTolerance);
      s.xset(pow(2, 1./6.)*0.85, 1, 0);
      p.initEnergy();
      EXPECT_NEAR(0, p.peTot(), doubleTolerance);
      s.xset(0.9, 1, 0);
      p.initEnergy();
      EXPECT_NEAR(0.175851657413201000, p.peTot(), doubleTolerance);
    }
  }
}

TEST(Pair, cg3analytical) {
  const double rCut = 3;
  Space s(3, 0);
  s.addMolInit("../forcefield/data.cg3_60_1_1");
  vector<double> x(s.dimen(), 0.);
  s.xAdd = x;
  s.addMol("../forcefield/data.cg3_60_1_1");
  x[0] = 2.;
  s.xAdd = x;
  s.addMol("../forcefield/data.cg3_60_1_1");
  PairLJMulti p(&s, rCut);
  p.initLMPData("../forcefield/data.cg3_60_1_1");
  p.rCutijset(1, 1, rCut);
  p.linearShiftijset(1, 1, 1);
  p.initWCA(1, 2);
  p.initWCA(2, 2);
  p.initEnergy();
  EXPECT_NEAR(p.peTot(), 0.95489983432133096, doubleTolerance);
}

TEST(Pair, LJYanalytical) {
  const double rCut = 3;
  Space s(3, 0);
  s.addMolInit("../forcefield/data.lj");
  vector<double> x(s.dimen(), 0.);
  s.xAdd = x;
  s.addMol("../forcefield/data.lj");
  x[0] = 2.;
  s.xAdd = x;
  s.addMol("../forcefield/data.lj");
  PairLJMulti p(&s, rCut);
  p.initExpType(1);
  p.initScreenedElectro(2, 0.5);
  p.initLMPData("../forcefield/data.lj");
  p.linearShift(0);
  p.lrcFlag = 0;
  p.initEnergy();
  EXPECT_NEAR(p.peTot(), 0.366903117090021000, doubleTolerance);
  p.linearShift(1);
  p.initEnergy();
  EXPECT_NEAR(p.peTot(), 0.094226110309604982, doubleTolerance);
}

TEST(Pair, MMLJanalytical) {
  Space s(3, 0);
  s.addMolInit("../forcefield/data.cg3_91_0.57_2");
  vector<double> x(s.dimen(), 0.);
  s.xAdd = x;
  s.addMol("../forcefield/data.cg3_91_0.57_2");
  x[0] = 2.*0.85;
  s.xAdd = x;
  s.addMol("../forcefield/data.cg3_91_0.57_2");
  PairLJMulti p(&s, 3);
  p.initLMPData("../forcefield/data.cg3_91_0.57_2");
  p.rCutijset(1, 1, 3);
  p.linearShiftijset(1, 1, 1);
  p.initWCA(1, 2);
  p.initWCA(2, 2);
  p.initEnergy();
  EXPECT_NEAR(p.peTot(), -0.139144838318008000 + 0.302421948569261000, 5*doubleTolerance);
  x[0] = -0.9;
  x[1] = 1;
  s.transMol(1, x);
  p.initEnergy();
  EXPECT_NEAR(p.peTot(), -0.676951777428172000 + 66.449659060736600000, 1000*doubleTolerance);
}

TEST(Pair, cg3exampleConfig) {
  const double rCut = 3;
  Space s(3, 0);
  s.addMolInit("../forcefield/data.cg3_60_1_1");
  std::ifstream inFile("../unittest/cg3/cg3_60_1_1/example/moviep1n50.xyz");
  s.readxyz2(inFile);
  EXPECT_EQ(50, s.nMol());
  vector<double> x(s.dimen(), 0.);
  PairLJMulti p(&s, rCut);
  p.initLMPData("../forcefield/data.cg3_60_1_1");
  p.rCutijset(1, 1, rCut);
  p.linearShiftijset(1, 1, 1);
  p.rCutijset(1, 2, pow(2, 1./6.));
  p.cutShiftijset(1, 2, 1);
  p.rCutijset(2, 2, pow(2, 1./6.));
  p.cutShiftijset(2, 2, 1);
  p.initEnergy();
  EXPECT_NEAR(p.peTot(), -108.25895014589899, doubleTolerance);
}

TEST(Pair, cg3analyticalAlpha128) {
  const double rCut = 1.08;
  Space s(3, 0);
  string addMolType("../forcefield/data.cg3_91_0.57_2");
  s.addMolInit(addMolType.c_str());
  vector<double> x(s.dimen(), 0.);
  s.xAdd = x;
  s.addMol(addMolType.c_str());
  x[1] = 1.22;
  s.xAdd = x;
  s.addMol(addMolType.c_str());

  PairLJMulti p(&s, rCut);
  p.initLMPData(addMolType.c_str());
  p.initExpType(4); // alpha=128
  p.rCutijset(1, 1, rCut);
  p.linearShiftijset(1, 1, 1);
  p.initWCA(1, 2);
  p.initWCA(2, 2);
  p.initEnergy();
  s.initCellAtomCut(1);

  EXPECT_NEAR(p.peTot(), 2*39.789289254425900000, 10000*doubleTolerance);
  EXPECT_NEAR(p.f(0, 1), 0, doubleTolerance);
  EXPECT_NEAR(sqrt(pow(p.f(1, 1),2)+pow(p.f(1, 0), 2)), 23095.254845213545, 1e-9);

  // test peMap and neighCut
  const double peTot = p.peTot();
  p.initNeighCut(1);
  p.initPEMap(1);
  const vector<int> mpart = s.randMol();
  p.multiPartEner(mpart, 0);
  vector<int> neigh;
  vector<double> peMap;
  p.neighCutMolPEMap(neigh, peMap);
  EXPECT_EQ(1, int(neigh.size()));
  EXPECT_EQ(1, int(peMap.size()));
  EXPECT_NEAR(peMap[0], peTot, doubleTolerance);

  // flip
  s.qMolAlt(1, 0, 1);
  s.qMolAlt(1, 3, 0);
  s.quat2pos(1);
  x[1] = -s.x(4, 1) + 2*0.266345520433943000 + 1.02;
  s.transMol(1, x);
  p.initEnergy();
  EXPECT_NEAR(p.peTot(), -0.290278106387070000, 100*doubleTolerance);
  EXPECT_NEAR(p.f(0, 1), 0, doubleTolerance);
  EXPECT_NEAR(sqrt(pow(p.f(1, 1),2)+pow(p.f(1, 0), 2)), 33.461405536957100000, 20000*doubleTolerance);
  EXPECT_EQ(1, p.checkEnergy(doubleTolerance, 2));
}

//TEST(Pair, marcoMAB) {
//  const double boxl = 30, rCut = 3, rCutLJ = 2;
//  std::ostringstream addMolType("../forcefield/data.mab1");
//  Space s(3, 0);
//  for (int dim=0; dim < s.dimen(); ++dim) s.lset(boxl,dim);
//  s.addMolInit(addMolType.str().c_str());
//  std::ifstream inFile("test/mab/marco_energy/Position_0001.xyz");
//  s.readxyz2(inFile);
//  EXPECT_EQ(2, s.nMol());
//  EXPECT_EQ(8, s.natom());
//  EXPECT_NEAR(s.x(0,0), 0.1682214722932941E+02, doubleTolerance);
//  EXPECT_NEAR(s.x(7,2), 0.2948447461948864E+02, doubleTolerance);
//
//  s.initCellAtomCut(1);
//  if (boxl/3. > rCut) {
//    s.updateCells(rCut);
//  }
//
//  PairHybrid p(&s, 0.);
//  PairLJMulti pLJ(&s, rCutLJ);
//  pLJ.initLMPData(addMolType.str().c_str());
//  pLJ.initExpType(1);
//  for (int i = 0; i < s.nParticleTypes(); ++i) {
//    for (int j = i; j < s.nParticleTypes(); ++j) {
//      pLJ.rCutijset(i, j, 2*pLJ.sigij()[i][j]);
//      pLJ.linearShiftijset(i, j, 1);
//    }
//  }
//  pLJ.lrcFlag = 0;
//  pLJ.initEnergy();
//  p.addPair(&pLJ);
//
//  PairLJMulti pCC(&s, rCut);
//  pCC.initLMPData(addMolType.str().c_str());
//  pCC.initScreenedElectro(1, 1, 2);
//  for (int i = 0; i < s.nParticleTypes(); ++i) {
//    for (int j = i; j < s.nParticleTypes(); ++j) {
//      pCC.epsijset(i, j, 0);
//      pCC.rCutijset(i, j, 6*pCC.sigij(i, j));
//      pCC.linearShiftijset(i, j, 1);
//    }
//  }
//  pCC.lrcFlag = 0;
//  pCC.initEnergy();
//  p.addPair(&pCC);
//  p.initEnergy();
//  p.printxyz("asdf",1);
//  p.checkEnergy(doubleTolerance, 1);
//
//  EXPECT_NEAR(-0.3424421004334772E+01, p.peTot(), doubleTolerance);
//}

TEST(Pair, LJSlowGaussian) {
  const double rCut = 3.00;
  Space s(3, 0);
  string addMolType("../forcefield/data.lj");
  s.addMolInit(addMolType.c_str());
  vector<double> x(s.dimen(), 0.);
  s.xAdd = x;
  s.addMol(addMolType.c_str());
  x[1] = pow(2,1./6.);
  s.xAdd = x;
  s.addMol(addMolType.c_str());

  PairLJMulti p(&s, rCut);
  p.initLMPData(addMolType.c_str());
  p.lrcFlag = 0;
  p.cutShift(0);
  p.initEnergy();
  EXPECT_NEAR(-1, p.peTot(), 10000*doubleTolerance);

  const double height = 1.3408572374689746728347876674875;
  p.addGaussian(height, x[1], 1);

  p.initEnergy();
  EXPECT_NEAR(-1+height, p.peTot(), 10000*doubleTolerance);

  p.addGaussian(1, 1, 2);
  p.initEnergy();
  EXPECT_NEAR(-1+height+exp(-pow(((x[1]-1)/2),2)), p.peTot(), 10000*doubleTolerance);

  p.writeRestart("tmp/yoyorst");
  PairLJMulti p2(&s, "tmp/yoyorst");
  p2.writeRestart("tmp/yoyorst2");
  p2.initEnergy();

  EXPECT_NEAR(p.peTot(), p2.peTot(), 10000*doubleTolerance);
}

TEST(Pair, LJSlowLambda) {
  const double rCut = 3.00;
  Space s(2, 0);
  for (int dim = 0; dim < s.dimen(); ++dim) s.lset(9, dim);
  string addMolType("../forcefield/data.lj");
  s.addMolInit(addMolType.c_str());
  vector<double> x(s.dimen(), 0.);
  s.xAdd = x;
  s.addMol(addMolType.c_str());
  x[1] = pow(2,1./6.);
  s.xAdd = x;
  s.addMol(addMolType.c_str());

  PairLJMulti p(&s, rCut);
  p.initLMPData(addMolType.c_str());
  p.setLambdaij(0,0,1);
  p.lrcFlag = 0;
  p.cutShift(1);
//  p.cutShift(0);
//  p.rCutijset(0,0,rCut);
//  p.initLRC();
  p.initEnergy();
  EXPECT_NEAR(-1+0.005479441744238780, p.peTot(), 10000*doubleTolerance);
  p.setLambdaij(0,0,-1);
  p.initEnergy();
  EXPECT_NEAR(1-0.005479441744238780, p.peTot(), 10000*doubleTolerance);
  p.setLambdaij(0, 0, 0);
  p.initEnergy();
  EXPECT_NEAR(0., p.peTot(), 10000*doubleTolerance);

//  exit(0);
  p.writeRestart("tmp/yoyorst");
  PairLJMulti p2(&s, "tmp/yoyorst");
  p2.writeRestart("tmp/yoyorst2");
  p2.initEnergy();

  EXPECT_NEAR(p.peTot(), p2.peTot(), 10000*doubleTolerance);
}
