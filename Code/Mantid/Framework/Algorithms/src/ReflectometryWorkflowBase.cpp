#include "MantidAPI/WorkspaceValidators.h"
#include "MantidKernel/MandatoryValidator.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/RebinParamsValidator.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidAlgorithms/ReflectometryWorkflowBase.h"

#include <boost/assign/list_of.hpp>

using namespace Mantid::API;
using namespace Mantid::Kernel;

namespace Mantid
{
  namespace Algorithms
  {
    namespace
    {
      /**
       * Helper non-member function for translating all the workspace indexes in an origin workspace into workspace indexes
       * of a host end-point workspace. This is done using spectrum numbers as the intermediate.
       *
       * This function will throw a runtime error if the specId are not found to exist on the host end-point workspace.
       *
       * @param originWS : Origin workspace, which provides the original workspace index to spectrum id mapping.
       * @param hostWS : Workspace onto which the resulting workspace indexes will be hosted
       * @return Remapped wokspace indexes applicable for the host workspace.
       */
      ReflectometryWorkflowBase::WorkspaceIndexList createWorkspaceIndexListFromDetectorWorkspace(
          MatrixWorkspace_const_sptr originWS, MatrixWorkspace_const_sptr hostWS)
      {
        auto spectrumMap = originWS->getSpectrumToWorkspaceIndexMap();
        ReflectometryWorkflowBase::WorkspaceIndexList translatedIndexList;
        for (auto it = spectrumMap.begin(); it != spectrumMap.end(); ++it)
        {
          specid_t specId = (*it).first;
          translatedIndexList.push_back(static_cast<int>(hostWS->getIndexFromSpectrumNumber(specId))); // Could be slow to do it like this.
        }
        return translatedIndexList;
      }

      /**
       *  Helper method used with the stl to determine whether values are negative
       * @param value : Value to check
       * @return : True if negative.
       */
      bool checkNotPositive(const int value)
      {
        return value < 0;
      }
    }

    //----------------------------------------------------------------------------------------------
    /** Constructor
     */
    ReflectometryWorkflowBase::ReflectometryWorkflowBase()
    {
    }

    //----------------------------------------------------------------------------------------------
    /** Destructor
     */
    ReflectometryWorkflowBase::~ReflectometryWorkflowBase()
    {
    }

    /**
     *
     */
    void ReflectometryWorkflowBase::initIndexInputs()
    {

      boost::shared_ptr<CompositeValidator> mandatoryWorkspaceIndex = boost::make_shared<
          CompositeValidator>();
      mandatoryWorkspaceIndex->add(boost::make_shared<MandatoryValidator<int> >());
      auto boundedIndex = boost::make_shared<BoundedValidator<int> >();
      boundedIndex->setLower(0);
      mandatoryWorkspaceIndex->add(boundedIndex);

      declareProperty(
          new PropertyWithValue<int>("I0MonitorIndex", Mantid::EMPTY_INT(), mandatoryWorkspaceIndex),
          "I0 monitor index");

      declareProperty(new ArrayProperty<int>("WorkspaceIndexList"),
          "Indices of the spectra in pairs (lower, upper) that mark the ranges that correspond to detectors of interest.");

    }

    /**
     * Init common wavlength inputs.
     */
    void ReflectometryWorkflowBase::initWavelengthInputs()
    {
      declareProperty(
          new PropertyWithValue<double>("WavelengthMin", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength minimum in angstroms");

      declareProperty(
          new PropertyWithValue<double>("WavelengthMax", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength maximum in angstroms");

      declareProperty(
          new PropertyWithValue<double>("WavelengthStep", 0.05,
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength rebinning step in angstroms. Defaults to 0.05. Used for rebinning intermediate workspaces converted into wavelength.");

      declareProperty(
          new PropertyWithValue<double>("MonitorBackgroundWavelengthMin", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength minimum for monitor background in angstroms.");

      declareProperty(
          new PropertyWithValue<double>("MonitorBackgroundWavelengthMax", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength maximum for monitor background in angstroms.");

      declareProperty(
          new PropertyWithValue<double>("MonitorIntegrationWavelengthMin", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength minimum for integration in angstroms.");
      declareProperty(
          new PropertyWithValue<double>("MonitorIntegrationWavelengthMax", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength maximum for integration in angstroms.");
    }

    void ReflectometryWorkflowBase::initStitchingInputs()
    {
      declareProperty(
          new ArrayProperty<double>("Params", boost::make_shared<RebinParamsValidator>(true)),
          "A comma separated list of first bin boundary, width, last bin boundary. "
              "These parameters are used for stitching together transmission runs. "
              "Values are in q. This input is only needed if a SecondTransmission run is provided.");

      declareProperty(
          new PropertyWithValue<double>("StartOverlapQ", Mantid::EMPTY_DBL(), Direction::Input),
          "Start Q for stitching transmission runs together");

      declareProperty(
          new PropertyWithValue<double>("EndOverlapQ", Mantid::EMPTY_DBL(), Direction::Input),
          "End Q for stitching transmission runs together");

    }

    /**
     * Determine if the property value is the same as the default value.
     * This can be used to determine if the property has not been set.
     * @param propertyName : Name of property to query
     * @return: True only if the property has it's default value.
     */
    bool ReflectometryWorkflowBase::isPropertyDefault(const std::string& propertyName) const
    {
      Property* property = this->getProperty(propertyName);
      return property->isDefault();
    }

    /**
     * Get the workspace index list
     * @return Workspace index list.
     */
    ReflectometryWorkflowBase::WorkspaceIndexList ReflectometryWorkflowBase::getWorkspaceIndexList() const
    {
      WorkspaceIndexList indexList = getProperty("WorkspaceIndexList");
      if (indexList.size() % 2 != 0 || indexList.size() == 0)
      {
        throw std::invalid_argument(
            "WorkspaceIndex list must be composed of pairs of min index, max index.");
      }

      if (std::find_if(indexList.begin(), indexList.end(), checkNotPositive) != indexList.end())
      {
        throw std::invalid_argument("WorkspaceIndexList contains negative indexes");
      }

      for (size_t i = 0; (i + 1) < indexList.size(); i += 2)
      {
        if (indexList[i] > indexList[i + 1])
          throw std::invalid_argument("WorkspaceIndexList pairs must be in min, max order");
      }
      return indexList;
    }

    /**
     * Fetch min, max inputs as a vector (int) if they are non-default and set them to the optionalUpperLower object.
     * Performs checks to verify that invalid indexes have not been passed in.
     * @param propertyName : Property name to fetch
     * @param isPointDetector : Flag indicates that the execution is in point detector mode.
     * @param optionalUpperLower : Object to set min and max on.
     */
    void ReflectometryWorkflowBase::fetchOptionalLowerUpperPropertyValue(const std::string& propertyName,
        bool isPointDetector, OptionalWorkspaceIndexes& optionalUpperLower) const
    {
      if (!isPropertyDefault(propertyName))
      {
        // Validation of property inputs.
        if (isPointDetector)
        {
          throw std::invalid_argument(
              "Cannot have a region of interest property in point detector mode.");
        }
        std::vector<int> temp = this->getProperty(propertyName);
        if (temp.size() != 2)
        {
          const std::string message = propertyName + " requires a lower and upper boundary";
          throw std::invalid_argument(message);
        }
        if (temp[0] > temp[1])
        {
          throw std::invalid_argument("Min must be <= Max index");
        }
        if (std::find_if(temp.begin(), temp.end(), checkNotPositive) != temp.end())
        {
          const std::string message = propertyName + " contains negative indexes";
          throw std::invalid_argument(message);
        }
        // Assignment
        optionalUpperLower = temp;
      }
    }

    /**
     * Get min max pairs as a tuple.
     * @param minProperty : Property name for the min property
     * @param maxProperty : Property name for the max property
     * @return A tuple consisting of min, max
     */
    ReflectometryWorkflowBase::MinMax ReflectometryWorkflowBase::getMinMax(
        const std::string& minProperty, const std::string& maxProperty) const
    {
      const double min = getProperty(minProperty);
      const double max = getProperty(maxProperty);
      if (min > max)
      {
        throw std::invalid_argument("Cannot have any WavelengthMin > WavelengthMax");
      }
      return MinMax(min, max);
    }

    /**
     * Validate the transmission workspace inputs when a second transmission run is provided.
     * Throws if any of the property values do not make sense.
     */
    void ReflectometryWorkflowBase::validateTransmissionInputs() const
    {
      // Verify that all the required inputs for the second transmission run are now given.
      if (isPropertyDefault("FirstTransmissionRun"))
      {
        throw std::invalid_argument(
            "A SecondTransmissionRun is only valid if a FirstTransmissionRun is provided.");
      }
      if (isPropertyDefault("Params"))
      {
        throw std::invalid_argument(
            "If a SecondTransmissionRun has been given, then stitching Params for the transmission runs are also required.");
      }
      if (isPropertyDefault("StartOverlapQ"))
      {
        throw std::invalid_argument(
            "If a SecondTransmissionRun has been given, then a stitching StartOverlapQ for the transmission runs is also required.");
      }
      if (isPropertyDefault("EndOverlapQ"))
      {
        throw std::invalid_argument(
            "If a SecondTransmissionRun has been given, then a stitching EndOverlapQ for the transmission runs is also required.");
      }
      const double startOverlapQ = this->getProperty("StartOverlapQ");
      const double endOverlapQ = this->getProperty("EndOverlapQ");
      if (startOverlapQ >= endOverlapQ)
      {
        throw std::invalid_argument("EndOverlapQ must be > StartOverlapQ");
      }

    }

    /**
     * Get the transmission run information.
     *
     * Transmission runs are optional, but you cannot have the second without the first. Also, stitching
     * parameters are required if the second is present. This getter fetches and assigns to the optional reference arguments
     *
     * @param firstTransmissionRun
     * @param secondTransmissionRun
     * @param stitchingStartQ
     * @param stitchingDeltaQ
     * @param stitchingEndQ
     */
    void ReflectometryWorkflowBase::getTransmissionRunInfo(
        OptionalMatrixWorkspace_sptr& firstTransmissionRun,
        OptionalMatrixWorkspace_sptr& secondTransmissionRun, OptionalDouble& stitchingStartQ,
        OptionalDouble& stitchingDeltaQ, OptionalDouble& stitchingEndQ,
        OptionalDouble& stitchingStartOverlapQ, OptionalDouble& stitchingEndOverlapQ) const
    {
      if (!isPropertyDefault("FirstTransmissionRun"))
      {
        MatrixWorkspace_sptr temp = this->getProperty("FirstTransmissionRun");
        firstTransmissionRun = temp;
      }

      if (!isPropertyDefault("SecondTransmissionRun"))
      {
        // Check that the property values provided make sense together.
        validateTransmissionInputs();

        // Set the values.
        {
          MatrixWorkspace_sptr temp = this->getProperty("SecondTransmissionRun");
          secondTransmissionRun = temp;
        }
        {
          std::vector<double> params = getProperty("Params");
          stitchingStartQ = params[0];
          stitchingDeltaQ = params[1];
          stitchingEndQ = params[2];
        }
        {
          double temp = this->getProperty("StartOverlapQ");
          stitchingStartOverlapQ = temp;
          temp = this->getProperty("EndOverlapQ");
          stitchingEndOverlapQ = temp;
        }
      }

    }

    /**
     * Convert the TOF workspace into a monitor workspace. Crops to the monitorIndex and applying flat background correction as part of the process.
     * @param toConvert : TOF wavlength to convert.
     * @param monitorIndex : Monitor index to crop to
     * @param backgroundMinMax : Min and Max Lambda range for Flat background correction.
     * @return The cropped and corrected monitor workspace.
     */
    MatrixWorkspace_sptr ReflectometryWorkflowBase::toLamMonitor(const MatrixWorkspace_sptr& toConvert,
        const int monitorIndex, const MinMax& backgroundMinMax)
    {
      // Convert Units.
      auto convertUnitsAlg = this->createChildAlgorithm("ConvertUnits");
      convertUnitsAlg->initialize();
      convertUnitsAlg->setProperty("InputWorkspace", toConvert);
      convertUnitsAlg->setProperty("Target", "Wavelength");
      convertUnitsAlg->setProperty("AlignBins", true);
      convertUnitsAlg->execute();

      // Crop the to the monitor index.
      MatrixWorkspace_sptr monitorWS = convertUnitsAlg->getProperty("OutputWorkspace");
      auto cropWorkspaceAlg = this->createChildAlgorithm("CropWorkspace");
      cropWorkspaceAlg->initialize();
      cropWorkspaceAlg->setProperty("InputWorkspace", monitorWS);
      cropWorkspaceAlg->setProperty("StartWorkspaceIndex", monitorIndex);
      cropWorkspaceAlg->setProperty("EndWorkspaceIndex", monitorIndex);
      cropWorkspaceAlg->execute();
      monitorWS = cropWorkspaceAlg->getProperty("OutputWorkspace");

      // Flat background correction
      auto correctMonitorsAlg = this->createChildAlgorithm("CalculateFlatBackground");
      correctMonitorsAlg->initialize();
      correctMonitorsAlg->setProperty("InputWorkspace", monitorWS);
      correctMonitorsAlg->setProperty("WorkspaceIndexList",
          boost::assign::list_of(0).convert_to_container<std::vector<int> >());
      correctMonitorsAlg->setProperty("StartX", backgroundMinMax.get<0>());
      correctMonitorsAlg->setProperty("EndX", backgroundMinMax.get<1>());
      correctMonitorsAlg->execute();
      monitorWS = correctMonitorsAlg->getProperty("OutputWorkspace");

      return monitorWS;
    }

    /**
     * Convert to a detector workspace in lambda.
     * @param detectorIndexRange : Workspace index ranges to keep
     * @param toConvert : TOF wavelength to convert.
     * @param wavelengthMinMax : Wavelength minmax to keep. Crop out the rest.
     * @param wavelengthStep : Wavelength step for rebinning
     * @return Detector workspace in wavelength
     */
    MatrixWorkspace_sptr ReflectometryWorkflowBase::toLamDetector(
        const WorkspaceIndexList& detectorIndexRange, const MatrixWorkspace_sptr& toConvert,
        const MinMax& wavelengthMinMax, const double& wavelengthStep)
    {
      // Detector Workspace Processing
      MatrixWorkspace_sptr detectorWS;

      // Loop over pairs of detector index ranges. Peform the cropping and then conjoin the results into a single workspace.
      for (size_t i = 0; i < detectorIndexRange.size(); i += 2)
      {
        auto cropWorkspaceAlg = this->createChildAlgorithm("CropWorkspace");
        cropWorkspaceAlg->initialize();
        cropWorkspaceAlg->setProperty("InputWorkspace", toConvert);
        cropWorkspaceAlg->setProperty("StartWorkspaceIndex", detectorIndexRange[i]);
        cropWorkspaceAlg->setProperty("EndWorkspaceIndex", detectorIndexRange[i + 1]);
        cropWorkspaceAlg->execute();
        MatrixWorkspace_sptr subRange = cropWorkspaceAlg->getProperty("OutputWorkspace");
        if (i == 0)
        {
          detectorWS = subRange;
        }
        else
        {
          auto conjoinWorkspaceAlg = this->createChildAlgorithm("ConjoinWorkspaces");
          conjoinWorkspaceAlg->initialize();
          conjoinWorkspaceAlg->setProperty("InputWorkspace1", detectorWS);
          conjoinWorkspaceAlg->setProperty("InputWorkspace2", subRange);
          conjoinWorkspaceAlg->execute();
          detectorWS = conjoinWorkspaceAlg->getProperty("InputWorkspace1");
        }
      }
      // Now convert units. Do this after the conjoining step otherwise the x bins will not match up.
      auto convertUnitsAlg = this->createChildAlgorithm("ConvertUnits");
      convertUnitsAlg->initialize();
      convertUnitsAlg->setProperty("InputWorkspace", detectorWS);
      convertUnitsAlg->setProperty("Target", "Wavelength");
      convertUnitsAlg->setProperty("AlignBins", true);
      convertUnitsAlg->execute();
      detectorWS = convertUnitsAlg->getProperty("OutputWorkspace");

      // Crop out the lambda x-ranges now that the workspace is in wavelength.
      auto cropWorkspaceAlg = this->createChildAlgorithm("CropWorkspace");
      cropWorkspaceAlg->initialize();
      cropWorkspaceAlg->setProperty("InputWorkspace", detectorWS);
      cropWorkspaceAlg->setProperty("XMin", wavelengthMinMax.get<0>());
      cropWorkspaceAlg->setProperty("XMax", wavelengthMinMax.get<1>());
      cropWorkspaceAlg->execute();
      detectorWS = cropWorkspaceAlg->getProperty("OutputWorkspace");

      auto rebinWorkspaceAlg = this->createChildAlgorithm("Rebin");
      rebinWorkspaceAlg->initialize();
      std::vector<double> params = boost::assign::list_of(wavelengthStep);
      rebinWorkspaceAlg->setProperty("Params", params);
      rebinWorkspaceAlg->setProperty("InputWorkspace", detectorWS);
      rebinWorkspaceAlg->execute();
      detectorWS = rebinWorkspaceAlg->getProperty("OutputWorkspace");

      return detectorWS;
    }

    /**
     * Convert From a TOF workspace into a detector and monitor workspace both in Lambda.
     * @param toConvert: TOF workspace to convert
     * @param detectorIndexRange : Detector index ranges
     * @param monitorIndex : Monitor index
     * @param wavelengthMinMax : Wavelength min max for detector workspace
     * @param backgroundMinMax : Wavelength min max for flat background correction of monitor workspace
     * @param wavelengthStep : Wavlength step size for rebinning.
     * @return Tuple of detector and monitor workspaces
     */
    ReflectometryWorkflowBase::DetectorMonitorWorkspacePair ReflectometryWorkflowBase::toLam(
        MatrixWorkspace_sptr toConvert, const WorkspaceIndexList& detectorIndexRange,
        const int monitorIndex, const MinMax& wavelengthMinMax, const MinMax& backgroundMinMax,
        const double& wavelengthStep)
    {
      // Detector Workspace Processing
      MatrixWorkspace_sptr detectorWS = toLamDetector(detectorIndexRange, toConvert, wavelengthMinMax,
          wavelengthStep);

      // Monitor Workspace Processing
      MatrixWorkspace_sptr monitorWS = toLamMonitor(toConvert, monitorIndex, backgroundMinMax);

      // Rebin the Monitor Workspace to match the Detector Workspace.
      auto rebinToWorkspaceAlg = this->createChildAlgorithm("RebinToWorkspace");
      rebinToWorkspaceAlg->initialize();
      rebinToWorkspaceAlg->setProperty("WorkspaceToRebin", monitorWS);
      rebinToWorkspaceAlg->setProperty("WorkspaceToMatch", detectorWS);
      rebinToWorkspaceAlg->execute();
      monitorWS = rebinToWorkspaceAlg->getProperty("OutputWorkspace");

      return DetectorMonitorWorkspacePair(detectorWS, monitorWS);
    }


    /**
     * Create a transmission corrections workspace utilising one or two workspaces.
     *
     * Input workspaces are in TOF. These are converted to lambda, normalized and stitched together (if two given).
     *
     * @param IvsLam : Run workspace which is to be normalized by the results of the transmission corrections.
     * @param wavelengthInterval : Wavelength interval for the run workspace.
     * @param wavelengthMonitorBackgroundInterval : Wavelength interval for the monitor background
     * @param wavelengthMonitorIntegrationInterval : Wavelength interval for the monitor integration
     * @param i0MonitorIndex : Monitor index for the I0 monitor
     * @param firstTransmissionRun : The first transmission run
     * @param secondTransmissionRun : The second transmission run (optional)
     * @param stitchingStartQ : Stitching start Q (optional but dependent on secondTransmissionRun)
     * @param stitchingDeltaQ : Stitching delta Q (optional but dependent on secondTransmissionRun)
     * @param stitchingEndQ : Stitching end Q (optional but dependent on secondTransmissionRun)
     * @param stitchingStartOverlapQ : Stitching start Q overlap (optional but dependent on secondTransmissionRun)
     * @param stitchingEndOverlapQ : Stitching end Q overlap (optional but dependent on secondTransmissionRun)
     * @param wavelengthStep : Step in angstroms for rebinning for workspaces converted into wavelength.
     * @return A transmission workspace in Wavelength units.
     */
    MatrixWorkspace_sptr ReflectometryWorkflowBase::makeTransmissionCorrection(
        const WorkspaceIndexList& detectorIndexes,
        const MinMax& wavelengthInterval,
        const MinMax& wavelengthMonitorBackgroundInterval,
        const MinMax& wavelengthMonitorIntegrationInterval,
        const int& i0MonitorIndex,
        MatrixWorkspace_sptr firstTransmissionRun,
        OptionalMatrixWorkspace_sptr secondTransmissionRun,
        const OptionalDouble& stitchingStartQ,
        const OptionalDouble& stitchingDeltaQ,
        const OptionalDouble& stitchingEndQ,
        const OptionalDouble& stitchingStartOverlapQ,
        const OptionalDouble& stitchingEndOverlapQ,
        const double& wavelengthStep)
    {
      auto trans1InLam = toLam(firstTransmissionRun, detectorIndexes, i0MonitorIndex, wavelengthInterval,
          wavelengthMonitorBackgroundInterval, wavelengthStep);
      MatrixWorkspace_sptr trans1Detector = trans1InLam.get<0>();
      MatrixWorkspace_sptr trans1Monitor = trans1InLam.get<1>();

      // Monitor integration ... can this happen inside the toLam routine?
      auto integrationAlg = this->createChildAlgorithm("Integration");
      integrationAlg->initialize();
      integrationAlg->setProperty("InputWorkspace", trans1Monitor);
      integrationAlg->setProperty("RangeLower", wavelengthMonitorIntegrationInterval.get<0>());
      integrationAlg->setProperty("RangeUpper", wavelengthMonitorIntegrationInterval.get<1>());
      integrationAlg->execute();
      trans1Monitor = integrationAlg->getProperty("OutputWorkspace");

      MatrixWorkspace_sptr transmissionWS = trans1Detector / trans1Monitor;

      if (secondTransmissionRun.is_initialized())
      {
        // TODO check that the spectra to workspace index maps match for both First and Second transmsisson runs. If they don't THROW because the detectorIndexList won't work.

        auto transRun2 = secondTransmissionRun.get();
        g_log.debug("Extracting second transmission run workspace indexes from spectra");

        auto trans2InLam = toLam(transRun2, detectorIndexes, i0MonitorIndex, wavelengthInterval,
            wavelengthMonitorBackgroundInterval, wavelengthStep);

        // Unpack the conversion results.
        MatrixWorkspace_sptr trans2Detector = trans2InLam.get<0>();
        MatrixWorkspace_sptr trans2Monitor = trans2InLam.get<1>();

        // Monitor integration ... can this happen inside the toLam routine?
        auto integrationAlg = this->createChildAlgorithm("Integration");
        integrationAlg->initialize();
        integrationAlg->setProperty("InputWorkspace", trans2Monitor);
        integrationAlg->setProperty("RangeLower", wavelengthMonitorIntegrationInterval.get<0>());
        integrationAlg->setProperty("RangeUpper", wavelengthMonitorIntegrationInterval.get<1>());
        integrationAlg->execute();
        trans2Monitor = integrationAlg->getProperty("OutputWorkspace");

        MatrixWorkspace_sptr normalizedTrans2 = trans2Detector / trans2Monitor;

        // Stitch the results.
        auto stitch1DAlg = this->createChildAlgorithm("Stitch1D");
        stitch1DAlg->initialize();
        AnalysisDataService::Instance().addOrReplace("transmissionWS", transmissionWS);
        AnalysisDataService::Instance().addOrReplace("normalizedTrans2", normalizedTrans2);
        stitch1DAlg->setProperty("LHSWorkspace", transmissionWS);
        stitch1DAlg->setProperty("RHSWorkspace", normalizedTrans2);
        stitch1DAlg->setProperty("StartOverlap", stitchingStartOverlapQ.get());
        stitch1DAlg->setProperty("EndOverlap", stitchingEndOverlapQ.get());
        const std::vector<double> params = boost::assign::list_of(stitchingStartQ.get())(
            stitchingDeltaQ.get())(stitchingEndQ.get()).convert_to_container<std::vector<double> >();
        stitch1DAlg->setProperty("Params", params);
        stitch1DAlg->execute();
        transmissionWS = stitch1DAlg->getProperty("OutputWorkspace");
        AnalysisDataService::Instance().remove("transmissionWS");
        AnalysisDataService::Instance().remove("normalizedTrans2");
      }

      return transmissionWS;
    }


    /**
     * Perform Transmission Corrections.
     * @param IvsLam : Run workspace which is to be normalized by the results of the transmission corrections.
     * @param wavelengthInterval : Wavelength interval for the run workspace.
     * @param wavelengthMonitorBackgroundInterval : Wavelength interval for the monitor background
     * @param wavelengthMonitorIntegrationInterval : Wavelength interval for the monitor integration
     * @param i0MonitorIndex : Monitor index for the I0 monitor
     * @param firstTransmissionRun : The first transmission run
     * @param secondTransmissionRun : The second transmission run (optional)
     * @param stitchingStartQ : Stitching start Q (optional but dependent on secondTransmissionRun)
     * @param stitchingDeltaQ : Stitching delta Q (optional but dependent on secondTransmissionRun)
     * @param stitchingEndQ : Stitching end Q (optional but dependent on secondTransmissionRun)
     * @param stitchingStartOverlapQ : Stitching start Q overlap (optional but dependent on secondTransmissionRun)
     * @param stitchingEndOverlapQ : Stitching end Q overlap (optional but dependent on secondTransmissionRun)
     * @param wavelengthStep : Step in angstroms for rebinning for workspaces converted into wavelength.
     * @return Normalized run workspace by the transmission workspace, which have themselves been converted to Lam, normalized by monitors and possibly stitched together.
     */
    MatrixWorkspace_sptr ReflectometryWorkflowBase::transmissonCorrection(MatrixWorkspace_sptr IvsLam,
        const MinMax& wavelengthInterval, const MinMax& wavelengthMonitorBackgroundInterval,
        const MinMax& wavelengthMonitorIntegrationInterval, const int& i0MonitorIndex,
        MatrixWorkspace_sptr firstTransmissionRun, OptionalMatrixWorkspace_sptr secondTransmissionRun,
        const OptionalDouble& stitchingStartQ, const OptionalDouble& stitchingDeltaQ,
        const OptionalDouble& stitchingEndQ, const OptionalDouble& stitchingStartOverlapQ,
        const OptionalDouble& stitchingEndOverlapQ, const double& wavelengthStep)
    {
      g_log.debug("Extracting first transmission run workspace indexes from spectra");
      const WorkspaceIndexList detectorIndexes = createWorkspaceIndexListFromDetectorWorkspace(IvsLam,
          firstTransmissionRun);

      // Make the transmission run.
      MatrixWorkspace_sptr denominator = makeTransmissionCorrection(detectorIndexes, wavelengthInterval,
          wavelengthMonitorBackgroundInterval, wavelengthMonitorIntegrationInterval, i0MonitorIndex,
          firstTransmissionRun, secondTransmissionRun, stitchingStartQ, stitchingDeltaQ, stitchingEndQ,
          stitchingStartOverlapQ, stitchingEndOverlapQ, wavelengthStep);

      // Rebin the transmission run to be the same as the input.
      auto rebinToWorkspaceAlg = this->createChildAlgorithm("RebinToWorkspace");
      rebinToWorkspaceAlg->initialize();
      rebinToWorkspaceAlg->setProperty("WorkspaceToMatch", IvsLam);
      rebinToWorkspaceAlg->setProperty("WorkspaceToRebin", denominator);
      rebinToWorkspaceAlg->execute();
      denominator = rebinToWorkspaceAlg->getProperty("OutputWorkspace");

      // Do normalization.
      MatrixWorkspace_sptr normalizedIvsLam = IvsLam / denominator;
      return normalizedIvsLam;
    }

  } // namespace Algorithms
} // namespace Mantid
