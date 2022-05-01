/*
* This file is part of OctoXBPS, an open-source GUI for XBPS.
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

#include "unixcommand.h"
#include "strconstants.h"
#include "wmhelper.h"
#include "terminal.h"
#include <iostream>

#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QTextStream>
#include <QtNetwork/QNetworkInterface>
#include <QRegularExpression>
#include <QDebug>
#include <QTcpSocket>

/*
 * Collection of methods to execute many Unix commands
 */

QFile *UnixCommand::m_temporaryFile = 0;

/*
 * UnixCommand's constructor: the relevant environment english setting and the connectors
 */
UnixCommand::UnixCommand(QObject *parent): QObject()
{
  m_process = new QProcess(parent);
  m_terminal = new Terminal(parent);

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  m_process->setProcessEnvironment(env);

  QObject::connect(m_process, SIGNAL( started() ), this,
                   SIGNAL( started() ));
  QObject::connect(this, SIGNAL( started() ), this,
                   SLOT( processReadyReadStandardOutput() ));

  QObject::connect(m_process, SIGNAL( readyReadStandardOutput() ), this,
                   SIGNAL( readyReadStandardOutput() ));
  QObject::connect(this, SIGNAL( readyReadStandardOutput() ), this,
                   SLOT( processReadyReadStandardOutput() ));

  QObject::connect(m_process, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SIGNAL( finished ( int, QProcess::ExitStatus )) );
  QObject::connect(this, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SLOT( processReadyReadStandardOutput() ));

  QObject::connect(m_process, SIGNAL( readyReadStandardError() ), this,
                   SIGNAL( readyReadStandardError() ));
  QObject::connect(this, SIGNAL( readyReadStandardError() ), this,
                   SLOT( processReadyReadStandardError() ));

  //Terminal signals
  QObject::connect(m_terminal, SIGNAL( started()), this,
                   SIGNAL( started()));
  QObject::connect(m_terminal, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SIGNAL( finished ( int, QProcess::ExitStatus )) );
  QObject::connect(m_terminal, SIGNAL(commandToExecInQTermWidget(QString)), this,
                   SIGNAL(commandToExecInQTermWidget(QString)));
}

/*
 * Executes the CURL command and returns the StandardError Output, if result code <> 0.
 */
QString UnixCommand::runCurlCommand(QStringList& params){
  QProcess proc;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  proc.setProcessEnvironment(env);
  proc.start(QStringLiteral("curl"), params);
  proc.waitForStarted();
  proc.waitForFinished(-1);

  QString res("");

  if (proc.exitCode() != 0)
  {
    res = proc.readAllStandardError();
  }

  proc.close();
  return res;
}

/*
 * Cleans Pacman's package cache.
 * Returns true if finished OK
 */
bool UnixCommand::cleanXBPSCache()
{
  QProcess pacman;
  //QString commandStr = "/usr/bin/xbps-remove -O";
  QStringList sl;
  sl << ctn_OCTOXBPS_SUDO_PARAMS;
  sl << QStringLiteral("/usr/bin/xbps-remove");
  sl << QStringLiteral("-O");
  pacman.start(WMHelper::getSUCommand(), sl);
  pacman.waitForFinished();

  return (pacman.exitCode() == 0);
}

/*
 * Performs a pacman query
 */
QByteArray UnixCommand::performQuery(const QStringList args)
{
  QByteArray result("");
  QProcess pacman;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  env.insert("LC_ALL", "C");
  pacman.setProcessEnvironment(env);
  pacman.start("pkg", args);
  pacman.waitForFinished();
  result = pacman.readAllStandardOutput();
  pacman.close();

  return result;
}

/*
 * Performs a pacman query
 * Overloaded with QString parameter
 */
QByteArray UnixCommand::performQuery(const QString &args)
{
  QByteArray result("");
  QProcess pacman;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove("COLUMNS");
  env.insert("COLUMNS", "170");
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  env.insert("LC_ALL", "C");
  pacman.setProcessEnvironment(env);

  QStringList sl = args.split(QStringLiteral(" "), Qt::SkipEmptyParts);
  QString command=sl.at(0);
  sl.removeFirst();
  pacman.start("/usr/bin/xbps-" + command, sl);
  pacman.waitForFinished();
  result = pacman.readAllStandardOutput();
  pacman.close();
  return result;
}

/*
 * Returns a string containing all AUR packages given a searchString parameter
 */
QByteArray UnixCommand::getRemotePackageList(const QString &searchString, bool useCommentSearch)
{
  QByteArray result("");

  if (useCommentSearch)
    result = performQuery("query -Rs " + searchString);
  else
    result = performQuery("query -Rs " + searchString);

  return result;
}

/*
 * Returns a string containing all packages no one depends on
 */
QByteArray UnixCommand::getUnrequiredPackageList()
{
  QByteArray result = performQuery("query -m");
  return result;
}

/*
 * Returns a string containing all packages that are outdated since last DB sync
 */
QByteArray UnixCommand::getOutdatedPackageList()
{
  //QByteArray result = "qt5-x11extras-5.5.0_2 update x86_64 http://repo.voidlinux.eu/current\nqtchooser-52_1 update x86_64 http://repo.voidlinux.eu/current\nrtkit-0.11_12 update x86_64 http://repo.voidlinux.eu/current\nsudo-1.8.14p3_1 update x86_64 http://repo.voidlinux.eu/current";
  QByteArray result = performQuery("install -un");
  return result;
}

/*
 * Retrieves the dependencies pkg list
 */
QByteArray UnixCommand::getDependenciesList(const QString &pkgName)
{
  QByteArray result = performQuery("query -x " + pkgName);
  if (result.isEmpty())
  {
    result = performQuery("query -Rx " + pkgName);
  }

  return result;
}

/*
 * Returns a string with the list of all packages available in all repositories
 * (installed + not installed)
 *
 * @param pkgName Used while the user is searching for the pkg that provides a certain file
 */
QByteArray UnixCommand::getPackageList(const QString &pkgName)
{
  QByteArray result;

  if (pkgName.isEmpty())
  {
#ifdef UNIFIED_SEARCH
    result = performQuery("query -Rs -");
#else
    result = performQuery("query -l");
#endif
  }
  else
  {
  }

  return result;
}

/*
 * Given a package name and if it is default to the official repositories,
 * returns a string containing all of its information fields
 * (ex: name, description, version, dependsOn...)
 */
QByteArray UnixCommand::getPackageInformation(const QString &pkgName, bool foreignPackage = false)
{
  Q_UNUSED(foreignPackage)
  QString args;

  if(isPackageInstalled(pkgName))
  {
    args = "query " + pkgName;
  }
  else
  {
    args = "query -R " + pkgName;
  }

  QByteArray result = performQuery(args);
  return result;
}

/*
 * Given a package name, returns a string containing all the files inside it
 */
QByteArray UnixCommand::getPackageContentsUsingXBPS(const QString& pkgName, bool isInstalled)
{
  QString extraArg="";
  if (!isInstalled) extraArg = " --repository ";

  QByteArray res = performQuery("query " + extraArg + " -f " + pkgName);
  return res;
}

/*
 * Given a complete file path, returns the package that provides that file
 */
QString UnixCommand::getPackageByFilePath(const QString &filePath)
{
  QString pkgName="";
  QString out = performQuery("query -o " + filePath);

  if (!out.isEmpty())
  {
    int pos = out.indexOf(":");
    if (pos != -1)
    {      
      pkgName = out.left(pos);
      //Now we have to remove the pkg version...
      int dash = pkgName.lastIndexOf("-");
      if (dash != -1)
      {
        pkgName = pkgName.left(dash);
      }
    }
  }

  return pkgName;
}

/*
 * Based on the given file, we use 'slocate' to suggest complete paths
 */
QStringList UnixCommand::getFilePathSuggestions(const QString &file)
{
  QProcess slocate;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  slocate.setProcessEnvironment(env);
  QStringList sl;
  sl << QStringLiteral("-l");
  sl << QStringLiteral("8");
  sl << file;
  slocate.start(QStringLiteral("slocate"), sl);
  slocate.waitForFinished();

  QString ba = slocate.readAllStandardOutput();
  return ba.split("\n", Qt::SkipEmptyParts);
}

/*
 * Retrieves the list of targets needed to update the entire system or a given package
 */
QByteArray UnixCommand::getTargetUpgradeList(const QString &pkgName)
{
  QString args;
  QByteArray res = "";

  if(!pkgName.isEmpty())
  {
    args = "install -n -Rs " + pkgName;
    res = performQuery(args);
  }
  else //pkg upgrade
  {
    args = "install -un";
    res = performQuery(args);
  }

  return res;
}

/*
 * Given a package name, retrieves the list of all targets needed for its removal
 */
QByteArray UnixCommand::getTargetRemovalList(const QString &pkgName)
{
  QString args;
  QByteArray res = "";

  if(!pkgName.isEmpty())
  {
    args = "remove -R -n " + pkgName;
    res = performQuery(args);
  }

  return res;
}

/*
 * Retrieves the given field for a local package search
 */
QByteArray UnixCommand::getFieldFromLocalPackage(const QString &field, const QString &pkgName)
{
  QByteArray res = performQuery("query -p " + field + " " + pkgName);
  return res;
}

/*
 * Retrieves the given field for a remote package search
 */
QByteArray UnixCommand::getFieldFromRemotePackage(const QString &field, const QString &pkgName)
{
  QByteArray res = performQuery("query -R -p " + field + " " + pkgName);
  return res;
}

/*
 * Checks if we have internet access!
 */
bool UnixCommand::hasInternetConnection()
{
  QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
  bool result = false;

  for (int i = 0; i < ifaces.count(); i++){
    QNetworkInterface iface = ifaces.at(i);

    if ( iface.flags().testFlag(QNetworkInterface::IsUp)
         && !iface.flags().testFlag(QNetworkInterface::IsLoopBack) ){
      for (int j=0; j<iface.addressEntries().count(); j++){
        /*
         We have an interface that is up, and has an ip address
         therefore the link is present.

         We will only enable this check on first positive,
         all later results are incorrect
        */
        if (result == false)
          result = true;
      }
    }
  }

  //It seems to be alright, but let's make a ping to see the result
  /*if (result == true)
  {
    result = UnixCommand::doInternetPingTest();
  }*/

  return result;
}

/*
 * Pings google site, to make sure internet is OK
 */
bool UnixCommand::doInternetPingTest()
{
  QTcpSocket socket;
  QString hostname = QStringLiteral("www.google.com");

  socket.connectToHost(hostname, 80);
  if (socket.waitForConnected(5000))
    return true;
  else
  {
    hostname = QStringLiteral("www.baidu.com");
    socket.connectToHost(hostname, 80);
    if (socket.waitForConnected(5000))
      return true;
    else
      return false;
  }
}

/*
 * Checks if the given executable is available somewhere in the system
 */
bool UnixCommand::hasTheExecutable( const QString& exeName )
{
  QProcess proc;
  proc.setProcessChannelMode(QProcess::MergedChannels);
  QString sParam = QLatin1String("which ") + exeName;

  QStringList sl;
  sl << QLatin1String("-c");
  sl << sParam;

  proc.start(QLatin1String("/bin/sh"), sl);
  proc.waitForFinished();

  QString out = QString::fromUtf8(proc.readAllStandardOutput());
  proc.close();

  if (out.isEmpty() || out.count(QStringLiteral("which")) > 0) return false;
  else return true;
}

/*
 * Does some garbage collection, removing uneeded files
 */
void UnixCommand::removeTemporaryFiles()
{
  QDir tempDir(QDir::tempPath());
  QStringList nameFilters;
  nameFilters << "qtsingleapp*" << "gpg*" << ".qt_temp_*";

  QFileInfoList list = tempDir.entryInfoList(nameFilters, QDir::Dirs | QDir::Files | QDir::System | QDir::Hidden);
  tempDir.setPath(QDir::homePath() + QDir::separator() + ".config/octoxbps" + QDir::separator());
  QFileInfoList list2 = tempDir.entryInfoList(nameFilters, QDir::Dirs | QDir::Files | QDir::System | QDir::Hidden);
  list.append(list2);

  foreach(QFileInfo file, list){
    QFile fileAux(file.filePath());

    if (!file.isDir()){
      fileAux.remove();
    }
    else{
      QDir dir(file.filePath());
      QFileInfoList listd = dir.entryInfoList(QDir::Files | QDir::System);

      foreach(QFileInfo filed, listd){
        QFile fileAuxd(filed.filePath());
        fileAuxd.remove();
      }

      dir.rmdir(file.filePath());
    }
  }
}

/*
 * Given a filename, checks if it is a text file
 */
bool UnixCommand::isTextFile(const QString& fileName)
{
  QProcess *p = new QProcess();
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  p->setProcessEnvironment(env);

  QStringList s(fileName);
  p->start("file", s);
  p->waitForFinished();

  QByteArray output = p->readAllStandardOutput();
  p->close();
  delete p;

  int from = output.indexOf(":", 0)+1;

  return (((output.indexOf( "ASCII", from ) != -1) ||
          (output.indexOf( "text", from ) != -1) ||
          (output.indexOf( "empty", from ) != -1)) &&
          (output.indexOf( "executable", from) == -1));
}

/*
 * Retrieves pkgNG version.
 */
QString UnixCommand::getXBPSVersion()
{
  QString v = performQuery("query -V");
  return v;
}

/*
 * Returns the SHELL environment variable, if not set defaults to bash.
 */
QString UnixCommand::getShell()
{
  if (QFile::exists("/usr/bin/bash"))
    return "bash";

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QString shell = env.value("SHELL", "/bin/bash");

  QFileInfo fi(shell);
  return fi.fileName();
}

/*
 * Executes given commandToRun inside a terminal, so the user can interact
 */
void UnixCommand::runCommandInTerminal(const QStringList& commandList){
  m_terminal->runCommandInTerminal(commandList);
}

/*
 * Executes the given command using QProcess async technology with ROOT credentials
 */
void UnixCommand::executeCommand(const QString &pCommand, Language lang)
{
  QString command;

  if (lang == ectn_LANG_USER_DEFINED)
  {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("LANG");
    env.remove("LC_MESSAGES");
    env.insert("LANG", QLocale::system().name() + ".UTF-8");
    env.insert("LC_MESSAGES", QLocale::system().name() + ".UTF-8");
    m_process->setProcessEnvironment(env);
  }

  QStringList sl;
  sl = pCommand.split(QStringLiteral(" "), Qt::SkipEmptyParts);
  sl.insert(0, ctn_OCTOXBPS_SUDO_PARAMS);

  m_process->start(WMHelper::getSUCommand(), sl);
}

/*
 * Executes given pCommand without the need to create an UnixCommand obj
 */
void UnixCommand::execCommand(const QString &pCommand, const QStringList &params)
{
  QProcess p;
  p.start(pCommand, params);
  p.waitForStarted(-1);
  p.waitForFinished(-1);
  p.close();
}

/*
 * Puts all Standard output of the member process into a member string
 */
void UnixCommand::processReadyReadStandardOutput()
{
  if (m_process->isOpen())
    m_readAllStandardOutput = m_process->readAllStandardOutput();
}

/*
 * Puts all StandardError output of the member process into a member string
 */
void UnixCommand::processReadyReadStandardError()
{
  if (m_process->isOpen())
  {
    m_readAllStandardError = m_process->readAllStandardError();
    m_errorString = m_process->errorString();
  }
}

/*
 * Retrieves Standard output of member process
 */
QString UnixCommand::readAllStandardOutput()
{
  return m_readAllStandardOutput;
}

/*
 * Retrieves StandardError output of member process
 */
QString UnixCommand::readAllStandardError()
{
  return m_readAllStandardError;
}

/*
 * Retrieves ErrorString of member process
 */
QString UnixCommand::errorString()
{
  return m_errorString;
}

/*
 * If justOneInstance = false (default), returns TRUE if one instance of the app is ALREADY running
 * Otherwise, it returns TRUE if the given app is running.
 */
bool UnixCommand::isAppRunning(const QString &appName, bool justOneInstance)
{
  QStringList slParam;
  QProcess proc;

  slParam << "-C";
  //ps only works with 15 char process names
  QString app = appName.left(15);
  slParam << app;
  proc.start("ps", slParam);
  proc.waitForFinished();

  QString out = proc.readAll();
  proc.close();

  if (justOneInstance)
  {
    if (out.count(app)>0)
      return true;
    else
      return false;
  }
  else
  {
    if (out.count(app)>1)
      return true;
    else
      return false;
  }
}

/*
 * Given a 'pkgName' package name, checks if that one is installed in the system
 */
bool UnixCommand::isPackageInstalled(const QString &pkgName)
{
  QProcess xbps;
  QStringList sl;
  sl << QStringLiteral("-S");
  sl << pkgName;
  xbps.start(QStringLiteral("/usr/bin/xbps-query"), sl);
  xbps.waitForFinished();
  return (xbps.exitCode() == 0);
}

/*
 * Retrieves the BSDFlavour where OctoPkg is running on!
 * Reads file "/etc/os-release" and searchs for compatible OctoPkg BSDs
 */
LinuxDistro UnixCommand::getLinuxDistro()
{
  static LinuxDistro ret;
  static bool firstTime = true;

  if (firstTime)
  {
    if (QFile::exists("/etc/trident-login.d"))
    {
      ret = ectn_TRIDENT;
    }
    else if (QFile::exists("/etc/os-release"))
    {
      QFile file("/etc/os-release");

      if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        ret = ectn_UNKNOWN;

      QString contents = file.readAll();

      if (contents.contains("ID=\"void\""))
      {
        ret = ectn_VOID;
      }
      else
      {
        ret = ectn_UNKNOWN;
      }
    }

    firstTime = false;
  }

  return ret;
}
