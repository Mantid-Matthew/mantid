#ifndef MANTIDQTCUSTOMINTERFACES_ENGGDIFFRACTION_ENGGDIFFGSASFITTINGWORKER_H_
#define MANTIDQTCUSTOMINTERFACES_ENGGDIFFRACTION_ENGGDIFFGSASFITTINGWORKER_H_

#include "DllConfig.h"
#include "GSASIIRefineFitPeaksParameters.h"

#include <QObject>

/**
Worker for long-running tasks (ie GSASIIRefineFitPeaks) in the GSAS tab of the
Engineering Diffraction GUI
*/
namespace MantidQt {
namespace CustomInterfaces {

class EnggDiffGSASFittingPresenter;

class EnggDiffGSASFittingWorker : public QObject {
  Q_OBJECT

public:
  EnggDiffGSASFittingWorker(EnggDiffGSASFittingPresenter *pres,
                            const GSASIIRefineFitPeaksParameters &params);

public slots:
  void doRefinement();

signals:
  void refinementFailed(const std::string &failureMessage);
  void refinementSucceeded();

private:
  EnggDiffGSASFittingPresenter *m_presenter;

  const GSASIIRefineFitPeaksParameters m_refinementParams;
};

} // namespace CustomInterfaces
} // namespace MantidQt

#endif // MANTIDQTCUSTOMINTERFACES_ENGGDIFFRACTION_ENGGDIFFGSASFITTINGWORKER_H_
