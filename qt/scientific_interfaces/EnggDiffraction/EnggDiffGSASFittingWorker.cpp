#include "EnggDiffGSASFittingWorker.h"
#include "EnggDiffGSASFittingPresenter.h"

namespace MantidQt {
namespace CustomInterfaces {

EnggDiffGSASFittingWorker::EnggDiffGSASFittingWorker(
    EnggDiffGSASFittingPresenter *pres,
    const GSASIIRefineFitPeaksParameters &params)
    : m_presenter(pres), m_refinementParams(params) {}

void EnggDiffGSASFittingWorker::doRefinement() {
  m_presenter->doRefinement(m_refinementParams);
  emit finished();
}

} // namespace CustomInterfaces
} // namespace MantidQt
