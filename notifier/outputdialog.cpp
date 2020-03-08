/*
* This file is part of OctoXBPS, an open-source GUI for XBPS.
* Copyright (C) 2013 Alexandre Albuquerque Arnt
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#include "outputdialog.h"
#include "../src/xbpsexec.h"
#include "../src/searchbar.h"
#include "../src/uihelper.h"
#include "../src/strconstants.h"
#include "../src/termwidget.h"

#include <QTextBrowser>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QCloseEvent>
#include <QMessageBox>

/*
 * Class that displays pacman output for system upgrade
 */

/*
 * The obligatory constructor...
 */
OutputDialog::OutputDialog(QWidget *parent): QDialog(parent)
{
  m_upgradeRunning = false;
  m_debugInfo = false;
  m_upgradeXBPS = false;
}

/*
 * Sets if xbpsExec will be called in debugMode or not
 */
void OutputDialog::setDebugMode(bool newValue)
{
  m_debugInfo = newValue;
}

/*
 * Sets if xbpsExec will upgrade XBPS package first
 */
void OutputDialog::setUpgradeXBPS(bool newValue)
{
  m_upgradeXBPS = newValue;
}

/*
 * Controls if this dialog was called for updates in GUI or in the embedded qtermwidget
 */
void OutputDialog::setViewAsTextBrowser(bool value)
{
  m_viewAsTextBrowser = value;
}

/*
 * Let's init for graphical updates...
 */
void OutputDialog::initAsTextBrowser()
{
  this->resize(650, 500);

  setWindowTitle(QCoreApplication::translate("MainWindow", "System upgrade"));
  setWindowIcon(IconHelper::getIconSystemUpgrade());

  m_mainLayout = new QVBoxLayout(this);
  m_textBrowser = new QTextBrowser(this);
  m_progressBar = new QProgressBar(this);
  m_textBrowser->setGeometry(QRect(0, 0, 650, 500));
  m_textBrowser->setFrameShape(QFrame::NoFrame);

  m_mainLayout->addWidget(m_textBrowser);

  m_searchBar = new SearchBar(this);
  connect(m_searchBar, SIGNAL(textChanged(QString)), this, SLOT(onSearchBarTextChanged(QString)));
  connect(m_searchBar, SIGNAL(closed()), this, SLOT(onSearchBarClosed()));
  connect(m_searchBar, SIGNAL(findNext()), this, SLOT(onSearchBarFindNext()));
  connect(m_searchBar, SIGNAL(findPrevious()), this, SLOT(onSearchBarFindPrevious()));
  m_mainLayout->addWidget(m_progressBar);
  m_mainLayout->addWidget(m_searchBar);
  m_mainLayout->setSpacing(0);
  m_mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
  m_mainLayout->setContentsMargins(2, 2, 2, 2);
  m_progressBar->setMinimum(0);
  m_progressBar->setMaximum(100);
  m_progressBar->setValue(0);
  m_progressBar->close();
  m_searchBar->show();
}

/*
 * Let's init for manual updates in a qtermwidget...
 */
void OutputDialog::initAsTermWidget()
{
  this->resize(650, 500);
  setWindowTitle(QCoreApplication::translate("MainWindow", "System upgrade"));
  setWindowIcon(IconHelper::getIconSystemUpgrade());

  m_mainLayout = new QVBoxLayout(this);
  m_console = new TermWidget(this);
  m_mainLayout->addWidget(m_console);

  m_mainLayout->setSpacing(0);
  m_mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
  m_mainLayout->setContentsMargins(2, 2, 2, 2);
  m_console->setFocus();
  //m_console->toggleShowSearchBar();
  m_console->installEventFilter(this);
}

/*
 * When there is a command to exec in the terminal
 */
void OutputDialog::onExecCommandInTabTerminal(QString command)
{
  disconnect(m_console, SIGNAL(onPressAnyKeyToContinue()), this, SLOT(onPressAnyKeyToContinue()));
  disconnect(m_console, SIGNAL(onCancelControlKey()), this, SLOT(onCancelControlKey()));
  disconnect(m_console, SIGNAL(onKeyQuit()), this, SLOT(reject()));
  connect(m_console, SIGNAL(onPressAnyKeyToContinue()), this, SLOT(onPressAnyKeyToContinue()));
  connect(m_console, SIGNAL(onCancelControlKey()), this, SLOT(onCancelControlKey()));
  connect(m_console, SIGNAL(onKeyQuit()), this, SLOT(reject()));
  m_console->execute(command);
  m_console->setFocus();
}

/*
 * Whenever the terminal transaction has finished, we can update the UI
 */
void OutputDialog::onPressAnyKeyToContinue()
{
  m_console->setFocus();

  if (!m_upgradeRunning) return;
  if (m_xbpsExec != nullptr)
    delete m_xbpsExec;

  m_upgradeRunning = false;
}

/*
 * Whenever a user strikes Ctrl+C, Ctrl+D or Ctrl+Z in the terminal
 */
void OutputDialog::onCancelControlKey()
{
  if (m_upgradeRunning)
  {
    if (m_xbpsExec != nullptr)
      delete m_xbpsExec;

    m_xbpsExec = nullptr;
    m_upgradeRunning = false;
  }
}

/*
 * Calls xbpsExec to begin system upgrade
 */
void OutputDialog::doSystemUpgrade()
{
  m_xbpsExec = new XBPSExec(this);

  if (m_debugInfo)
    m_xbpsExec->setDebugMode(true);

  QObject::connect(m_xbpsExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( xbpsProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_xbpsExec, SIGNAL(percentage(int)), this, SLOT(onPencertange(int)));
  QObject::connect(m_xbpsExec, SIGNAL(textToPrintExt(QString)), this, SLOT(onWriteOutput(QString)));

  m_upgradeRunning = true;
  m_xbpsExec->doSystemUpgrade(m_upgradeXBPS);
}

/*
 * Calls xbpsExec to begin system upgrade inside a terminal
 */
void OutputDialog::doSystemUpgradeInTerminal()
{
  m_xbpsExec = new XBPSExec(this);

  QObject::connect(m_xbpsExec, SIGNAL(commandToExecInQTermWidget(QString)), this,
                   SLOT(onExecCommandInTabTerminal(QString)));

  m_upgradeRunning = true;
  m_xbpsExec->doSystemUpgradeInTerminal(m_upgradeXBPS);
}

/*
 * Centers the dialog in the screen
 */
void OutputDialog::show()
{
  //If we are asking for a graphical system upgrade...
  if (m_viewAsTextBrowser)
    initAsTextBrowser();
  else
    initAsTermWidget();

  //Let's restore the dialog size saved...
  restoreGeometry(SettingsManager::getOutputDialogWindowSize());

  QDialog::show();
}

/*
 * Whenever the user presses the ESC key
 */
void OutputDialog::reject()
{
  if (!m_upgradeRunning)
  {
    //Let's save the dialog size value before closing it.
    QByteArray windowSize=saveGeometry();
    SettingsManager::setOutputDialogWindowSize(windowSize);
    QDialog::reject();
  }
}

/*
 * Slot called whenever xbpsExec emits a new percentage change
 */
void OutputDialog::onPencertange(int percentage)
{
  if (percentage > 0 && !m_progressBar->isVisible()) m_progressBar->show();
  m_progressBar->setValue(percentage);
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void OutputDialog::positionTextEditCursorAtEnd()
{
  QTextCursor tc = m_textBrowser->textCursor();
  tc.clearSelection();
  tc.movePosition(QTextCursor::End);
  m_textBrowser->setTextCursor(tc);
}

/*
 * A helper method which writes the given string to the textbrowser
 */
void OutputDialog::writeToTabOutput(const QString &msg, TreatURLLinks treatURLLinks)
{
  utils::writeToTextBrowser(m_textBrowser, msg, treatURLLinks);
}

/*
 * Slot called whenever xbpsExec emits a new output
 */
void OutputDialog::onWriteOutput(const QString &output)
{
  utils::positionTextEditCursorAtEnd(m_textBrowser);
  m_textBrowser->insertHtml(output);
  m_textBrowser->ensureCursorVisible();
}

/*
 * Helper method to find the given "findText" in a TextEdit
 */
bool OutputDialog::textInTabOutput(const QString& findText)
{
  return (utils::strInQTextEdit(m_textBrowser, findText));
}

/*
 * Slot called whenever xbpsExec finishes its job
 */
void OutputDialog::xbpsProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  m_progressBar->close();

  if ((exitCode == 0) && exitStatus == QProcess::NormalExit)
  {
    writeToTabOutput("<br><b>" + StrConstants::getCommandFinishedOK() + "</b><br>");
  }
  else
  {
    writeToTabOutput("<br><b>" + StrConstants::getCommandFinishedWithErrors() + "</b><br>");
  }

  if (exitCode != 0 && (textInTabOutput("conflict"))) //|| _textInTabOutput("could not satisfy dependencies")))
  {
    int res = QMessageBox::question(this, StrConstants::getThereHasBeenATransactionError(),
                                    StrConstants::getConfirmExecuteTransactionInTerminal(),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
      m_xbpsExec->runLastestCommandInTerminal();
      return;
    }
  }

  if (m_xbpsExec != nullptr) delete m_xbpsExec;
  m_upgradeRunning = false;
}

/*
 * User wants to close the ouput window...
 */
void OutputDialog::cancelUpgrade()
{
  UnixCommand::execCommand("killall xbps-install");
}

/*
 * User changed text to search in the line edit
 */
void OutputDialog::onSearchBarTextChanged(QString strToSearch)
{
  utils::searchBarTextChangedInTextBrowser(m_textBrowser, m_searchBar, strToSearch);
}

/*
 * User closed the search bar
 */
void OutputDialog::onSearchBarClosed()
{
  utils::searchBarClosedInTextBrowser(m_textBrowser, m_searchBar);
}

/*
 * User requested next found string
 */
void OutputDialog::onSearchBarFindNext()
{
  utils::searchBarFindNextInTextBrowser(m_textBrowser, m_searchBar);
}

/*
 * User requested previous found string
 */
void OutputDialog::onSearchBarFindPrevious()
{
  utils::searchBarFindPreviousInTextBrowser(m_textBrowser, m_searchBar);
}

/*
 * Let's not exit the dialog if a system upgrade is running
 */
void OutputDialog::closeEvent(QCloseEvent *event)
{
  //We cannot quit while there is a running transaction!
  if(m_upgradeRunning)
  {
    int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                    StrConstants::getThereIsARunningTransaction() + "\n" +
                                    StrConstants::getDoYouReallyWantToQuit(),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
      cancelUpgrade();
      m_upgradeRunning = false;
      emit finished(-1);
      event->accept();
    }
    else
    {
      event->ignore();
    }
  }
  else
  {
    emit finished(0);
    event->accept();
  }
}

/*
 * Whenever user presses Ctrl+F, we show the searchbar again
 */
void OutputDialog::keyPressEvent(QKeyEvent *ke)
{
  if(ke->key() == Qt::Key_F && ke->modifiers() == Qt::ControlModifier)
  {
    m_searchBar->show();
  }
  else if(ke->key() == Qt::Key_Escape)
  {
    reject();
  }
  else ke->accept();
}

/*
 * Filters keypressevents from Console
 */
bool OutputDialog::eventFilter(QObject *, QEvent *event)
{
  if(event->type() == QKeyEvent::KeyRelease)
  {
    QKeyEvent *ke = static_cast<QKeyEvent*>(event);
    if (ke->key() == Qt::Key_Escape)
    {
      if (m_upgradeRunning)
      {
        int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                        StrConstants::getThereIsARunningTransaction() + "\n" +
                                        StrConstants::getDoYouReallyWantToQuit(),
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No);
        if (res == QMessageBox::Yes)
        {
          cancelUpgrade();
          m_upgradeRunning = false;
          reject();
          return true;
        }
        else
        {
          ke->ignore();
          return true;
        }
      }
      else
      {
        reject();
        return true;
      }
    }
    else if(ke->key() == Qt::Key_F && ke->modifiers() == Qt::ControlModifier)
    {
      m_console->toggleShowSearchBar();
    }
  }

  return false;
}
