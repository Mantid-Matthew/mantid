//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidQtCustomInterfaces/MuonAnalysisResultTableTab.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/TimeSeriesProperty.h"

#include "MantidQtAPI/UserSubWindow.h"

#include <boost/shared_ptr.hpp>

#include <QLineEdit>
#include <QFileDialog>
#include <QHash>
#include <QMessageBox>
#include <QDesktopServices>

#include <algorithm>

//-----------------------------------------------------------------------------


namespace MantidQt
{
namespace CustomInterfaces
{
namespace Muon
{
  using namespace MantidQt::API;
  using namespace Mantid::Kernel;
  using namespace MantidQt::MantidWidgets;


/**
* Init the layout.
*/
void MuonAnalysisResultTableTab::initLayout()
{
  // Connect the help button to the wiki page.
  connect(m_uiForm.muonAnalysisHelpResults, SIGNAL(clicked()), this, SLOT(helpResultsClicked()));
  
  // add check boxes for the include columns on log table and fitting table.
  for (int i = 0; i < m_uiForm.valueTable->rowCount(); i++)
  {
    m_uiForm.valueTable->setCellWidget(i,1, new QCheckBox);
  }
  for (int i = 0; i < m_uiForm.fittingResultsTable->rowCount(); i++)
  {
    m_uiForm.fittingResultsTable->setCellWidget(i,1, new QCheckBox);
  }

  //Set the default name
  m_uiForm.tableName->setText("ResultsTable");

  // Connect the select/deselect all buttons.
  connect(m_uiForm.selectAllLogValues, SIGNAL(toggled(bool)), this, SLOT(selectAllLogs(bool)));
  connect(m_uiForm.selectAllFittingResults, SIGNAL(toggled(bool)), this, SLOT(selectAllFittings(bool)));

  // Connect the create table button
  connect(m_uiForm.createTableBtn, SIGNAL(clicked()), this, SLOT(createTable()));
}


/**
* Muon Analysis Results Table Help (slot)
*/
void MuonAnalysisResultTableTab::helpResultsClicked()
{
  QDesktopServices::openUrl(QUrl(QString("http://www.mantidproject.org/") +
            "MuonAnalysisResultsTable"));
}


/**
* Select/Deselect all log values to be included in the table
*/
void MuonAnalysisResultTableTab::selectAllLogs(bool state)
{
  if (state)
  {
    for (int i = 0; i < m_uiForm.valueTable->rowCount(); i++)
    {
      QTableWidgetItem* temp = static_cast<QTableWidgetItem*>(m_uiForm.valueTable->item(i,0));
      // If there is an item there then check the box
      if (temp != NULL)
      {
        QCheckBox* includeCell = static_cast<QCheckBox*>(m_uiForm.valueTable->cellWidget(i,1));
        includeCell->setChecked(true);
      }
    }
  }
  else
  {
    for (int i = 0; i < m_uiForm.valueTable->rowCount(); i++)
    {
      QCheckBox* includeCell = static_cast<QCheckBox*>(m_uiForm.valueTable->cellWidget(i,1));
      includeCell->setChecked(false);
    }
  }
}


/**
* Select/Deselect all fitting results to be included in the table
*/
void MuonAnalysisResultTableTab::selectAllFittings(bool state)
{
  if (state)
  {
    for (int i = 0; i < m_uiForm.fittingResultsTable->rowCount(); i++)
    {
      QTableWidgetItem* temp = static_cast<QTableWidgetItem*>(m_uiForm.fittingResultsTable->item(i,0));
      // If there is an item there then check the box
      if (temp != NULL)
      {
        QCheckBox* includeCell = static_cast<QCheckBox*>(m_uiForm.fittingResultsTable->cellWidget(i,1));
        includeCell->setChecked(true);
      }
    }
  }
  else
  {
    for (int i = 0; i < m_uiForm.fittingResultsTable->rowCount(); i++)
    {
      QCheckBox* includeCell = static_cast<QCheckBox*>(m_uiForm.fittingResultsTable->cellWidget(i,1));
      includeCell->setChecked(false);
    }
  }
}


/**
* Populates the tables with all the correct log values and fitting results. It takes the
* given workspace list and checks to see if a fit has been done to the data set and if so
* adds it to a new workspace list.
*
* @param wsList :: A list containing all the data set workspaces that have been loaded
*                   by muon analysis.
*/
void MuonAnalysisResultTableTab::populateTables(const QStringList& wsList)
{
  // Clear the previous table values
  m_tableValues.clear();
  QVector<QString> fittedWsList;
  // Get all the workspaces from the fitPropertyBrowser and find out whether they have had fitting done to them.
  for (int i(0); i<wsList.size(); ++i)
  {
    if((Mantid::API::AnalysisDataService::Instance().doesExist(wsList[i].toStdString() + "_Parameters"))&&(Mantid::API::AnalysisDataService::Instance().doesExist(wsList[i].toStdString()))) 
      fittedWsList.append(wsList[i]);
  }

  if(fittedWsList.size() > 0)
  { 
    // Make sure all params match.
    QVector<QString> sameFittedWsList(fittedWsList);

    // Populate the individual log values and fittings into their respective tables.
    populateFittings(sameFittedWsList);
    populateLogsAndValues(sameFittedWsList);

    QTableWidgetItem* temp = static_cast<QTableWidgetItem*>(m_uiForm.valueTable->item(0,0));
    // If there is no item in the first row then there must be no log files found between the two data sets.
    if (temp == NULL)
    {
      QMessageBox::information(this, "Mantid - Muon Analysis", "There were no common log files found.");
    }
    else
    {
      // Make sure all fittings are selected by default.
      selectAllFittings(true);
    }
  }
  else
  {
    QMessageBox::information(this, "Mantid - Muon Analysis", "A fitting must be made on the Data Analysis tab before producing a Results Table.");
  }
}


/**
* Populates the items (log values) into their table.
*
* @param fittedWsList :: a workspace list containing ONLY the workspaces that have parameter
*                   tables associated with it.
*/
void MuonAnalysisResultTableTab::populateLogsAndValues(const QVector<QString>& fittedWsList)
{
  // Clear the logs if not empty and then repopulate.
  QVector<QString> logsToDisplay;
  
  for (int i=0; i<fittedWsList.size(); i++)
  { 
    QMap<QString, double> allLogs;

    // Get log information
    Mantid::API::ExperimentInfo_sptr ws = boost::dynamic_pointer_cast<Mantid::API::ExperimentInfo>(Mantid::API::AnalysisDataService::Instance().retrieve(fittedWsList[i].toStdString()));
    if (!ws)
    {
      throw std::runtime_error("Wrong type of Workspace");
    }

    const std::vector< Mantid::Kernel::Property * > & logData = ws->run().getLogData();
    std::vector< Mantid::Kernel::Property * >::const_iterator pEnd = logData.end();
    
    Mantid::Kernel::DateAndTime start = ws->run().startTime();
    Mantid::Kernel::DateAndTime end = ws->run().endTime();

    for( std::vector< Mantid::Kernel::Property * >::const_iterator pItr = logData.begin();
          pItr != pEnd; ++pItr )
    {  

      QString logFile(QFileInfo((**pItr).name().c_str()).fileName());
      // Just get the num.series log values
      Mantid::Kernel::TimeSeriesProperty<double> *tspd = dynamic_cast<Mantid::Kernel::TimeSeriesProperty<double> *>(*pItr);

      if( tspd )//If it is then it must be num.series
      {
        bool logFound(false);
        double value(0.0);
        double count(0.0);

        Mantid::Kernel::DateAndTime logTime;

        //iterate through all logs entries of a specific log
        for (int k(0); k<tspd->size(); k++)
        {
          // Get the log time for the specific entry
          logTime = tspd->nthTime(k);

          // If the entry was made during the run times
          if ((logTime >= start) && (logTime <= end))
          {
            // add it to a total and increment the count (will be used to make average entry value during a run)
            value += tspd->nthValue(k);
            count++;
            logFound = true;
          }
        }

        if (logFound == true)
        {
          //Find average
          allLogs[logFile] = value/count;
          if (i == 0)
            logsToDisplay.push_back(logFile);
          else
          {
            bool reg(true);
            for(int j=0; j<logsToDisplay.size(); ++j)
            {
              //if log file already registered then don't register it again.
              if (logsToDisplay[j] == logFile)
              {
                reg = false;
                break;
              }
            }
            if (reg==true)
            logsToDisplay.push_back(logFile);
          }
        }
      }
    }

    // Add all data collected from one workspace to another map. Will be used when creating table.
    m_tableValues[fittedWsList[i]] = allLogs;

  } // End loop over all workspace's log information and param information

  // Remove the logs that don't appear in all workspaces
  QVector<int> toRemove;
  for(int i=0; i<logsToDisplay.size(); ++i)
  {
    QMap<QString,QMap<QString, double> >::Iterator itr; 
    for (itr = m_tableValues.begin(); itr != m_tableValues.end(); itr++)
    { 
      QMap<QString, double> logsAndValues = itr.value();

      if (!(logsAndValues.contains(logsToDisplay[i])))
      {      
        toRemove.push_back(i);
        break;
      }
    }      
  }

  for(int i=0; i<toRemove.size(); ++i)
  {
    logsToDisplay.remove(toRemove[i]-i);
  }

  // If there isn't enough rows in the table to populate all logs then display error message
  if(logsToDisplay.size() > m_uiForm.valueTable->rowCount())
  {
    QMessageBox::information(this, "Mantid - Muon Analysis", "There is not enough room in the table to populate all fitting parameter results");
  }
  else
  {
    // Populate table with all log values available without repeating any.
    for (int row = 0; row < m_uiForm.valueTable->rowCount(); row++)
    {
      if (row < logsToDisplay.size())
        m_uiForm.valueTable->setItem(row,0, new QTableWidgetItem(logsToDisplay[row]));
      else
        m_uiForm.valueTable->setItem(row,0, NULL);
    }
  }

  // Save the number of logs displayed so don't have to search through all cells.
  m_numLogsdisplayed = logsToDisplay.size();
}


/**
* Populates the items (fitted workspaces) into their table.
*
* @param fittedWsList :: a workspace list containing ONLY the workspaces that have parameter
*                        tables associated with it.
*/
void MuonAnalysisResultTableTab::populateFittings(const QVector<QString>& fittedWsList)
{
  if(fittedWsList.size() > m_uiForm.fittingResultsTable->rowCount())
  {
    QMessageBox::information(this, "Mantid - Muon Analysis", "There is not enough room in the table to populate all fitting parameter results");
  }
  else
  {
    // Get colors, 0=Black, 1=Red, 2=Green, 3=Blue, 4=Orange, 5=Purple. (If there are more than this then use black as default.)
    QMap<int, int> colors = getWorkspaceColors(fittedWsList);
    for (int row = 0; row < m_uiForm.fittingResultsTable->rowCount(); row++)
    {
      // Fill values and delete previous old ones.
      if (row < fittedWsList.size())
      {
        QTableWidgetItem *item = new QTableWidgetItem(fittedWsList[row]);
        int color(colors.find(row).data());
        switch (color)
        {
          case(1):
            item->setTextColor("red");
            break;
          case(2):
            item->setTextColor("green");
            break;
          case(3):
            item->setTextColor("blue");
            break;
          case(4):
            item->setTextColor("orange");
            break;
          case(5):
            item->setTextColor("purple");
            break;
          default:
            item->setTextColor("black");
        }
        m_uiForm.fittingResultsTable->setItem(row, 0, item);
      }
      else
        m_uiForm.fittingResultsTable->setItem(row,0, NULL);
    }
  }
}


/**
* Get the colors corresponding to their position in the workspace list.
*
* @params wsList :: List of all workspaces with fitted parameters.
* @return colors :: List of colors (as numbers) with the key being position in wsList.
*/
QMap<int, int> MuonAnalysisResultTableTab::getWorkspaceColors(const QVector<QString>& wsList)
{
  QMap<int,int> colors; //position, color
  int posCount(0);
  int colorCount(0);

  while (wsList.size() != posCount)
  {
    // If a color has already been chosen for the current workspace then skip
    if (!colors.contains(posCount))
    {
      std::vector<std::string> firstParams;
      // Find the first parameter table and use this as a comparison for all the other tables.
      Mantid::API::ITableWorkspace_sptr paramWs = boost::dynamic_pointer_cast<Mantid::API::ITableWorkspace>(Mantid::API::AnalysisDataService::Instance().retrieve(wsList[posCount].toStdString() + "_Parameters") );

      Mantid::API::TableRow paramRow = paramWs->getFirstRow();
      do
      {  
        std::string key;
        paramRow >> key;
        firstParams.push_back(key);
      }
      while(paramRow.next());

      colors.insert(posCount, colorCount);

      // Compare to all the other parameters. +1 don't compare with self.
      for (int i=(posCount + 1); i<wsList.size(); ++i)
      {
        if (!colors.contains(i))
        {
          std::vector<std::string> nextParams;
          Mantid::API::ITableWorkspace_sptr paramWs = boost::dynamic_pointer_cast<Mantid::API::ITableWorkspace>(Mantid::API::AnalysisDataService::Instance().retrieve(wsList[i].toStdString() + "_Parameters") );

          Mantid::API::TableRow paramRow = paramWs->getFirstRow();
          do
          {  
            std::string key;
            paramRow >> key;
            nextParams.push_back(key);
          }
          while(paramRow.next());

          if (firstParams == nextParams)
          {
            colors.insert(i, colorCount);
          }
        }
      }
      colorCount++;
    }
    posCount++;
  }
  return colors;
}


/**
* Creates the table using the information selected by the user in the tables
*/
void MuonAnalysisResultTableTab::createTable()
{  
  if (m_tableValues.size() == 0)
  {
    QMessageBox::information(this, "Mantid - Muon Analysis", "No workspace found with suitable fitting.");
    return;
  }

  // Get the user selection
  QVector<QString> wsSelected = getSelectedWs();
  QVector<QString> logsSelected = getSelectedLogs();

  if ((wsSelected.size() == 0) || logsSelected.size() == 0)
  {
    QMessageBox::information(this, "Mantid - Muon Analysis", "Please select options from both tables.");
    return;
  }

  // Check workspaces have same parameters
  if (haveSameParameters(wsSelected))
  {
    // Create the results table
    Mantid::API::ITableWorkspace_sptr table = Mantid::API::WorkspaceFactory::Instance().createTable("TableWorkspace");
    table->addColumn("str","Run Number");
    table->getColumn(table->columnCount()-1)->setPlotType(6);
    for(int i=0; i<logsSelected.size(); ++i)
    {
      table->addColumn("double", logsSelected[i].toStdString());
      table->getColumn(table->columnCount()-1)->setPlotType(1);
    }

    // Get param information
    QMap<QString, QMap<QString, double> > wsParamsList;
    QVector<QString> paramsToDisplay;
    for(int i=0; i<wsSelected.size(); ++i)
    {
      QMap<QString, double> paramsList;
      Mantid::API::ITableWorkspace_sptr paramWs = boost::dynamic_pointer_cast<Mantid::API::ITableWorkspace>(Mantid::API::AnalysisDataService::Instance().retrieve(wsSelected[i].toStdString() + "_Parameters") );

      Mantid::API::TableRow paramRow = paramWs->getFirstRow();
    
      // Loop over all rows and get values and errors.
      do
      {  
        std::string key;
        double value;
        double error;
        paramRow >> key >> value >> error;
        if (i == 0)
        {
          table->addColumn("double", key);
          table->getColumn(table->columnCount()-1)->setPlotType(2);
          table->addColumn("double", key + "Error");
          table->getColumn(table->columnCount()-1)->setPlotType(5);
          paramsToDisplay.append(QString::fromStdString(key));
          paramsToDisplay.append(QString::fromStdString(key) + "Error");
        }
        paramsList[QString::fromStdString(key)] = value;
        paramsList[QString::fromStdString(key) + "Error"] = error;
      }
      while(paramRow.next());

      wsParamsList[wsSelected[i]] = paramsList;
    }

    // Add data to table
    QMap<QString,QMap<QString, double> >::Iterator itr; 
    for (itr = m_tableValues.begin(); itr != m_tableValues.end(); itr++)
    { 
      for(int i=0; i<wsSelected.size(); ++i)
      {
        if (wsSelected[i] == itr.key())
        {
          //Add new row and add run number
          Mantid::API::TableRow row = table->appendRow();
          QString run(itr.key().left(itr.key().find(';')));
        
          for (int j=0; j<run.size(); ++j)
          {
            if(run[j].isNumber())
            {
              run = run.right(run.size() - j);
              break;
            }
          }
          row << run.toStdString();

          // Add log values
          QMap<QString, double> logsAndValues = itr.value();
          for(int j=0; j<logsSelected.size(); ++j)
          {
            row << logsAndValues.find(logsSelected[j]).value();
          }

          // Add param values (presume params the same for all workspaces)
          QMap<QString, double> paramsList = wsParamsList.find(itr.key()).value();
          for(int j=0; j<paramsToDisplay.size(); ++j)
          {
            row << paramsList.find(paramsToDisplay[j]).value();
          }
        }
      }
    }  
    // Save the table to the ADS
    Mantid::API::AnalysisDataService::Instance().addOrReplace(getFileName(),table);
  }
  else
  {
    QMessageBox::information(this, "Mantid - Muon Analysis", "Please pick workspaces with the same fitted parameters");
  }
}


/**
* See if the workspaces selected have the same parameters.
*
* @params wsList :: A list of workspaces with fitted parameters.
* @return bool :: Whether or not the wsList given share the same fitting parameters.
*/
bool MuonAnalysisResultTableTab::haveSameParameters(const QVector<QString>& wsList)
{
  std::vector<std::string> firstParams;

  // Find the first parameter table and use this as a comparison for all the other tables.
  Mantid::API::ITableWorkspace_sptr paramWs = boost::dynamic_pointer_cast<Mantid::API::ITableWorkspace>(Mantid::API::AnalysisDataService::Instance().retrieve(wsList[0].toStdString() + "_Parameters") );

  Mantid::API::TableRow paramRow = paramWs->getFirstRow();
  do
  {  
    std::string key;
    paramRow >> key;
    firstParams.push_back(key);
  }
  while(paramRow.next());

  // Compare to all the other parameters.
  for (int i=1; i<wsList.size(); ++i)
  {
    std::vector<std::string> nextParams;
    Mantid::API::ITableWorkspace_sptr paramWs = boost::dynamic_pointer_cast<Mantid::API::ITableWorkspace>(Mantid::API::AnalysisDataService::Instance().retrieve(wsList[i].toStdString() + "_Parameters") );

    Mantid::API::TableRow paramRow = paramWs->getFirstRow();
    do
    {  
      std::string key;
      paramRow >> key;
      nextParams.push_back(key);
    }
    while(paramRow.next());

    if (!(firstParams == nextParams))
      return false;
  }
  return true;
}


/**
* Get the user selected workspaces with _parameters files associated with it
*
* @return wsSelected :: A vector of QString's containing the workspace that are selected.
*/
QVector<QString> MuonAnalysisResultTableTab::getSelectedWs()
{
  QVector<QString> wsSelected;
  for (int i = 0; i < m_tableValues.size(); i++)
  {
    QCheckBox* includeCell = static_cast<QCheckBox*>(m_uiForm.fittingResultsTable->cellWidget(i,1));
    if (includeCell->isChecked())
    {
      QTableWidgetItem* wsName = static_cast<QTableWidgetItem*>(m_uiForm.fittingResultsTable->item(i,0));
      wsSelected.push_back(wsName->text());
    }
  }
  return wsSelected;
}


/**
* Get the user selected log file
*
* @return logsSelected :: A vector of QString's containing the logs that are selected.
*/
QVector<QString> MuonAnalysisResultTableTab::getSelectedLogs()
{
  QVector<QString> logsSelected;
  for (int i = 0; i < m_numLogsdisplayed; i++)
  {
    QCheckBox* includeCell = static_cast<QCheckBox*>(m_uiForm.valueTable->cellWidget(i,1));
    if (includeCell->isChecked())
    {
      QTableWidgetItem* logParam = static_cast<QTableWidgetItem*>(m_uiForm.valueTable->item(i,0));
      logsSelected.push_back(logParam->text());
    }
  }
  return logsSelected;
}


/**
* Checks that the file name isn't been used, displays the appropriate message and
* then returns the name in which to save.
*
* @return name :: The name the results table should be created with.
*/
std::string MuonAnalysisResultTableTab::getFileName()
{
  std::string fileName(m_uiForm.tableName->text());
  
  if (Mantid::API::AnalysisDataService::Instance().doesExist(fileName))
  {
    int choice = QMessageBox::question(this, tr("MantidPlot - Overwrite Warning"), QString::fromStdString(fileName) +
          tr(" already exists. Do you want to replace it?"),
          QMessageBox::Yes|QMessageBox::Default, QMessageBox::No|QMessageBox::Escape);
    if (choice == QMessageBox::No)
    {
      int versionNum(2); 
      fileName += " #";
      while(Mantid::API::AnalysisDataService::Instance().doesExist(fileName + boost::lexical_cast<std::string>(versionNum)))
      {
        versionNum += 1;
      }
      return (fileName + boost::lexical_cast<std::string>(versionNum));
    }
  }
  return fileName;
}


}
}
}
