/*
* This file is part of OctoXBPS, an open-source GUI for xbps.
* Copyright (C) 2015 Alexandre Albuquerque Arnt
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

#ifndef XBPSEXEC_H
#define XBPSEXEC_H

#include <QObject>
#include "constants.h"
#include "unixcommand.h"

class XBPSExec : public QObject
{
  Q_OBJECT

private:
  bool m_iLoveCandy;
  bool m_debugMode;
  UnixCommand *m_unixCommand;
  CommandExecuting m_commandExecuting;
  QStringList m_lastCommandList; //run in terminal commands
  QStringList m_textPrinted;

  bool searchForKeyVerbs(QString output);
  bool splitOutputStrings(QString output);
  void parseXBPSProcessOutput(QString output);
  void prepareTextToPrint(QString str, TreatString ts = ectn_TREAT_STRING, TreatURLLinks tl = ectn_TREAT_URL_LINK);

private slots:
  //UnixCommand slots:
  void onStarted();
  void onReadOutput();
  void onReadOutputError();
  void onFinished(int exitCode, QProcess::ExitStatus);

public:
  explicit XBPSExec(QObject *parent = 0);
  virtual ~XBPSExec();

  void setDebugMode(bool value);
  void runLastestCommandInTerminal();
  void removeTemporaryFile();

  //static bool isDatabaseLocked();
  //static void removeDatabaseLock();

  //XBPS
  void doCleanCache();
  void doInstall(const QString &listOfPackages);
  void doInstallInTerminal(const QString &listOfPackages);

  //void doInstallLocal(const QString &listOfPackages);
  //void doInstallLocalInTerminal(const QString &listOfPackages);

  void doRemove(const QString &listOfPackages);
  void doRemoveInTerminal(const QString &listOfPackages);

  void doRemoveAndInstall(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall);
  void doRemoveAndInstallInTerminal(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall);

  void doSystemUpgrade(bool upgradeXBPS);
  void doSystemUpgradeInTerminal();
  void doSyncDatabase();

signals:
  void percentage(int);
  void started();
  void readOutput();
  void readOutputError();
  void finished(int exitCode, QProcess::ExitStatus);
  void textToPrintExt(QString m_textToPrint);

};

#endif // XBPSEXEC_H
