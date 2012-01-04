/*WIKI* 



This algorithm starts with a PeaksWorkspace containing the expected positions of peaks in reciprocal space. It calculates the centroid of the peak by calculating the average of the coordinates of all events within a given radius of the peak, weighted by the weight (signal) of the event.





*WIKI*/
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidKernel/System.h"
#include "MantidMDEvents/CoordTransformDistance.h"
#include "MantidMDEvents/CentroidPeaksMD.h"
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidMDEvents/IntegratePeaksMD.h"

using Mantid::DataObjects::PeaksWorkspace;

namespace Mantid
{
namespace MDEvents
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(CentroidPeaksMD)
  
  using namespace Mantid::API;
  using namespace Mantid::DataObjects;
  using namespace Mantid::Geometry;
  using namespace Mantid::Kernel;
  using namespace Mantid::MDEvents;


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  CentroidPeaksMD::CentroidPeaksMD()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  CentroidPeaksMD::~CentroidPeaksMD()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void CentroidPeaksMD::initDocs()
  {
    this->setWikiSummary("Find the centroid of single-crystal peaks in a MDEventWorkspace, in order to refine their positions.");
    this->setOptionalMessage("Find the centroid of single-crystal peaks in a MDEventWorkspace, in order to refine their positions.");
  }

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void CentroidPeaksMD::init()
  {
    declareProperty(new WorkspaceProperty<IMDEventWorkspace>("InputWorkspace","",Direction::Input), "An input MDEventWorkspace.");

    std::vector<std::string> propOptions;
    propOptions.push_back("Q (lab frame)");
    propOptions.push_back("Q (sample frame)");
    propOptions.push_back("HKL");
    declareProperty("CoordinatesToUse", "HKL",new ListValidator(propOptions),
      "Which coordinates of the peak center do you wish to use to find the center? This should match the InputWorkspace's dimensions."
       );

    declareProperty(new PropertyWithValue<double>("PeakRadius",1.0,Direction::Input),
        "Fixed radius around each peak position in which to calculate the centroid.");

    declareProperty(new WorkspaceProperty<PeaksWorkspace>("PeaksWorkspace","",Direction::Input),
        "A PeaksWorkspace containing the peaks to centroid.");

    declareProperty(new WorkspaceProperty<PeaksWorkspace>("OutputWorkspace","",Direction::Output),
        "The output PeaksWorkspace will be a copy of the input PeaksWorkspace "
        "with the peaks' positions modified by the new found centroids.");
  }

  //----------------------------------------------------------------------------------------------
  /** Integrate the peaks of the workspace using parameters saved in the algorithm class
   * @param ws ::  MDEventWorkspace to integrate
   */
  template<typename MDE, size_t nd>
  void CentroidPeaksMD::integrate(typename MDEventWorkspace<MDE, nd>::sptr ws)
  {
    if (nd != 3)
      throw std::invalid_argument("For now, we expect the input MDEventWorkspace to have 3 dimensions only.");

    /// Peak workspace to centroid
    Mantid::DataObjects::PeaksWorkspace_sptr inPeakWS = getProperty("PeaksWorkspace");

    /// Output peaks workspace, create if needed
    Mantid::DataObjects::PeaksWorkspace_sptr peakWS = getProperty("OutputWorkspace");
    if (peakWS != inPeakWS)
      peakWS = inPeakWS->clone();

    /// Value of the CoordinatesToUse property.
    std::string CoordinatesToUse = getPropertyValue("CoordinatesToUse");

    // TODO: Confirm that the coordinates requested match those in the MDEventWorkspace

    /// Radius to use around peaks
    double PeakRadius = getProperty("PeakRadius");

    // cppcheck-suppress syntaxError
    PRAGMA_OMP(parallel for schedule(dynamic, 10) )
    for (int i=0; i < int(peakWS->getNumberPeaks()); ++i)
    {
      // Get a direct ref to that peak.
      IPeak & p = peakWS->getPeak(i);
      double detectorDistance = p.getL2();

      // Get the peak center as a position in the dimensions of the workspace
      V3D pos;
      if (CoordinatesToUse == "Q (lab frame)")
        pos = p.getQLabFrame();
      else if (CoordinatesToUse == "Q (sample frame)")
        pos = p.getQSampleFrame();
      else if (CoordinatesToUse == "HKL")
        pos = p.getHKL();

      // Build the sphere transformation
      bool dimensionsUsed[nd];
      coord_t center[nd];
      for (size_t d=0; d<nd; ++d)
      {
        dimensionsUsed[d] = true; // Use all dimensions
        center[d] = pos[d];
      }
      CoordTransformDistance sphere(nd, center, dimensionsUsed);

      // Initialize the centroid to 0.0
      signal_t signal = 0;
      coord_t centroid[nd];
      for (size_t d=0; d<nd; d++)
        centroid[d] = 0.0;

      // Perform centroid
      ws->getBox()->centroidSphere(sphere, PeakRadius*PeakRadius, centroid, signal);

      // Normalize by signal
      if (signal != 0.0)
      {
        for (size_t d=0; d<nd; d++)
          centroid[d] /= signal;

        V3D vecCentroid(centroid[0], centroid[1], centroid[2]);

        // Save it back in the peak object, in the dimension specified.
        if (CoordinatesToUse == "Q (lab frame)")
        {
          p.setQLabFrame( vecCentroid, detectorDistance);
          p.findDetector();
        }
        else if (CoordinatesToUse == "Q (sample frame)")
        {
          p.setQSampleFrame( vecCentroid, detectorDistance);
          p.findDetector();
        }
        else if (CoordinatesToUse == "HKL")
        {
          p.setHKL( vecCentroid );
        }


        g_log.information() << "Peak " << i << " at " << pos << ": signal "
            << signal << ", centroid " << vecCentroid
            << " in " << CoordinatesToUse
            << std::endl;
      }
      else
      {
        g_log.information() << "Peak " << i << " at " << pos << " had no signal, and could not be centroided."
            << std::endl;
      }
    }

    // Save the output
    setProperty("OutputWorkspace", peakWS);

  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
   */
  void CentroidPeaksMD::exec()
  {
    inWS = getProperty("InputWorkspace");

    CALL_MDEVENT_FUNCTION3(this->integrate, inWS);
  }

} // namespace Mantid
} // namespace MDEvents

