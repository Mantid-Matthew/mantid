/*WIKI*

This algorithm is responsible for making the conversion from time-of-flight to
energy transfer for direct geometry spectrometers.

*WIKI*/

#include "MantidWorkflowAlgorithms/DgsConvertToEnergyTransfer.h"
#include "MantidAPI/PropertyManagerDataService.h"
#include "MantidAPI/WorkspaceHistory.h"
#include "MantidGeometry/IDetector.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/FacilityInfo.h"
#include "MantidKernel/ListValidator.h"
#include "MantidKernel/PropertyManager.h"
#include "MantidKernel/RebinParamsValidator.h"
#include "MantidKernel/System.h"
#include "MantidKernel/VisibleWhenProperty.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::Geometry;

namespace Mantid
{
  namespace WorkflowAlgorithms
  {

    // Register the algorithm into the AlgorithmFactory
    DECLARE_ALGORITHM(DgsConvertToEnergyTransfer)

    //----------------------------------------------------------------------------------------------
    /** Constructor
     */
    DgsConvertToEnergyTransfer::DgsConvertToEnergyTransfer()
    {
    }

    //----------------------------------------------------------------------------------------------
    /** Destructor
     */
    DgsConvertToEnergyTransfer::~DgsConvertToEnergyTransfer()
    {
    }

    //----------------------------------------------------------------------------------------------
    /// Algorithm's name for identification. @see Algorithm::name
    const std::string DgsConvertToEnergyTransfer::name() const { return "DgsConvertToEnergyTransfer"; };

    /// Algorithm's version for identification. @see Algorithm::version
    int DgsConvertToEnergyTransfer::version() const { return 1; };

    /// Algorithm's category for identification. @see Algorithm::category
    const std::string DgsConvertToEnergyTransfer::category() const { return "Workflow\\Inelastic"; }

    //----------------------------------------------------------------------------------------------
    /// Sets documentation strings for this algorithm
    void DgsConvertToEnergyTransfer::initDocs()
    {
      this->setWikiSummary("Algorithm to convert from TOF to energy transfer.");
      this->setOptionalMessage("Algorithm to convert from TOF to energy transfer.");
    }

    //----------------------------------------------------------------------------------------------
    /** Initialize the algorithm's properties.
     */
    void DgsConvertToEnergyTransfer::init()
    {
      this->declareProperty(new WorkspaceProperty<>("InputWorkspace", "", Direction::Input),
          "A sample data workspace.");
      this->declareProperty("IncidentEnergyGuess", EMPTY_DBL(),
          "This is the starting point for the incident energy calculation.");
      this->declareProperty(new WorkspaceProperty<>("IntegratedDetectorVanadium", "",
          Direction::Input, PropertyMode::Optional), "A workspace containing the "
          "integrated detector vanadium.");
      this->declareProperty(new WorkspaceProperty<MatrixWorkspace>("MaskWorkspace",
          "", Direction::Input, PropertyMode::Optional), "A mask workspace");
      this->declareProperty(new WorkspaceProperty<MatrixWorkspace>("GroupingWorkspace",
          "", Direction::Input, PropertyMode::Optional), "A grouping workspace");
      this->declareProperty("AlternateGroupingTag", "",
          "Allows modification to the OldGroupingFile property name");
      this->declareProperty(new WorkspaceProperty<>("OutputWorkspace", "",
          Direction::Output), "The name for the output workspace.");
      this->declareProperty("ReductionProperties", "__dgs_reduction_properties", Direction::Input);
    }

    //----------------------------------------------------------------------------------------------
    /** Execute the algorithm.
     */
    void DgsConvertToEnergyTransfer::exec()
    {
      g_log.notice() << "Starting DgsConvertToEnergyTransfer" << std::endl;
      // Get the reduction property manager
      const std::string reductionManagerName = this->getProperty("ReductionProperties");
      boost::shared_ptr<PropertyManager> reductionManager;
      if (PropertyManagerDataService::Instance().doesExist(reductionManagerName))
      {
        reductionManager = PropertyManagerDataService::Instance().retrieve(reductionManagerName);
      }
      else
      {
        throw std::runtime_error("DgsConvertToEnergyTransfer cannot run without a reduction PropertyManager.");
      }

      MatrixWorkspace_sptr inputWS = this->getProperty("InputWorkspace");
      MatrixWorkspace_sptr outputWS = this->getProperty("OutputWorkspace");

      // Make a monitor workspace name for SNS data
      std::string monWsName = inputWS->getName() + "_monitors";
      bool preserveEvents = false;

      // Calculate the initial energy and time zero
      const std::string facility = ConfigService::Instance().getFacility().name();
      g_log.notice() << "Processing for " << facility << std::endl;
      double eiGuess = this->getProperty("IncidentEnergyGuess");
      if (EMPTY_DBL() == eiGuess)
      {
        eiGuess = reductionManager->getProperty("IncidentEnergyGuess");
      }
      const bool useEiGuess = reductionManager->getProperty("UseIncidentEnergyGuess");
      const double tZeroGuess = reductionManager->getProperty("TimeZeroGuess");
      std::vector<double> etBinning = reductionManager->getProperty("EnergyTransferRange");

      // Create a default set of binning parameters: (-0.5Ei, 0.01Ei, Ei)
      if (etBinning.empty())
      {
        double emin = -0.5 * eiGuess;
        double deltaE = eiGuess / 100.0;
        etBinning.push_back(emin);
        etBinning.push_back(deltaE);
        etBinning.push_back(eiGuess);
      }

      double incidentEnergy = 0.0;
      double monPeak = 0.0;
      specid_t eiMon1Spec = static_cast<specid_t>(reductionManager->getProperty("Monitor1SpecId"));
      specid_t eiMon2Spec = static_cast<specid_t>(reductionManager->getProperty("Monitor2SpecId"));

      if ("SNS" == facility)
      {
        // SNS wants to preserve events until the last
        preserveEvents = true;
        const std::string instName = inputWS->getInstrument()->getName();
        double tZero = 0.0;
        if ("CNCS" == instName || "HYSPEC" == instName)
        {
          incidentEnergy = eiGuess;
          if ("HYSPEC" == instName)
          {
            double powVal = incidentEnergy / 27.0;
            tZero = 25.0 + 85.0 / (1.0 + std::pow(powVal, 4));
          }
          // Do CNCS
          else
          {
            double powVal = 1.0 + incidentEnergy;
            tZero = (0.1982 * std::pow(powVal, -0.84098)) * 1000.0;
          }
          if (EMPTY_DBL() != tZeroGuess)
          {
            tZero = tZeroGuess;
          }
        }
        // Do ARCS and SEQUOIA
        else
        {
          if (useEiGuess)
          {
            incidentEnergy = eiGuess;
            if (EMPTY_DBL() != tZeroGuess)
            {
              tZero = tZeroGuess;
            }
          }
          else
          {
            g_log.notice() << "Trying to determine file name" << std::endl;
            std::string runFileName("");
            if (reductionManager->existsProperty("SampleMonitorFilename"))
            {
              runFileName = reductionManager->getPropertyValue("SampleMonitorFilename");
              if (runFileName.empty())
              {
                throw std::runtime_error("Cannot find run filename, therefore cannot find the initial energy");
              }
            }
            else
            {
              throw std::runtime_error("Input workspaces are not handled, therefore cannot find the initial energy");
            }

            std::string loadAlgName("");
            std::string fileProp("");
            if (boost::ends_with(runFileName, "_event.nxs"))
            {
              g_log.notice() << "Loading NeXus monitors" << std::endl;
              loadAlgName = "LoadNexusMonitors";
              fileProp = "Filename";
            }

            if (boost::ends_with(runFileName, "_neutron_event.dat"))
            {
              g_log.notice() << "Loading PreNeXus monitors" << std::endl;
              loadAlgName = "LoadPreNexusMonitors";
              boost::replace_first(runFileName, "_neutron_event.dat",
                  "_runinfo.xml");
              fileProp = "RunInfoFilename";
            }

            // Load the monitors
            IAlgorithm_sptr loadmon = this->createSubAlgorithm(loadAlgName);
            loadmon->setAlwaysStoreInADS(true);
            loadmon->setProperty(fileProp, runFileName);
            loadmon->setProperty("OutputWorkspace", monWsName);
            loadmon->executeAsSubAlg();

            reductionManager->declareProperty(new PropertyWithValue<std::string>("MonitorWorkspace", monWsName));

            // Calculate Ei
            IAlgorithm_sptr getei = this->createSubAlgorithm("GetEi");
            getei->setProperty("InputWorkspace", monWsName);
            getei->setProperty("Monitor1Spec", eiMon1Spec);
            getei->setProperty("Monitor2Spec", eiMon2Spec);
            getei->setProperty("EnergyEstimate", eiGuess);
            getei->executeAsSubAlg();
            incidentEnergy = getei->getProperty("IncidentEnergy");
            tZero = getei->getProperty("Tzero");
          }
        }

        g_log.notice() << "Adjusting for T0" << std::endl;
        IAlgorithm_sptr alg = this->createSubAlgorithm("ChangeBinOffset");
        alg->setProperty("InputWorkspace", inputWS);
        alg->setProperty("OutputWorkspace", outputWS);
        alg->setProperty("Offset", -tZero);
        alg->executeAsSubAlg();
        outputWS = alg->getProperty("OutputWorkspace");

        // Add T0 to sample logs
        IAlgorithm_sptr addLog = this->createSubAlgorithm("AddSampleLog");
        addLog->setProperty("Workspace", outputWS);
        addLog->setProperty("LogName", "CalculatedT0");
        addLog->setProperty("LogType", "Number");
        addLog->setProperty("LogText", boost::lexical_cast<std::string>(tZero));
        addLog->executeAsSubAlg();
      }
      // Do ISIS
      else
      {
        IAlgorithm_sptr getei = this->createSubAlgorithm("GetEi");
        getei->setProperty("InputWorkspace", inputWS);
        getei->setProperty("Monitor1Spec", eiMon1Spec);
        getei->setProperty("Monitor2Spec", eiMon2Spec);
        getei->setProperty("EnergyEstimate", eiGuess);
        getei->executeAsSubAlg();

        monPeak = getei->getProperty("FirstMonitorPeak");
        const specid_t monIndex = static_cast<const specid_t>(getei->getProperty("FirstMonitorIndex"));
        // Why did the old way get it from the log?
        incidentEnergy = getei->getProperty("IncidentEnergy");

        IAlgorithm_sptr cbo = this->createSubAlgorithm("ChangeBinOffset");
        cbo->setProperty("InputWorkspace", inputWS);
        cbo->setProperty("OutputWorkspace", outputWS);
        cbo->setProperty("Offset", -monPeak);
        cbo->executeAsSubAlg();
        outputWS = cbo->getProperty("OutputWorkspace");

        IDetector_const_sptr monDet = inputWS->getDetector(monIndex);
        V3D monPos = monDet->getPos();
        std::string srcName = inputWS->getInstrument()->getSource()->getName();

        IAlgorithm_sptr moveInstComp = this->createSubAlgorithm("MoveInstrumentComponent");
        moveInstComp->setProperty("Workspace", outputWS);
        moveInstComp->setProperty("ComponentName", srcName);
        moveInstComp->setProperty("X", monPos.X());
        moveInstComp->setProperty("Y", monPos.Y());
        moveInstComp->setProperty("Z", monPos.Z());
        moveInstComp->setProperty("RelativePosition", false);
        moveInstComp->executeAsSubAlg();
        outputWS = moveInstComp->getProperty("Workspace");
      }

      const double binOffset = -monPeak;

      if ("ISIS" == facility)
      {
        std::string detcalFile("");
        if (reductionManager->existsProperty("SampleDetCalFilename"))
        {
          detcalFile = reductionManager->getPropertyValue("SampleDetCalFilename");
        }
        // Try to get it from run object.
        else
        {
          detcalFile = inputWS->run().getProperty("Filename")->value();
        }
        if (!detcalFile.empty())
        {
          const bool relocateDets = reductionManager->getProperty("RelocateDetectors");
          IAlgorithm_sptr loaddetinfo = this->createSubAlgorithm("LoadDetectorInfo");
          loaddetinfo->setProperty("Workspace", outputWS);
          loaddetinfo->setProperty("DataFilename", detcalFile);
          loaddetinfo->setProperty("RelocateDets", relocateDets);
          loaddetinfo->executeAsSubAlg();
          outputWS = loaddetinfo->getProperty("Workspace");
        }
        else
        {
          throw std::runtime_error("Cannot find detcal filename in run object or as parameter.");
        }
      }

      // Subtract time-independent background if necessary
      const bool doTibSub = reductionManager->getProperty("TimeIndepBackgroundSub");
      if (doTibSub)
      {
        // Set the binning parameters for the background region
        double tibTofStart = reductionManager->getProperty("TibTofRangeStart");
        tibTofStart += binOffset;
        double tibTofEnd = reductionManager->getProperty("TibTofRangeEnd");
        tibTofEnd += binOffset;
        const double tibTofWidth = tibTofEnd - tibTofStart;
        std::vector<double> params;
        params.push_back(tibTofStart);
        params.push_back(tibTofWidth);
        params.push_back(tibTofEnd);

        if ("SNS" == facility)
        {
          // Create an original background workspace from a portion of the
          // result workspace.
          std::string origBkgWsName = "background_origin_ws";
          IAlgorithm_sptr rebin = this->createSubAlgorithm("Rebin");
          rebin->setProperty("InputWorkspace", outputWS);
          rebin->setProperty("OutputWorkspace", origBkgWsName);
          rebin->setProperty("Params", params);
          rebin->executeAsSubAlg();
          MatrixWorkspace_sptr origBkgWS = rebin->getProperty("OutputWorkspace");

          // Convert result workspace to DeltaE since we have Et binning
          IAlgorithm_sptr cnvun = this->createSubAlgorithm("ConvertUnits");
          cnvun->setProperty("InputWorkspace", outputWS);
          cnvun->setProperty("OutputWorkspace", outputWS);
          cnvun->setProperty("Target", "DeltaE");
          cnvun->setProperty("EMode", "Direct");
          cnvun->setProperty("EFixed", incidentEnergy);
          cnvun->executeAsSubAlg();
          outputWS = cnvun->getProperty("OutputWorkspace");

          // Rebin to Et
          rebin->setProperty("InputWorkspace", outputWS);
          rebin->setProperty("OutputWorkspace", outputWS);
          rebin->setProperty("Params", etBinning);
          rebin->setProperty("PreserveEvents", false);
          rebin->executeAsSubAlg();
          outputWS = rebin->getProperty("OutputWorkspace");

          // Convert result workspace to TOF
          cnvun->setProperty("InputWorkspace", outputWS);
          cnvun->setProperty("OutputWorkspace", outputWS);
          cnvun->setProperty("Target", "TOF");
          cnvun->setProperty("EMode", "Direct");
          cnvun->setProperty("EFixed", incidentEnergy);
          cnvun->executeAsSubAlg();
          outputWS = cnvun->getProperty("OutputWorkspace");

          // Make result workspace a distribution
          IAlgorithm_sptr cnvToDist = this->createSubAlgorithm("ConvertToDistribution");
          cnvToDist->setAlwaysStoreInADS(true);
          cnvToDist->setProperty("Workspace", outputWS);
          cnvToDist->executeAsSubAlg();
          outputWS = cnvToDist->getProperty("OutputWorkspace");

          // Calculate the background
          std::string bkgWsName = "background_ws";
          IAlgorithm_sptr flatBg = this->createSubAlgorithm("FlatBackground");
          flatBg->setProperty("InputWorkspace", origBkgWS);
          flatBg->setProperty("OutputWorkspace", bkgWsName);
          flatBg->setProperty("StartX", tibTofStart);
          flatBg->setProperty("EndX", tibTofEnd);
          flatBg->setProperty("Mode", "Mean");
          flatBg->setProperty("OutputMode", "Return Background");
          flatBg->executeAsSubAlg();
          MatrixWorkspace_sptr bkgWS = flatBg->getProperty("OutputWorkspace");

          // Remove unneeded original background workspace
          IAlgorithm_sptr delWs = this->createSubAlgorithm("DeleteWorkspace");
          delWs->setProperty("Workspace", origBkgWS);
          delWs->executeAsSubAlg();

          // Make background workspace a distribution
          cnvToDist->setProperty("Workspace", bkgWS);
          cnvToDist->executeAsSubAlg();
          bkgWS = flatBg->getProperty("Workspace");

          // Subtrac background from result workspace
          IAlgorithm_sptr minus = this->createSubAlgorithm("Minus");
          minus->setAlwaysStoreInADS(true);
          minus->setProperty("LHSWorkspace", outputWS);
          minus->setProperty("RHSWorkspace", bkgWS);
          minus->setProperty("OutputWorkspace", outputWS);
          minus->executeAsSubAlg();

          // Remove unneeded background workspace
          delWs->setProperty("Workspace", bkgWS);
          delWs->executeAsSubAlg();
        }
        // Do ISIS
        else
        {
          IAlgorithm_sptr flatBg = this->createSubAlgorithm("FlatBackground");
          flatBg->setProperty("InputWorkspace", outputWS);
          flatBg->setProperty("OutputWorkspace", outputWS);
          flatBg->setProperty("StartX", tibTofStart);
          flatBg->setProperty("EndX", tibTofEnd);
          flatBg->setProperty("Mode", "Mean");
          flatBg->executeAsSubAlg();
          outputWS = flatBg->getProperty("OutputWorkspace");
        }

        // Convert result workspace back to histogram
        IAlgorithm_sptr cnvFrDist = this->createSubAlgorithm("ConvertFromDistribution");
        cnvFrDist->setProperty("Workspace", outputWS);
        cnvFrDist->executeAsSubAlg();
        outputWS = cnvFrDist->getProperty("Workspace");
      }

      // Normalise result workspace to incident beam parameter
      IAlgorithm_sptr norm = this->createSubAlgorithm("DgsPreprocessData");
      norm->setProperty("InputWorkspace", outputWS);
      norm->setProperty("OutputWorkspace", outputWS);
      norm->setProperty("TofRangeOffset", binOffset);
      norm->executeAsSubAlg();
      outputWS = norm->getProperty("OutputWorkspace");

      // Convert to energy transfer
      g_log.notice() << "Converting to energy transfer." << std::endl;
      IAlgorithm_sptr cnvun = this->createSubAlgorithm("ConvertUnits");
      cnvun->setProperty("InputWorkspace", outputWS);
      cnvun->setProperty("OutputWorkspace", outputWS);
      cnvun->setProperty("Target", "DeltaE");
      cnvun->setProperty("EMode", "Direct");
      cnvun->setProperty("EFixed", incidentEnergy);
      cnvun->executeAsSubAlg();
      outputWS = cnvun->getProperty("OutputWorkspace");

      g_log.notice() << "Rebinning data" << std::endl;
      IAlgorithm_sptr rebin = this->createSubAlgorithm("Rebin");
      rebin->setProperty("InputWorkspace", outputWS);
      rebin->setProperty("OutputWorkspace", outputWS);
      rebin->setProperty("Params", etBinning);
      rebin->setProperty("PreserveEvents", preserveEvents);
      rebin->executeAsSubAlg();
      outputWS = rebin->getProperty("OutputWorkspace");

      // Correct for detector efficiency
      if ("SNS" == facility)
      {
        // He3TubeEfficiency requires the workspace to be in wavelength
        cnvun->setProperty("InputWorkspace", outputWS);
        cnvun->setProperty("OutputWorkspace", outputWS);
        cnvun->setProperty("Target", "Wavelength");
        cnvun->executeAsSubAlg();
        outputWS = cnvun->getProperty("OutputWorkspace");

        // Do the correction
        IAlgorithm_sptr alg2 = this->createSubAlgorithm("He3TubeEfficiency");
        alg2->setProperty("InputWorkspace", outputWS);
        alg2->setProperty("OutputWorkspace", outputWS);
        alg2->executeAsSubAlg();
        outputWS = alg2->getProperty("OutputWorkspace");

        // Convert back to energy transfer
        cnvun->setProperty("InputWorkspace", outputWS);
        cnvun->setProperty("OutputWorkspace", outputWS);
        cnvun->setProperty("Target", "DeltaE");
        cnvun->executeAsSubAlg();
        outputWS = cnvun->getProperty("OutputWorkspace");
      }
      // Do ISIS
      else
      {
        IAlgorithm_sptr alg = this->createSubAlgorithm("DetectorEfficiencyCor");
        alg->setProperty("InputWorkspace", outputWS);
        alg->setProperty("OutputWorkspace", outputWS);
        alg->executeAsSubAlg();
        outputWS = alg->getProperty("OutputWorkspace");
      }

      const bool correctKiKf = reductionManager->getProperty("CorrectKiKf");
      if (correctKiKf)
      {
        // Correct for Ki/Kf
        IAlgorithm_sptr kikf = this->createSubAlgorithm("CorrectKiKf");
        kikf->setProperty("InputWorkspace", outputWS);
        kikf->setProperty("OutputWorkspace", outputWS);
        kikf->setProperty("EMode", "Direct");
        kikf->executeAsSubAlg();
        outputWS = kikf->getProperty("OutputWorkspace");
      }

      // Mask and group workspace if necessary.
      MatrixWorkspace_sptr maskWS = this->getProperty("MaskWorkspace");
      MatrixWorkspace_sptr groupWS = this->getProperty("GroupingWorkspace");
      std::string oldGroupFile("");
      std::string filePropMod = this->getProperty("AlternateGroupingTag");
      std::string fileProp = filePropMod + "OldGroupingFilename";
      if (reductionManager->existsProperty(fileProp))
      {
        oldGroupFile = reductionManager->getPropertyValue(fileProp);
      }
      IAlgorithm_sptr remap = this->createSubAlgorithm("DgsRemap");
      remap->setProperty("InputWorkspace", outputWS);
      remap->setProperty("OutputWorkspace", outputWS);
      remap->setProperty("MaskWorkspace", maskWS);
      remap->setProperty("GroupingWorkspace", groupWS);
      remap->setProperty("OldGroupingFile", oldGroupFile);
      if (reductionManager->existsProperty("UseProcessedDetVan"))
      {
        bool runOpposite = reductionManager->getProperty("UseProcessedDetVan");
        remap->setProperty("ExecuteOppositeOrder", runOpposite);
      }
      remap->executeAsSubAlg();
      outputWS = remap->getProperty("OutputWorkspace");

      // Rebin to ensure consistency
      const bool sofphieIsDistribution = reductionManager->getProperty("SofPhiEIsDistribution");

      g_log.notice() << "Rebinning data" << std::endl;
      rebin->setProperty("InputWorkspace", outputWS);
      rebin->setProperty("OutputWorkspace", outputWS);
      if (sofphieIsDistribution)
      {
        rebin->setProperty("PreserveEvents", false);
      }
      rebin->executeAsSubAlg();
      outputWS = rebin->getProperty("OutputWorkspace");

      if (sofphieIsDistribution)
      {
        g_log.notice() << "Making distribution" << std::endl;
        IAlgorithm_sptr distrib = this->createSubAlgorithm("ConvertToDistribution");
        distrib->setProperty("Workspace", outputWS);
        distrib->executeAsSubAlg();
        outputWS = distrib->getProperty("Workspace");
      }

      // Normalise by the detector vanadium if necessary
      MatrixWorkspace_sptr detVanWS = this->getProperty("IntegratedDetectorVanadium");
      if (detVanWS)
      {
        IAlgorithm_sptr divide = this->createSubAlgorithm("Divide");
        divide->setProperty("LHSWorkspace", outputWS);
        divide->setProperty("RHSWorkspace", detVanWS);
        divide->setProperty("OutputWorkspace", outputWS);
        divide->executeAsSubAlg();
        outputWS = divide->getProperty("OutputWorkspace");
      }

      // Correct for solid angle if grouping is requested, but detector vanadium
      // not used.
      if (groupWS && !detVanWS)
      {
        std::string solidAngWsName = "SolidAngle";
        IAlgorithm_sptr solidAngle = this->createSubAlgorithm("SolidAngle");
        solidAngle->setProperty("InputWorkspace", outputWS);
        solidAngle->setProperty("OutputWorkspace", solidAngWsName);
        solidAngle->executeAsSubAlg();
        MatrixWorkspace_sptr solidAngWS = solidAngle->getProperty("OutputWorkspace");

        IAlgorithm_sptr divide = this->createSubAlgorithm("Divide");
        divide->setProperty("LHSWorkspace", outputWS);
        divide->setProperty("RHSWorkspace", solidAngWS);
        divide->setProperty("OutputWorkspace", outputWS);
        divide->executeAsSubAlg();
        outputWS = divide->getProperty("OutputWorkspace");

        solidAngWS.reset();
      }

      if ("ISIS" == facility)
      {
        double scaleFactor = inputWS->getInstrument()->getNumberParameter("scale-factor")[0];
        const std::string scaleFactorName = "ScaleFactor";
        IAlgorithm_sptr csvw = this->createSubAlgorithm("CreateSingleValuedWorkspace");
        csvw->setProperty("OutputWorkspace", scaleFactorName);
        csvw->setProperty("DataValue", scaleFactor);
        csvw->executeAsSubAlg();
        MatrixWorkspace_sptr scaleFactorWS = csvw->getProperty("OutputWorkspace");

        IAlgorithm_sptr mult = this->createSubAlgorithm("Multiply");
        mult->setProperty("LHSWorkspace", outputWS);
        mult->setProperty("RHSWorkspace", scaleFactorWS);
        mult->setProperty("OutputWorkspace", outputWS);
        mult->executeAsSubAlg();
      }

      this->setProperty("OutputWorkspace", outputWS);
    }

  } // namespace Mantid
} // namespace WorkflowAlgorithms
