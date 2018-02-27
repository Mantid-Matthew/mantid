#include "EnggDiffGSASFittingWorker.h"
#include "EnggDiffGSASFittingPresenter.h"

namespace MantidQt {
namespace CustomInterfaces {

EnggDiffGSASFittingWorker::EnggDiffGSASFittingWorker(
    EnggDiffGSASFittingPresenter *pres,
    const GSASIIRefineFitPeaksParameters &params)
    : m_presenter(pres), m_refinementParams(params) {}

void EnggDiffGSASFittingWorker::doRefinement() {
  try {
    m_presenter->doRefinement(m_refinementParams);
  } catch (const std::exception &ex) {
    emit refinementFailed(ex.what());
    return;
  }
  emit refinementSucceeded();
}

} // namespace CustomInterfaces
} // namespace MantidQt
