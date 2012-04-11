#ifndef SCRIPTRUNNERWIDGET_H_
#define SCRIPTRUNNERWIDGET_H_

#include "Script.h"

#include <QWidget>
#include <QTextEdit>
#include <QPoint>
#include <QSplitter>

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class ScriptingEnv;
class ScriptEditor;
class ScriptOutputDisplay;

/**
 * Defines a widget that uses a ScriptEditor, a Script object and a
 * text display widget to give a single widget that can
 * edit, execute and display script code
 *
 */
class ScriptFileInterpreter : public QWidget
{
  Q_OBJECT

public:
  /// Construct the object
  ScriptFileInterpreter(QWidget *parent = NULL);
  /// Destroy the object
  ~ScriptFileInterpreter();
  /// Make sure we are in a safe state to delete the widget
  virtual void prepareToClose();
  /// Setup from a script environment
  virtual void setup(const ScriptingEnv & environ, const QString & identifier);

  /// Return the filename of the script in the editor
  virtual QString filename() const;
  /// Has the script text been modified
  virtual bool isScriptModified() const;
  /// Is the script running
  virtual bool isExecuting() const;

public slots:
  /// Save to the currently stored name
  virtual void saveToCurrentFile();
  /// Save to a different name
  virtual void saveAs();
  /// Save to the given filename
  virtual void saveScript(const QString & filename);
  /// Save the current output
  virtual void saveOutput(const QString & filename);
  /// Print the script
  virtual void printScript();
  /// Print the script
  virtual void printOutput();

  /// Undo
  virtual void undo();
  /// Redo
  virtual void redo();
  /// Copy from the editor
  virtual void copy();
  /// Cut from the editor
  virtual void cut();
  /// Paste into the editor
  virtual void paste();
  /// Find/replace in editor
  virtual void showFindReplaceDialog();

  /// Execute the whole script.
  virtual void executeAll(const Script::ExecutionMode mode);
  /// Execute the current selection
  virtual void executeSelection(const Script::ExecutionMode mode);

  /// Zoom in on script
  virtual void zoomInOnScript();
  /// Zoom out on script
  virtual void zoomOutOnScript();
  /// Toggles the progress reports on/off
  virtual void toggleProgressReporting(bool state);
  /// Toggles the code folding on/off
  virtual void toggleCodeFolding(bool state);

signals:
  /// Emits a signal when any text in the editor changes
  void textChanged();
  /// Emits a signal whenever the modification state of the editor changes
  void editorModificationChanged(bool);
  /// Emitted when the undo availability changes
  void editorUndoAvailable(bool);
  /// Emitted when the redo availability changes
  void editorRedoAvailable(bool);

private slots:
  // Popup a context menu
  void showContextMenu(const QPoint & clickPoint);

private:
  Q_DISABLE_COPY(ScriptFileInterpreter);
  void setupChildWidgets();

  void setupEditor(const ScriptingEnv & environ, const QString & identifier);
  void setupScriptRunner(const ScriptingEnv & environ, const QString & identifier);
  
  bool readFileIntoEditor(const QString & filename);
  void executeCode(const QString & code, const Script::ExecutionMode mode);

  QSplitter *m_splitter;
  ScriptEditor *m_editor;
  ScriptOutputDisplay *m_messages;
  QSharedPointer<Script> m_runner;
};

/**
 * A specialised NullScriptFileInterpreter class that
 * implements the Null object pattern to return a object of
 * this type that does nothing
 */
class NullScriptFileInterpreter : public ScriptFileInterpreter
{
  Q_OBJECT

public:
  /// Constructor
  NullScriptFileInterpreter() :
    ScriptFileInterpreter(NULL) {}

  /// Make sure we are in a safe state to delete the widget
  void prepareToClose() {};
  /// Setup from a script environment
  void setup(const ScriptingEnv &, const QString &) {};

  /// Return the filename of the script in the editor
  QString filename() const { return QString(); }
  /// Has the script text been modified
  bool isScriptModified() const { return false; }

private slots:
  /// Undo
  void undo() {}
  /// Redo
  void redo() {}
  /// Copy from the editor
  void copy() {}
  /// Cut from the editor
  void cut() {}
  /// Paste into the editor
  void paste() {}
  /// Find in editor
  void showFindReplaceDialog() {};

  /// Execute the whole script.
  virtual void executeAll(const Script::ExecutionMode) {}
  /// Execute the current selection
  virtual void executeSelection(const Script::ExecutionMode) {}

  /// Zoom in on script
  virtual void zoomInOnScript() {}
  /// Zoom out on script
  virtual void zoomOutOnScript() {}
  /// Toggles the progress reports on/off
  virtual void toggleProgressReporting(bool) {}
  /// Toggles the code folding on/off
  virtual void toggleCodeFolding(bool) {}


  /// Save to the currently stored name
  virtual void saveToCurrentFile() {}
  /// Save to a different name
  virtual void saveAs() {}
  /// Save to the given filename
  virtual void saveScript(const QString &) {};
  /// Save the current output
  virtual void saveOutput(const QString &) {};
  /// Print the script
  virtual void printScript() {}
  /// Print the script
  virtual void printOutput() {}

};

#endif /* SCRIPTRUNNERWIDGET_H_ */
