#ifndef MANTIDQT_CUSTOMINTERFACES_ENGGDIFFMULTIRUNFITTINGWIDGETPRESENTER_H_
#define MANTIDQT_CUSTOMINTERFACES_ENGGDIFFMULTIRUNFITTINGWIDGETPRESENTER_H_

#include "DllConfig.h"
#include "IEnggDiffMultiRunFittingWidgetModel.h"
#include "IEnggDiffMultiRunFittingWidgetPresenter.h"
#include "IEnggDiffMultiRunFittingWidgetView.h"

#include <memory>

namespace MantidQt {
namespace CustomInterfaces {

class MANTIDQT_ENGGDIFFRACTION_DLL EnggDiffMultiRunFittingWidgetPresenter
    : public IEnggDiffMultiRunFittingWidgetPresenter {

public:
  EnggDiffMultiRunFittingWidgetPresenter(
      std::unique_ptr<IEnggDiffMultiRunFittingWidgetModel> model,
      std::unique_ptr<IEnggDiffMultiRunFittingWidgetView> view);

  void addFittedPeaks(const int runNumber, const size_t bank,
                      const Mantid::API::MatrixWorkspace_sptr ws) override;

  void addFocusedRun(const int runNumber, const size_t bank,
                     const Mantid::API::MatrixWorkspace_sptr ws) override;

  boost::optional<Mantid::API::MatrixWorkspace_sptr>
  getFittedPeaks(const int runNumber, const size_t bank) const override;

  boost::optional<Mantid::API::MatrixWorkspace_sptr>
  getFocusedRun(const int runNumber, const size_t bank) const override;

private:
  std::unique_ptr<IEnggDiffMultiRunFittingWidgetModel> m_model;

  std::unique_ptr<IEnggDiffMultiRunFittingWidgetView> m_view;

  bool m_viewHasClosed;
};

} // namespace CustomInterfaces
} // namespace MantidQt

#endif // MANTIDQT_CUSTOMINTERFACES_ENGGDIFFMULTIRUNFITTINGWIDGETPRESENTER_H_
