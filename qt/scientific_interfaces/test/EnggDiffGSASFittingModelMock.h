#ifndef MANTID_CUSTOM_INTERFACES_ENGGDIFFFITTINGMODELMOCK_H_
#define MANTID_CUSTOM_INTERFACES_ENGGDIFFFITTINGMODELMOCK_H_

#include "../EnggDiffraction/IEnggDiffGSASFittingModel.h"
#include "MantidKernel/WarningSuppressions.h"

#include <gmock/gmock.h>

GCC_DIAG_OFF_SUGGEST_OVERRIDE

using namespace MantidQt::CustomInterfaces;

class MockEnggDiffGSASFittingModel : public IEnggDiffGSASFittingModel {

public:
  MOCK_METHOD1(doPawleyRefinement,
               Mantid::API::MatrixWorkspace_sptr(
                   const GSASIIRefineFitPeaksParameters &params));

  MOCK_METHOD1(doRietveldRefinement,
               Mantid::API::MatrixWorkspace_sptr(
                   const GSASIIRefineFitPeaksParameters &params));

  MOCK_CONST_METHOD1(getGamma,
                     boost::optional<double>(const RunLabel &runLabel));

  MOCK_CONST_METHOD1(getLatticeParams,
                     boost::optional<Mantid::API::ITableWorkspace_sptr>(
                         const RunLabel &runLabel));

  MOCK_CONST_METHOD1(getRwp, boost::optional<double>(const RunLabel &runLabel));

  MOCK_CONST_METHOD1(getSigma,
                     boost::optional<double>(const RunLabel &runLabel));

  MOCK_CONST_METHOD1(hasFitResultsForRun, bool(const RunLabel &runLabel));

  MOCK_CONST_METHOD1(loadFocusedRun, Mantid::API::MatrixWorkspace_sptr(
                                         const std::string &filename));
};

GCC_DIAG_ON_SUGGEST_OVERRIDE

#endif // MANTID_CUSTOM_INTERFACES_ENGGDIFFFITTINGMODELMOCK_H_
