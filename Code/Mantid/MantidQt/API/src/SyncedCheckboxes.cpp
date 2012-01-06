#include "MantidQtAPI/SyncedCheckboxes.h"
#include "MantidKernel/System.h"

namespace MantidQt
{
namespace API
{


  //----------------------------------------------------------------------------------------------
  /** Constructor that links a menu and a button
   *
   * @param menu :: menu to link
   * @param button :: button to link
   * @param checked :: state (checked or not) that they start in
   */
  SyncedCheckboxes::SyncedCheckboxes(QAction * menu, QAbstractButton * button, bool checked)
  : m_menu(menu), m_button(button)
  {
    m_menu->setCheckable(true);
    m_button->setCheckable(true);
    m_menu->setChecked(checked);
    m_button->setChecked(checked);
    // Now connect each signal to this object
    connect(m_menu, SIGNAL(toggled(bool)), this, SLOT(on_menu_toggled(bool)));
    connect(m_button, SIGNAL(toggled(bool)), this, SLOT(on_button_toggled(bool)));
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  SyncedCheckboxes::~SyncedCheckboxes()
  {
  }
  
  //----------------------------------------------------------------------------------------------
  /** Manually toggle the state of both checkboxes
   *
   * @param val :: True to check the boxes.
   */
  void SyncedCheckboxes::toggle(bool val)
  {
    // Set both GUI elements
    m_button->blockSignals(true);
    m_button->setChecked(val);
    m_button->blockSignals(false);
    m_menu->blockSignals(true);
    m_menu->setChecked(val);
    m_menu->blockSignals(false);
    // Re-transmit the signal
    emit toggled(val);
  }

  //----------------------------------------------------------------------------------------------
  /** Slot called when the menu is toggled */
  void SyncedCheckboxes::on_menu_toggled(bool val)
  {
    // Adjust the state of the other
    m_button->blockSignals(true);
    m_button->setChecked(val);
    m_button->blockSignals(false);
    // Re-transmit the signal
    emit toggled(val);
  }

  //----------------------------------------------------------------------------------------------
  /** Slot called when the button is toggled */
  void SyncedCheckboxes::on_button_toggled(bool val)
  {
    // Adjust the state of the other
    m_menu->blockSignals(true);
    m_menu->setChecked(val);
    m_menu->blockSignals(false);
    // Re-transmit the signal
    emit toggled(val);
  }


} // namespace Mantid
} // namespace API
