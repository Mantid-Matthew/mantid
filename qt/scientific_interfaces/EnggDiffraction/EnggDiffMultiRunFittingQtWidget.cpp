#include "EnggDiffMultiRunFittingQtWidget.h"

#include "MantidKernel/make_unique.h"

namespace MantidQt {
namespace CustomInterfaces {

EnggDiffMultiRunFittingQtWidget::EnggDiffMultiRunFittingQtWidget(
    boost::shared_ptr<IEnggDiffractionPythonRunner> pythonRunner)
    : m_pythonRunner(pythonRunner) {
  setupUI();

  m_zoomTool = Mantid::Kernel::make_unique<QwtPlotZoomer>(
      QwtPlot::xBottom, QwtPlot::yLeft,
      QwtPicker::DragSelection | QwtPicker::CornerToCorner,
      QwtPicker::AlwaysOff, m_ui.plotArea->canvas());
  m_zoomTool->setRubberBandPen(QPen(Qt::black));
  m_zoomTool->setEnabled(false);
}

EnggDiffMultiRunFittingQtWidget::~EnggDiffMultiRunFittingQtWidget() {
  cleanUpPlot();
}

void EnggDiffMultiRunFittingQtWidget::cleanUpPlot() {
  for (auto &curve : m_focusedRunCurves) {
    curve->detach();
  }
  m_focusedRunCurves.clear();
  for (auto &curve : m_fittedPeaksCurves) {
    curve->detach();
  }
  m_fittedPeaksCurves.clear();
}

boost::optional<RunLabel>
EnggDiffMultiRunFittingQtWidget::getSelectedRunLabel() const {
  if (hasSelectedRunLabel()) {
    const auto currentLabel = m_ui.listWidget_runLabels->currentItem()->text();
    const auto pieces = currentLabel.split("_");
    if (pieces.size() != 2) {
      throw std::runtime_error(
          "Unexpected run label: \"" + currentLabel.toStdString() +
          "\". Please contact the development team with this message");
    }
    return RunLabel(pieces[0].toInt(), pieces[1].toUInt());
  } else {
    return boost::none;
  }
}

void EnggDiffMultiRunFittingQtWidget::reportNoRunSelectedForPlot() {
  userError("No run selected",
            "Please select a run from the list before plotting");
}

void EnggDiffMultiRunFittingQtWidget::reportPlotInvalidFittedPeaks(
    const RunLabel &runLabel) {
  userError("Invalid fitted peaks identifier",
            "Tried to plot invalid fitted peaks, run number " +
                std::to_string(runLabel.runNumber) + " and bank ID " +
                std::to_string(runLabel.bank) +
                ". Please contact the development team with this message");
}

void EnggDiffMultiRunFittingQtWidget::reportPlotInvalidFocusedRun(
    const RunLabel &runLabel) {
  userError("Invalid focused run identifier",
            "Tried to plot invalid focused run, run number " +
                std::to_string(runLabel.runNumber) + " and bank ID " +
                std::to_string(runLabel.bank) +
                ". Please contact the development team with this message");
}

bool EnggDiffMultiRunFittingQtWidget::hasSelectedRunLabel() const {
  return m_ui.listWidget_runLabels->selectedItems().size() != 0;
}

void EnggDiffMultiRunFittingQtWidget::plotFittedPeaksStateChanged() {
  m_presenter->notify(IEnggDiffMultiRunFittingWidgetPresenter::Notification::
                          PlotPeaksStateChanged);
}

void EnggDiffMultiRunFittingQtWidget::plotFittedPeaks(
    const std::vector<boost::shared_ptr<QwtData>> &curves) {
  for (const auto &curve : curves) {
    auto plotCurve = Mantid::Kernel::make_unique<QwtPlotCurve>();

    plotCurve->setPen(QColor(Qt::red));
    plotCurve->setData(*curve);
    plotCurve->attach(m_ui.plotArea);
    m_fittedPeaksCurves.emplace_back(std::move(plotCurve));
  }
  m_ui.plotArea->replot();
  m_zoomTool->setZoomBase();
  m_zoomTool->setEnabled(true);
}

void EnggDiffMultiRunFittingQtWidget::processPlotToSeparateWindow() {
  m_presenter->notify(IEnggDiffMultiRunFittingWidgetPresenter::Notification::
                          PlotToSeparateWindow);
}

void EnggDiffMultiRunFittingQtWidget::plotFocusedRun(
    const std::vector<boost::shared_ptr<QwtData>> &curves) {
  for (const auto &curve : curves) {
    auto plotCurve = Mantid::Kernel::make_unique<QwtPlotCurve>();

    plotCurve->setData(*curve);
    plotCurve->attach(m_ui.plotArea);
    m_focusedRunCurves.emplace_back(std::move(plotCurve));
  }
  m_ui.plotArea->replot();
  m_zoomTool->setZoomBase();
  m_zoomTool->setEnabled(true);
}

void EnggDiffMultiRunFittingQtWidget::plotToSeparateWindow(
    const std::string &focusedRunName,
    const boost::optional<std::string> fittedPeaksName) {

  std::string plotCode = "ws1 = \"" + focusedRunName + "\"\n";

  plotCode += "workspaceToPlot = \"engg_gui_separate_plot_ws\"\n"

              "if (mtd.doesExist(workspaceToPlot)):\n"
              "    DeleteWorkspace(workspaceToPlot)\n"

              "ExtractSingleSpectrum(InputWorkspace=ws1, WorkspaceIndex=0, "
              "OutputWorkspace=workspaceToPlot)\n"

              "spectra_to_plot = [0]\n";

  if (fittedPeaksName) {
    plotCode += "ws2 = \"" + *fittedPeaksName + "\"\n";
    plotCode +=
        "ws2_spectrum = ExtractSingleSpectrum(InputWorkspace=ws2, "
        "WorkspaceIndex=0, StoreInADS=False)\n"

        "AppendSpectra(InputWorkspace1=workspaceToPlot, "
        "InputWorkspace2=ws2_spectrum, OutputWorkspace=workspaceToPlot)\n"

        "DeleteWorkspace(ws2_spectrum)\n"
        "spectra_to_plot = [0, 1]\n";
  }

  plotCode +=
      "plot = plotSpectrum(workspaceToPlot, spectra_to_plot).activeLayer()\n"
      "plot.setTitle(\"Engg GUI Fitting Workspaces\")\n";
  m_pythonRunner->enggRunPythonCode(plotCode);
}

void EnggDiffMultiRunFittingQtWidget::processRemoveRun() {
  emit removeRunClicked();
  m_presenter->notify(
      IEnggDiffMultiRunFittingWidgetPresenter::Notification::RemoveRun);
}

void EnggDiffMultiRunFittingQtWidget::processSelectRun() {
  emit runSelected();
  m_presenter->notify(
      IEnggDiffMultiRunFittingWidgetPresenter::Notification::SelectRun);
}

void EnggDiffMultiRunFittingQtWidget::resetCanvas() {
  cleanUpPlot();
  m_ui.plotArea->replot();
  resetPlotZoomLevel();
}

void EnggDiffMultiRunFittingQtWidget::resetPlotZoomLevel() {
  m_ui.plotArea->setAxisAutoScale(QwtPlot::xBottom);
  m_ui.plotArea->setAxisAutoScale(QwtPlot::yLeft);
  m_zoomTool->setZoomBase(true);
}

void EnggDiffMultiRunFittingQtWidget::setMessageProvider(
    boost::shared_ptr<IEnggDiffractionUserMsg> messageProvider) {
  m_userMessageProvider = messageProvider;
}

void EnggDiffMultiRunFittingQtWidget::setPresenter(
    boost::shared_ptr<IEnggDiffMultiRunFittingWidgetPresenter> presenter) {
  m_presenter = presenter;
}

void EnggDiffMultiRunFittingQtWidget::setupUI() {
  m_ui.setupUi(this);

  connect(m_ui.listWidget_runLabels, SIGNAL(itemSelectionChanged()), this,
          SLOT(processSelectRun()));
  connect(m_ui.checkBox_plotFittedPeaks, SIGNAL(stateChanged(int)), this,
          SLOT(plotFittedPeaksStateChanged()));
  connect(m_ui.pushButton_removeRun, SIGNAL(clicked()), this,
          SLOT(processRemoveRun()));
  connect(m_ui.pushButton_plotToSeparateWindow, SIGNAL(clicked()), this,
          SLOT(processPlotToSeparateWindow()));
}

bool EnggDiffMultiRunFittingQtWidget::showFitResultsSelected() const {
  return m_ui.checkBox_plotFittedPeaks->isChecked();
}

void EnggDiffMultiRunFittingQtWidget::updateRunList(
    const std::vector<RunLabel> &runLabels) {
  m_ui.listWidget_runLabels->clear();
  for (const auto &runLabel : runLabels) {
    const auto labelStr = QString::number(runLabel.runNumber) + tr("_") +
                          QString::number(runLabel.bank);
    m_ui.listWidget_runLabels->addItem(labelStr);
  }
}

void EnggDiffMultiRunFittingQtWidget::userError(
    const std::string &errorTitle, const std::string &errorDescription) {
  m_userMessageProvider->userError(errorTitle, errorDescription);
}

} // CustomInterfaces
} // MantidQt
