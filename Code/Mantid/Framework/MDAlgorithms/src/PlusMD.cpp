/*WIKI* 

This algorithm sums two [[MDHistoWorkspace]]s or merges two [[MDEventWorkspace]]s together.

=== MDHistoWorkspaces ===

* '''MDHistoWorkspace + MDHistoWorkspace'''
** The operation is performed element-by-element.
* '''MDHistoWorkspace + Scalar''' or '''Scalar + MDHistoWorkspace'''
** The scalar is subtracted from every element of the MDHistoWorkspace. The squares of errors are summed.

=== MDEventWorkspaces ===

This algorithm operates similary to calling Plus on two [[EventWorkspace]]s: it combines the events from the two workspaces together to form one large workspace.

==== Note for file-backed workspaces ====

The algorithm uses [[CloneMDWorkspace]] to create the output workspace, except when adding in place (e.g. <math> A = A + B </math> ).
See [[CloneMDWorkspace]] for details, but note that a file-backed [[MDEventWorkspace]] will have its file copied.

* If A is in memory and B is file-backed, the operation <math> C = A + B </math> will clone the B file-backed workspace and add A to it.
* However, the operation <math> A = A + B </math> will clone the A workspace and add B into memory (which might be too big!)

Also, be aware that events added to a MDEventWorkspace are currently added '''in memory''' and are not cached to file until [[SaveMD]]
or another algorithm requiring it is called. The workspace is marked as 'requiring file update'.

== Usage ==

 C = A + B
 C = A + 123.4
 A += B
 A += 123.4

See [[MDHistoWorkspace#Arithmetic_Operations|this page]] for examples on using arithmetic operations.

*WIKI*/

#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidKernel/System.h"
#include "MantidMDEvents/IMDBox.h"
#include "MantidMDEvents/MDBoxIterator.h"
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidMDAlgorithms/PlusMD.h"
#include "MantidKernel/ThreadScheduler.h"
#include "MantidKernel/ThreadPool.h"

using namespace Mantid::Kernel;
using namespace Mantid::MDEvents;
using namespace Mantid::API;

namespace Mantid
{
namespace MDAlgorithms
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(PlusMD)
  
  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  PlusMD::PlusMD()
  {  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  PlusMD::~PlusMD()
  {  }
  

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void PlusMD::initDocs()
  {
    this->setWikiSummary("Sum two [[MDHistoWorkspace]]s or merges two [[MDEventWorkspace]]s together by combining their events together in one workspace.");
    this->setOptionalMessage("Sum two MDHistoWorkspaces or merges two MDEventWorkspaces together by combining their events together in one workspace.");
  }


  //----------------------------------------------------------------------------------------------
  /** Perform the adding.
   *
   * Will do m_out_event += m_operand_event
   *
   * @param ws ::  MDEventWorkspace being added to
   */
  template<typename MDE, size_t nd>
  void PlusMD::doPlus(typename MDEventWorkspace<MDE, nd>::sptr ws)
  {
    typename MDEventWorkspace<MDE, nd>::sptr ws1 = ws;
    typename MDEventWorkspace<MDE, nd>::sptr ws2 = boost::dynamic_pointer_cast<MDEventWorkspace<MDE, nd> >(m_operand_event);
    if (!ws1 || !ws2)
      throw std::runtime_error("Incompatible workspace types passed to PlusMD.");

    IMDBox<MDE,nd> * box1 = ws1->getBox();
    IMDBox<MDE,nd> * box2 = ws2->getBox();

    Progress prog(this, 0.0, 0.4, box2->getBoxController()->getTotalNumMDBoxes());

    // How many events you started with
    size_t initial_numEvents = ws1->getNPoints();

    // Make a leaf-only iterator through all boxes with events in the RHS workspace
    MDBoxIterator<MDE,nd> it2(box2, 1000, true);
    do
    {
      MDBox<MDE,nd> * box = dynamic_cast<MDBox<MDE,nd> *>(it2.getBox());
      if (box)
      {
        // Copy the events from WS2 and add them into WS1
        const std::vector<MDE> & events = box->getConstEvents();
        // Add events, with bounds checking
        box1->addEvents(events);
        box->releaseEvents();
      }
      prog.report("Adding Events");
    } while (it2.next());

    this->progress(0.41, "Splitting Boxes");
    Progress * prog2 = new Progress(this, 0.4, 0.9, 100);
    ThreadScheduler * ts = new ThreadSchedulerFIFO();
    ThreadPool tp(ts, 0, prog2);
    ws1->splitAllIfNeeded(ts);
    prog2->resetNumSteps( ts->size(), 0.4, 0.6);
    tp.joinAll();

//    // Now we need to save all the data that was not saved before.
//    if (ws1->isFileBacked())
//    {
//      // Flush anything else in the to-write buffer
//      BoxController_sptr bc = ws1->getBoxController();
//
//      prog.resetNumSteps(bc->getTotalNumMDBoxes(), 0.6, 1.0);
//      MDBoxIterator<MDE,nd> it1(box1, 1000, true);
//      while (true)
//      {
//        MDBox<MDE,nd> * box = dynamic_cast<MDBox<MDE,nd> *>(it1.getBox());
//        if (box)
//        {
//          // Something was maybe added to this box
//          if (box->getEventVectorSize() > 0)
//          {
//            // By getting the events, this will merge the newly added and the cached events.
//            box->getEvents();
//            // The MRU to-write cache will optimize writes by reducing seek times
//            box->releaseEvents();
//          }
//        }
//        prog.report("Saving");
//        if (!it1.next()) break;
//      }
//      //bc->getDiskBuffer().flushCache();
//      // Flush the data writes to disk.
//      box1->flushData();
//    }

    this->progress(0.95, "Refreshing cache");
    ws1->refreshCache();

    // Set a marker that the file-back-end needs updating if the # of events changed.
    if (ws1->getNPoints() != initial_numEvents)
      ws1->setFileNeedsUpdating(true);

  }


  //----------------------------------------------------------------------------------------------
  /// Is the operation commutative?
  bool PlusMD::commutative() const
  { return true; }

  //----------------------------------------------------------------------------------------------
  /// Check the inputs and throw if the algorithm cannot be run
  void PlusMD::checkInputs()
  {
    if (m_lhs_event || m_rhs_event)
    {
      if (m_lhs_histo || m_rhs_histo)
        throw std::runtime_error("Cannot sum a MDHistoWorkspace and a MDEventWorkspace (only MDEventWorkspace + MDEventWorkspace is allowed).");
      if (m_lhs_scalar || m_rhs_scalar)
        throw std::runtime_error("Cannot sum a MDEventWorkspace and a scalar (only MDEventWorkspace + MDEventWorkspace is allowed).");
    }
  }

  //----------------------------------------------------------------------------------------------
  /// Run the algorithm with a MDHisotWorkspace as output and operand
  void PlusMD::execHistoHisto(Mantid::MDEvents::MDHistoWorkspace_sptr out, Mantid::MDEvents::MDHistoWorkspace_const_sptr operand)
  {
    out->add(*operand);
  }

  //----------------------------------------------------------------------------------------------
  /// Run the algorithm with a MDHisotWorkspace as output, scalar and operand
  void PlusMD::execHistoScalar(Mantid::MDEvents::MDHistoWorkspace_sptr out, Mantid::DataObjects::WorkspaceSingleValue_const_sptr scalar)
  {
    out->add(scalar->dataY(0)[0], scalar->dataE(0)[0]);
  }


  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm with an MDEventWorkspace as output
   */
  void PlusMD::execEvent()
  {
    // Now we add m_operand_event into m_out_event.
    CALL_MDEVENT_FUNCTION(this->doPlus, m_out_event);

    // Set to the output
    setProperty("OutputWorkspace", m_out_event);
  }



} // namespace Mantid
} // namespace MDAlgorithms
