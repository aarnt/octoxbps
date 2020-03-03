/*
* This file is part of Octopi, an open-source GUI for pacman.
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

#ifndef OUTPUTDIALOG_H
#define OUTPUTDIALOG_H

#include "../src/constants.h"

#include <QDialog>
#include <QProcess>

class XBPSExec;
class QString;
class QTextBrowser;
class QVBoxLayout;
class QProgressBar;
class SearchBar;
class QWidget;
class QCloseEvent;
class QKeyEvent;
class TermWidget;

class OutputDialog : public QDialog
{
  Q_OBJECT

private:
  QTextBrowser *m_textBrowser;
  QProgressBar *m_progressBar;
  QVBoxLayout *m_mainLayout;
  XBPSExec *m_xbpsExec;
  SearchBar *m_searchBar;
  TermWidget *m_console;
  bool m_upgradeRunning;
  bool m_debugInfo;
  bool m_upgradeXBPS;
  bool m_viewAsTextBrowser;

  void initAsTextBrowser();
  void initAsTermWidget();
  void positionTextEditCursorAtEnd();
  bool textInTabOutput(const QString& findText);
  void writeToTabOutput(const QString &msg, TreatURLLinks treatURLLinks = ectn_TREAT_URL_LINK);

private slots:
  void onPencertange(int percentage);
  void onWriteOutput(const QString &output);
  void pacmanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  //SearchBar slots
  void onSearchBarTextChanged(QString strToSearch);
  void onSearchBarClosed();
  void onSearchBarFindNext();
  void onSearchBarFindPrevious();

protected:
  virtual void closeEvent(QCloseEvent * event);
  virtual void keyPressEvent(QKeyEvent * ke);
  virtual bool eventFilter(QObject *, QEvent *);

public:
  explicit OutputDialog(QWidget *parent = 0);
  void setDebugMode(bool newValue);
  void setUpgradeXBPS(bool newValue);
  void setViewAsTextBrowser(bool value);

public slots:
  void show();
  void reject();

  void doSystemUpgrade();
  void doSystemUpgradeInTerminal();
  void onExecCommandInTabTerminal(QString command);
  void onPressAnyKeyToContinue();
  void onCancelControlKey();
};

#endif // OUTPUTDIALOG_H
