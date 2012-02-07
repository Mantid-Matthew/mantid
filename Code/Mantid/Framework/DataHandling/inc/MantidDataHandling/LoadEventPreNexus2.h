#ifndef LOADEVENTPRENEXUS_H_
#define LOADEVENTPRENEXUS_H_

#include <fstream>
#include <string>
#include <vector>
#include "MantidAPI/IDataFileChecker.h"
#include "MantidKernel/BinaryFile.h"
#include "MantidDataObjects/EventWorkspace.h"
#include "MantidDataObjects/Events.h"


namespace Mantid
{
  namespace DataHandling
  {
/** @class Mantid::DataHandling::LoadEventPreNexus2

    A data loading routine for SNS pre-nexus event files
    
    Copyright &copy; 2010-11 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>    
*/

/// This define is used to quickly turn parallel code on or off.
#undef LOADEVENTPRENEXUS_ALLOW_PARALLEL


/// Make the code clearer by having this an explicit type
typedef int PixelType;

/// Type for the DAS time of flight (data file)
typedef int DasTofType;

/// Structure that matches the form in the binary event list.
#pragma pack(push, 4) //Make sure the structure is 8 bytes.
struct DasEvent
{
  /// Time of flight.
  DasTofType tof;
  /// Pixel identifier as published by the DAS/DAE/DAQ.
  PixelType pid;
};
#pragma pack(pop)


/// Structure used as an intermediate for parallel processing of events
#pragma pack(push, 4) //Make sure the structure is 8 bytes.
struct IntermediateEvent
{
  /// Time of flight.
  DasTofType tof;
  /// Pixel identifier as published by the DAS/DAE/DAQ.
  PixelType pid;
  /// Frame index (pulse # of this event)
  size_t frame_index;
  /// Period of the event (not really used at this time)
  uint32_t period;
};
#pragma pack(pop)



/// Structure that matches the form in the new pulseid files.
#pragma pack(push, 4) //Make sure the structure is 16 bytes.
struct Pulse
{
  /// The number of nanoseconds since the seconds field. This is not necessarily less than one second.
  uint32_t nanoseconds;

  /// The number of seconds since January 1, 1990.
  uint32_t seconds;

  /// The index of the first event for this pulse.
  uint64_t event_index;

  /// The proton charge for the pulse.
  double pCurrent;
};
#pragma pack(pop)


class DLLExport LoadEventPreNexus2 : public API::IDataFileChecker
{
public:
  /// Constructor
  LoadEventPreNexus2();
  /// Virtual destructor
  virtual ~LoadEventPreNexus2();
  /// Algorithm's name
  virtual const std::string name() const { return "LoadEventPreNexus"; }
  /// Algorithm's version
  virtual int version() const { return (2); }
  /// Algorithm's category for identification
  virtual const std::string category() const { return "DataHandling\\PreNexus"; }
  /// Algorithm's aliases
  virtual const std::string alias() const { return "LoadEventPreNeXus2"; }

  /// Returns the name of the property to be considered as the Filename for Load
  virtual const char * filePropertyName() const;
  /// do a quick check that this file can be loaded 
  bool quickFileCheck(const std::string& filePath,size_t nread,const file_header& header);
  /// check the structure of the file and  return a value between 0 and 100 of how much this file can be loaded
  int fileCheck(const std::string& filePath);
  
private:
  /// Sets documentation strings for this algorithm
  virtual void initDocs();

  /// Initialisation code
  void init();
  ///Execution code
  void exec();

  Mantid::API::Progress * prog;

  DataObjects::EventWorkspace_sptr localWorkspace; //< Output EventWorkspace
  std::vector<int64_t> spectra_list; ///<the list of Spectra

  /// The times for each pulse.
  std::vector<Kernel::DateAndTime> pulsetimes;
  /// The index of the first event in each pulse.
  std::vector<uint64_t> event_indices;
  /// The proton charge on a pulse by pulse basis.
  std::vector<double> proton_charge;
  /// The total proton charge for the run.
  double proton_charge_tot;
  /// The value of the vector is the workspace index. The index into it is the pixel ID from DAS
  std::vector<std::size_t> pixel_to_wkspindex;
  /// Map between the DAS pixel IDs and our pixel IDs, used while loading.
  std::vector<PixelType> pixelmap;

  /// The maximum detector ID possible
  Mantid::detid_t detid_max;

  /// Handles loading from the event file
  Mantid::Kernel::BinaryFile<DasEvent> * eventfile;
  std::size_t num_events; ///< The number of events in the file
  std::size_t num_pulses; ///<the number of pulses
  uint32_t numpixel; ///<the number of pixels

  std::size_t num_good_events; ///< The number of good events loaded
  std::size_t num_error_events; ///< The number of error events encountered
  std::size_t num_bad_events; ///<The number of bad events. Part of error events
  std::size_t num_wrongdetid_events; ///<The number of events with wrong detector IDs. Part of error events.
  std::set<PixelType> wrongdetids; ///<set of all wrong detector IDs
  std::map<PixelType, size_t> wrongdetidmap;
  std::vector<std::vector<Kernel::DateAndTime> > wrongdetid_pulsetimes;
  std::vector<std::vector<double> > wrongdetid_tofs;

  /// the number of events that were ignored (not loaded) because, e.g. of only loading some spectra.
  std::size_t num_ignored_events;
  std::size_t first_event;   ///< The first event to load (count from zero)
  std::size_t max_events;    ///< Number of events to load


  /// Set to true if a valid Mapping file was provided.
  bool using_mapping_file;

  /// For loading only some spectra
  bool loadOnlySomeSpectra;
  /// Handle to the loaded spectra map
  std::map<int64_t, bool> spectraLoadMap;

  /// Longest TOF limit
  double longest_tof;
  /// Shortest TOF limit
  double shortest_tof;

  /// Flag to allow for parallel loading
  bool parallelProcessing;

  /// sample environment event
  std::vector<detid_t> mSEids;
  std::map<size_t, detid_t> mSEmap;
  std::vector<std::vector<int64_t> > mSEpulseids;
  std::vector<std::vector<double> > mSEtofs;

  void loadPixelMap(const std::string &filename);

  void openEventFile(const std::string &filename);

  void readPulseidFile(const std::string &filename, const bool throwError);

  void runLoadInstrument(const std::string &eventfilename, API::MatrixWorkspace_sptr localWorkspace);

  inline void fixPixelId(PixelType &pixel, uint32_t &period) const;

  void procEvents(DataObjects::EventWorkspace_sptr & workspace);

  void procEventsLinear(DataObjects::EventWorkspace_sptr & workspace, std::vector<DataObjects::TofEvent> ** arrayOfVectors, DasEvent * event_buffer, size_t current_event_buffer_size, size_t fileOffset);

  void setProtonCharge(DataObjects::EventWorkspace_sptr & workspace);

  void addToWorkspaceLog(std::string logtitle, size_t mindex);

  void processImbedLogs();

  void debugOutput(bool doit, size_t mindex);

};

  }
}
#endif /*LOADEVENTPRENEXUS_H_*/
