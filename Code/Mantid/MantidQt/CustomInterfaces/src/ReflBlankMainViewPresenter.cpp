#include "MantidQtCustomInterfaces/ReflBlankMainViewPresenter.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/TableRow.h"
using namespace Mantid::API;
namespace
{
  ITableWorkspace_sptr createWorkspace()
  {
    ITableWorkspace_sptr ws = WorkspaceFactory::Instance().createTable();
    auto colRuns = ws->addColumn("str","Run(s)");
    auto colTheta = ws->addColumn("str","ThetaIn");
    auto colTrans = ws->addColumn("str","TransRun(s)");
    auto colQmin = ws->addColumn("str","Qmin");
    auto colQmax = ws->addColumn("str","Qmax");
    auto colDqq = ws->addColumn("str","dq/q");
    auto colScale = ws->addColumn("str","Scale");
    auto colStitch = ws->addColumn("int","StitchGroup");

    colRuns->setPlotType(0);
    colTheta->setPlotType(0);
    colTrans->setPlotType(0);
    colQmin->setPlotType(0);
    colQmax->setPlotType(0);
    colDqq->setPlotType(0);
    colScale->setPlotType(0);
    colStitch->setPlotType(0);

    TableRow row = ws->appendRow();
    row << "" << "" << "" << "" << "" << "" << "" << 0;
    return ws;
  }
}

namespace MantidQt
{
  namespace CustomInterfaces
  {


    //----------------------------------------------------------------------------------------------
    /** Constructor
    */
    ReflBlankMainViewPresenter::ReflBlankMainViewPresenter(ReflMainView* view): ReflMainViewPresenter(view)
    {
      m_model = createWorkspace();
      load();
    }

    //----------------------------------------------------------------------------------------------
    /** Destructor
    */
    ReflBlankMainViewPresenter::~ReflBlankMainViewPresenter()
    {
    }



  } // namespace CustomInterfaces
} // namespace Mantid