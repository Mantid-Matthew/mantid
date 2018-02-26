#ifndef SOFQWCUTTEST_H_
#define SOFQWCUTTEST_H_

#include <cxxtest/TestSuite.h>
#include "MantidAlgorithms/SofQW.h"
#include "MantidAlgorithms/SofQWPolygon.h"
#include "MantidAlgorithms/SofQWNormalisedPolygon.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/Axis.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceHistory.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidKernel/Unit.h"
#include "MantidDataHandling/LoadNexusProcessed.h"

#include <boost/make_shared.hpp>

using namespace Mantid::API;

class SofQWCutTest : public CxxTest::TestSuite {
public:
  template <typename SQWType> WorkspaceGroup_sptr runSQW() {
    Mantid::DataHandling::LoadNexusProcessed loader;
    loader.initialize();
    loader.setChild(true);
    loader.setProperty("Filename", "MAR21335_Ei60meV.nxs");
    loader.setPropertyValue("OutputWorkspace", "__unused");
    loader.execute();

    Workspace_sptr loadedWS = loader.getProperty("OutputWorkspace");
    auto inWS = boost::dynamic_pointer_cast<MatrixWorkspace>(loadedWS);

    // First make a cut along |Q|
    SQWType sqw_q;
    sqw_q.initialize();
    // Cannot be marked as child or history is not recorded
    TS_ASSERT_THROWS_NOTHING(sqw_q.setProperty("InputWorkspace", inWS));
    std::ostringstream wsname_q;
    wsname_q << "_tmp_q_" << loadedWS;
    TS_ASSERT_THROWS_NOTHING(
        sqw_q.setPropertyValue("OutputWorkspace", wsname_q.str()));
    TS_ASSERT_THROWS_NOTHING(
        sqw_q.setPropertyValue("QAxisBinning", "0,0.0125,10"));
    TS_ASSERT_THROWS_NOTHING(sqw_q.setPropertyValue("EMode", "Direct"));
    TS_ASSERT_THROWS_NOTHING(
        sqw_q.setPropertyValue("EAxisBinning", "-1.5,3,1.5"));
    TS_ASSERT_THROWS_NOTHING(sqw_q.execute());
    TS_ASSERT(sqw_q.isExecuted());

    // Now make a cut along E
    SQWType sqw_e;
    sqw_e.initialize();
    // Cannot be marked as child or history is not recorded
    TS_ASSERT_THROWS_NOTHING(sqw_e.setProperty("InputWorkspace", inWS));
    std::ostringstream wsname_e;
    wsname_e << "_tmp_e_" << loadedWS;
    TS_ASSERT_THROWS_NOTHING(
        sqw_e.setPropertyValue("OutputWorkspace", wsname_e.str()));
    TS_ASSERT_THROWS_NOTHING(sqw_e.setPropertyValue("QAxisBinning", "5,5,10"));
    TS_ASSERT_THROWS_NOTHING(sqw_e.setPropertyValue("EMode", "Direct"));
    TS_ASSERT_THROWS_NOTHING(
        sqw_e.setPropertyValue("EAxisBinning", "-5,0.5,55"));
    TS_ASSERT_THROWS_NOTHING(sqw_e.execute());
    TS_ASSERT(sqw_e.isExecuted());

    auto &dataStore = AnalysisDataService::Instance();
    auto ws_q = dataStore.retrieveWS<MatrixWorkspace>(wsname_q.str());
    auto ws_e = dataStore.retrieveWS<MatrixWorkspace>(wsname_e.str());
    WorkspaceGroup_sptr result = boost::make_shared<WorkspaceGroup>();
    result->addWorkspace(ws_q);
    result->addWorkspace(ws_e);
    dataStore.remove(wsname_q.str());
    dataStore.remove(wsname_e.str());

    return result;
  }

  void test_sofqw1() {
    auto result = runSQW<Mantid::Algorithms::SofQW>();
    const double delta(1e-08);

    auto ws_q =
        boost::dynamic_pointer_cast<MatrixWorkspace>(result->getItem(0));
    TS_ASSERT_EQUALS(ws_q->getAxis(0)->length(), 2);
    TS_ASSERT_EQUALS(ws_q->getAxis(0)->unit()->unitID(), "DeltaE");
    TS_ASSERT_EQUALS((*(ws_q->getAxis(0)))(0), -1.5);
    TS_ASSERT_EQUALS((*(ws_q->getAxis(0)))(1), 1.5);
    TS_ASSERT_EQUALS(ws_q->getAxis(1)->length(), 801);
    TS_ASSERT_EQUALS(ws_q->getAxis(1)->unit()->unitID(), "MomentumTransfer");
    TS_ASSERT_EQUALS((*(ws_q->getAxis(1)))(0), 0.0);
    TS_ASSERT_DELTA((*(ws_q->getAxis(1)))(400), 5.0, delta);
    TS_ASSERT_EQUALS((*(ws_q->getAxis(1)))(800), 10.);
    TS_ASSERT_DELTA(ws_q->readY(44)[0], 957.651473192, delta);
    TS_ASSERT_DELTA(ws_q->readE(44)[0], 11.170620862, delta);
    TS_ASSERT_DELTA(ws_q->readY(231)[0], 398.376497999, delta);
    TS_ASSERT_DELTA(ws_q->readE(231)[0], 62.100406977, delta);
    TS_ASSERT_DELTA(ws_q->readY(377)[0], 232.378738932, delta);
    TS_ASSERT_DELTA(ws_q->readE(377)[0], 14.249051816, delta);
    TS_ASSERT_DELTA(ws_q->readY(536)[0], 1832.305224868, delta);
    TS_ASSERT_DELTA(ws_q->readE(536)[0], 30.518095107, delta);
    TS_ASSERT_DELTA(ws_q->readY(575)[0], 453.761721652, delta);
    TS_ASSERT_DELTA(ws_q->readE(575)[0], 13.114162862, delta);

    auto ws_e =
        boost::dynamic_pointer_cast<MatrixWorkspace>(result->getItem(1));
    TS_ASSERT_EQUALS(ws_e->getAxis(0)->length(), 121);
    TS_ASSERT_EQUALS(ws_e->getAxis(0)->unit()->unitID(), "DeltaE");
    TS_ASSERT_EQUALS((*(ws_e->getAxis(0)))(0), -5.);
    TS_ASSERT_DELTA((*(ws_e->getAxis(0)))(60), 25.0, delta);
    TS_ASSERT_EQUALS((*(ws_e->getAxis(0)))(120), 55.);
    TS_ASSERT_EQUALS(ws_e->getAxis(1)->length(), 2);
    TS_ASSERT_EQUALS(ws_e->getAxis(1)->unit()->unitID(), "MomentumTransfer");
    TS_ASSERT_EQUALS((*(ws_e->getAxis(1)))(0), 5.);
    TS_ASSERT_EQUALS((*(ws_e->getAxis(1)))(1), 10.);
    TS_ASSERT_DELTA(ws_e->readY(0)[29], 9.254559817, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[29], 0.030174342, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[87], 13.447772682, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[87], 0.051154627, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[88], 10.455499052, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[88], 0.044293372, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[93], 3.587987494, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[93], 0.026975541, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[113], 1.038679349, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[113], 0.044564335, delta);
  }

  void test_sofqw2() {
    auto result = runSQW<Mantid::Algorithms::SofQWPolygon>();
    const double delta(1e-08);

    auto ws_q =
        boost::dynamic_pointer_cast<MatrixWorkspace>(result->getItem(0));
    TS_ASSERT_EQUALS(ws_q->getAxis(0)->length(), 2);
    TS_ASSERT_EQUALS(ws_q->getAxis(0)->unit()->unitID(), "DeltaE");
    TS_ASSERT_EQUALS((*(ws_q->getAxis(0)))(0), -1.5);
    TS_ASSERT_EQUALS((*(ws_q->getAxis(0)))(1), 1.5);
    TS_ASSERT_EQUALS(ws_q->getAxis(1)->length(), 801);
    TS_ASSERT_EQUALS(ws_q->getAxis(1)->unit()->unitID(), "MomentumTransfer");
    TS_ASSERT_EQUALS((*(ws_q->getAxis(1)))(0), 0.0);
    TS_ASSERT_DELTA((*(ws_q->getAxis(1)))(400), 5.0, delta);
    TS_ASSERT_EQUALS((*(ws_q->getAxis(1)))(800), 10.);
    TS_ASSERT_DELTA(ws_q->readY(46)[0], 0.577055734, delta);
    TS_ASSERT_DELTA(ws_q->readE(46)[0], 0.016266516, delta);
    TS_ASSERT_DELTA(ws_q->readY(461)[0], 0.642083585, delta);
    TS_ASSERT_DELTA(ws_q->readE(461)[0], 0.027694702, delta);
    TS_ASSERT_DELTA(ws_q->readY(703)[0], 8.619229199, delta);
    TS_ASSERT_DELTA(ws_q->readE(703)[0], 0.119106057, delta);
    TS_ASSERT_DELTA(ws_q->readY(727)[0], 1.212655693, delta);
    TS_ASSERT_DELTA(ws_q->readE(727)[0], 0.047618940, delta);
    TS_ASSERT_DELTA(ws_q->readY(787)[0], 12.280788436, delta);
    TS_ASSERT_DELTA(ws_q->readE(787)[0], 0.239880567, delta);

    auto ws_e =
        boost::dynamic_pointer_cast<MatrixWorkspace>(result->getItem(1));
    TS_ASSERT_EQUALS(ws_e->getAxis(0)->length(), 121);
    TS_ASSERT_EQUALS(ws_e->getAxis(0)->unit()->unitID(), "DeltaE");
    TS_ASSERT_EQUALS((*(ws_e->getAxis(0)))(0), -5.);
    TS_ASSERT_DELTA((*(ws_e->getAxis(0)))(60), 25.0, delta);
    TS_ASSERT_EQUALS((*(ws_e->getAxis(0)))(120), 55.);
    TS_ASSERT_EQUALS(ws_e->getAxis(1)->length(), 2);
    TS_ASSERT_EQUALS(ws_e->getAxis(1)->unit()->unitID(), "MomentumTransfer");
    TS_ASSERT_EQUALS((*(ws_e->getAxis(1)))(0), 5.);
    TS_ASSERT_EQUALS((*(ws_e->getAxis(1)))(1), 10.);
    TS_ASSERT_DELTA(ws_e->readY(0)[5], 1120.875680688, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[5], 5.143783614, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[16], 171.212246850, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[16], 2.079560024, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[28], 40.854749824, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[28], 1.014309882, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[36], 54.655069317, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[36], 1.179213931, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[113], 3.724579351, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[113], 0.485226781, delta);
  }

  void test_sofqw3() {
    auto result = runSQW<Mantid::Algorithms::SofQWNormalisedPolygon>();
    const double delta(1e-08);

    auto ws_q =
        boost::dynamic_pointer_cast<MatrixWorkspace>(result->getItem(0));
    TS_ASSERT_EQUALS(ws_q->getAxis(0)->length(), 2);
    TS_ASSERT_EQUALS(ws_q->getAxis(0)->unit()->unitID(), "DeltaE");
    TS_ASSERT_EQUALS((*(ws_q->getAxis(0)))(0), -1.5);
    TS_ASSERT_EQUALS((*(ws_q->getAxis(0)))(1), 1.5);
    TS_ASSERT_EQUALS(ws_q->getAxis(1)->length(), 801);
    TS_ASSERT_EQUALS(ws_q->getAxis(1)->unit()->unitID(), "MomentumTransfer");
    TS_ASSERT_EQUALS((*(ws_q->getAxis(1)))(0), 0.0);
    TS_ASSERT_DELTA((*(ws_q->getAxis(1)))(400), 5.0, delta);
    TS_ASSERT_EQUALS((*(ws_q->getAxis(1)))(800), 10.);
    TS_ASSERT_DELTA(ws_q->readY(64)[0], 0.144715421, delta);
    TS_ASSERT_DELTA(ws_q->readE(64)[0], 0.004902364, delta);
    TS_ASSERT_DELTA(ws_q->readY(345)[0], 0.658678386, delta);
    TS_ASSERT_DELTA(ws_q->readE(345)[0], 0.016428510, delta);
    TS_ASSERT_DELTA(ws_q->readY(595)[0], 0.159563545, delta);
    TS_ASSERT_DELTA(ws_q->readE(595)[0], 0.007806737, delta);
    TS_ASSERT_DELTA(ws_q->readY(683)[0], 0.178108225, delta);
    TS_ASSERT_DELTA(ws_q->readE(683)[0], 0.012961016, delta);
    TS_ASSERT_DELTA(ws_q->readY(745)[0], 2.086237760, delta);
    TS_ASSERT_DELTA(ws_q->readE(745)[0], 0.034274820, delta);

    auto ws_e =
        boost::dynamic_pointer_cast<MatrixWorkspace>(result->getItem(1));
    TS_ASSERT_EQUALS(ws_e->getAxis(0)->length(), 121);
    TS_ASSERT_EQUALS(ws_e->getAxis(0)->unit()->unitID(), "DeltaE");
    TS_ASSERT_EQUALS((*(ws_e->getAxis(0)))(0), -5.);
    TS_ASSERT_DELTA((*(ws_e->getAxis(0)))(60), 25.0, delta);
    TS_ASSERT_EQUALS((*(ws_e->getAxis(0)))(120), 55.);
    TS_ASSERT_EQUALS(ws_e->getAxis(1)->length(), 2);
    TS_ASSERT_EQUALS(ws_e->getAxis(1)->unit()->unitID(), "MomentumTransfer");
    TS_ASSERT_EQUALS((*(ws_e->getAxis(1)))(0), 5.);
    TS_ASSERT_EQUALS((*(ws_e->getAxis(1)))(1), 10.);
    TS_ASSERT_DELTA(ws_e->readY(0)[3], 2.003485282, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[3], 0.012959174, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[20], 0.136945077, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[20], 0.003215584, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[27], 0.158356991, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[27], 0.003823510, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[78], 0.197240860, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[78], 0.005090697, delta);
    TS_ASSERT_DELTA(ws_e->readY(0)[119], 0.027223857, delta);
    TS_ASSERT_DELTA(ws_e->readE(0)[119], 0.002884865, delta);
  }
};

#endif /*SOFQWCUTTEST_H_*/
