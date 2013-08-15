#ifndef MANTID_DATAHANDLING_LOADDETECTORSGROUPINGFILETEST_H_
#define MANTID_DATAHANDLING_LOADDETECTORSGROUPINGFILETEST_H_

#include "MantidKernel/Timer.h"
#include "MantidKernel/System.h"

#include "MantidDataHandling/LoadDetectorsGroupingFile.h"
#include "MantidDataObjects/GroupingWorkspace.h"
#include "MantidTestHelpers/ScopedFileHelper.h"

#include <cxxtest/TestSuite.h>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace Mantid;
using namespace Mantid::DataHandling;
using namespace Mantid::DataObjects;
using namespace Mantid::API;

using ScopedFileHelper::ScopedFile;

class LoadDetectorsGroupingFileTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static LoadDetectorsGroupingFileTest *createSuite() { return new LoadDetectorsGroupingFileTest(); }
  static void destroySuite( LoadDetectorsGroupingFileTest *suite ) { delete suite; }


  void test_Init(){

    LoadDetectorsGroupingFile load;
    load.initialize();
    TS_ASSERT(load.isInitialized());

  }

  void test_DetectorsGroupingXMLFile()
  {
    /*
     * 0-3695 : 1.0
     * 3696-(-1): 2.0
     */

    const std::string ws = "Vulcan_Group";

    LoadDetectorsGroupingFile load;
    load.initialize();

    TS_ASSERT(load.setProperty("InputFile", "vulcangroup.xml"));
    TS_ASSERT(load.setProperty("OutputWorkspace", ws));

    load.execute();
    TS_ASSERT(load.isExecuted());

    DataObjects::GroupingWorkspace_sptr gws = boost::dynamic_pointer_cast<DataObjects::GroupingWorkspace>(API::AnalysisDataService::Instance().retrieve(ws));

    TS_ASSERT_DELTA(gws->dataY(0)[0],    1.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(3695)[0], 1.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(3696)[0], 2.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(7000)[0], 2.0, 1.0E-5);
    //Check if filename is saved
    TS_ASSERT_EQUALS(load.getPropertyValue("InputFile"),gws->run().getProperty("Filename")->value());

    // Clean-up
    API::AnalysisDataService::Instance().remove(ws);
  }

  /*
   * Test on the group XML with zero
   */
  void test_AutoGroupIndex()
  {

    const std::string ws = "Vulcan_Group2";

    LoadDetectorsGroupingFile load;
    load.initialize();

    ScopedFile f = generateAutoGroupIDGroupXMLFile("testautoidgroup.xml");

    TS_ASSERT(load.setProperty("InputFile", f.getFileName()));
    TS_ASSERT(load.setProperty("OutputWorkspace", ws));

    load.execute();
    TS_ASSERT(load.isExecuted());

    DataObjects::GroupingWorkspace_sptr gws = boost::dynamic_pointer_cast<DataObjects::GroupingWorkspace>(API::AnalysisDataService::Instance().retrieve(ws));

    TS_ASSERT_DELTA(gws->dataY(0)[0],    1.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(3695)[0], 1.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(3696)[0], 2.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(7000)[0], 2.0, 1.0E-5);

    // Clean-up
    API::AnalysisDataService::Instance().remove(ws);
  }

  ScopedFile generateAutoGroupIDGroupXMLFile(std::string xmlfilename)
  {
    std::ostringstream os;

    os << "<?xml version=\"1.0\"?>" << std::endl;
    os << "<detector-grouping instrument=\"VULCAN\">" << std::endl;
    os << "  <group>" << std::endl;
    os << "    <detids>26250-27481,27500-28731,28750-29981</detids>" << std::endl;
    os << "  </group>" << std::endl;
    os << "  <group>" << std::endl;
    os << "    <component>bank26</component>" << std::endl;
    os << "    <component>bank27</component>" << std::endl;
    os << "    <component>bank28</component>" << std::endl;
 
    os << "  </group>" << std::endl;
    os << "</detector-grouping>" << std::endl;

    return ScopedFile(os.str(), xmlfilename);
  }

  /*
   * Test XML file using "ids"
   */
  void test_SpectrumIDs()
  {

    const std::string ws = "Vulcan_Group3";

    LoadDetectorsGroupingFile load;
    load.initialize();

    ScopedFile f = generateSpectrumIDXMLFile("testnoinstrumentgroup.xml");

    TS_ASSERT(load.setProperty("InputFile", f.getFileName()));
    TS_ASSERT(load.setProperty("OutputWorkspace", ws));

    load.execute();
    TS_ASSERT(load.isExecuted());

    DataObjects::GroupingWorkspace_sptr gws = boost::dynamic_pointer_cast<DataObjects::GroupingWorkspace>(API::AnalysisDataService::Instance().retrieve(ws));

    TS_ASSERT_DELTA(gws->dataY(0)[0], 1.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(1)[0], 1.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(5)[0], 2.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(16)[0], 2.0, 1.0E-5);

    // Clean-up
    API::AnalysisDataService::Instance().remove(ws);
  }

  ScopedFile generateSpectrumIDXMLFile(std::string xmlfilename)
  {
    std::ostringstream os;

    os << "<?xml version=\"1.0\"?>" << std::endl;
    os << "<detector-grouping>" << std::endl;
    os << "  <group>" << std::endl;
    os << "    <ids>30-36,12-16,100-111</ids>" << std::endl;
    os << "  </group>" << std::endl;
    os << "  <group>" << std::endl;
    os << "    <ids>38</ids>" << std::endl;
    os << "    <ids>291</ids>" << std::endl;
    os << "    <ids>22-25</ids>" << std::endl;
    os << "  </group>" << std::endl;
    os << "  <group name=\"bwd2\"><ids val=\"333,444,555\"/>334,557</group>" << std::endl;
    os << "</detector-grouping>" << std::endl;

    return ScopedFile(os.str(), xmlfilename);
  }

  /*
   * Test XML file using "ids" in old format
   */
  void test_OldFormat()
  {

    const std::string ws = "Random_Group_Old";

    LoadDetectorsGroupingFile load;
    load.initialize();

    ScopedFile f = generateOldSpectrumIDXMLFile("testoldformat.xml");

    TS_ASSERT(load.setProperty("InputFile", f.getFileName()));
    TS_ASSERT(load.setProperty("OutputWorkspace", ws));

    load.execute();
    TS_ASSERT(load.isExecuted());

    DataObjects::GroupingWorkspace_sptr gws =
        boost::dynamic_pointer_cast<DataObjects::GroupingWorkspace>(API::AnalysisDataService::Instance().retrieve(ws));

    TS_ASSERT_DELTA(gws->dataY(0)[0], 1.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(31)[0], 1.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(32)[0], 2.0, 1.0E-5);
    TS_ASSERT_DELTA(gws->dataY(39)[0], 2.0, 1.0E-5);

    // Clean-up
    API::AnalysisDataService::Instance().remove(ws);
  }

  ScopedFile generateOldSpectrumIDXMLFile(std::string xmlfilename)
  {
    std::ostringstream os;

    os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    os << "<detector-grouping>" << std::endl;
    os << "  <group name=\"fwd1\"> <ids val=\"1-32\"/> </group>" << std::endl;
    os << "  <group name=\"bwd1\"> <ids val=\"33,36,38,60-64\"/> </group>" << std::endl;
    os << "</detector-grouping>" << std::endl;

    return ScopedFile(os.str(), xmlfilename);
  }

  void test_DescriptionAndNameLoading()
  {

    const std::string ws = "Grouping";

    // Initialize an algorithm
    LoadDetectorsGroupingFile load;
    load.initialize();

    TS_ASSERT(load.setProperty("InputFile", "MUSRGrouping.xml"));
    TS_ASSERT(load.setProperty("OutputWorkspace", ws));

    // Run the algorithm
    load.execute();
    TS_ASSERT(load.isExecuted());

    auto gws = boost::dynamic_pointer_cast<DataObjects::GroupingWorkspace>(API::AnalysisDataService::Instance().retrieve(ws));

    // Check that description was loaded
    TS_ASSERT_EQUALS(gws->run().getProperty("Description")->value(), "musr longitudinal (64 detectors)");

    // Check that group names were loaded
    TS_ASSERT_EQUALS(gws->run().getProperty("GroupName_1")->value(), "fwd");
    TS_ASSERT_EQUALS(gws->run().getProperty("GroupName_2")->value(), "bwd");

    // Clean-up
    API::AnalysisDataService::Instance().remove(ws);
  }

};


#endif /* MANTID_DATAHANDLING_LOADDETECTORSGROUPINGFILETEST_H_ */
