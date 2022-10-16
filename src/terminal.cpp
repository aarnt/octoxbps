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

#include "terminal.h"
#include "wmhelper.h"
#include "unixcommand.h"

#include <QApplication>
#include <QProcess>
#include <QTextStream>
#include <iostream>
#include <QDebug>

/*
 * This class abstracts all the relevant terminal related code
 */

Terminal::Terminal(QObject *parent) : QObject(parent)
{
}

Terminal::~Terminal()
{
  /*m_process->close();

  delete m_process;*/
}

/*
 * Executes the given command list with root credentials
 */
void Terminal::runCommandInTerminalWithSudo(const QString& command)
{
  QString cmd = getSudoCommand() + " " + UnixCommand::getShell() + " -c \"" + command + "\"";
  emit commandToExecInQTermWidget(cmd);
}

/*
 * Executes the given command list with root credentials
 */
void Terminal::runCommandInTerminal(const QStringList &commandList)
{
  QFile *ftemp = UnixCommand::getTemporaryFile();
  QTextStream out(ftemp);

  foreach(QString line, commandList)
    out << line;

  out.flush();
  ftemp->close();

  QString cmd = getSudoCommand() + " " + UnixCommand::getShell() + " -c \"" + ftemp->fileName() + "\"";
  emit commandToExecInQTermWidget(cmd);
}

/*
 * Executes the given command list as normal user
 */
void Terminal::runCommandInTerminalAsNormalUser(const QStringList &commandList)
{
  QFile *ftemp = UnixCommand::getTemporaryFile();
  QTextStream out(ftemp);

  foreach(QString line, commandList)
  {
    //We must remove the "ccr/" prefix in Chakra, cos this will not work
    if(line.contains("ccr/"))
    {
      line = line.replace("ccr/", "");
    }

    out << line;
  }

  out.flush();
  ftemp->close();

  QString cmd;

  if (m_selectedTerminal == ctn_AUTOMATIC)
  {
    if (UnixCommand::hasTheExecutable(ctn_RXVT_TERMINAL))
    {
      if (UnixCommand::isAppRunning("urxvtd"))
      {
        cmd = "urxvtc -name Urxvt -title Urxvt -e " + ftemp->fileName();
      }
      else
      {
        cmd = ctn_RXVT_TERMINAL + " -name Urxvt -title Urxvt -e " + ftemp->fileName();
      }
    }
    else if(WMHelper::isXFCERunning() && UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
      cmd = ctn_XFCE_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (WMHelper::isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE_TERMINAL))
    {
      cmd = ctn_KDE_TERMINAL + " --nofork -e sh -c " + ftemp->fileName();
    }
    else if (WMHelper::isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDE_TERMINAL)){
      cmd = ctn_TDE_TERMINAL + " --nofork -e " + ftemp->fileName();
    }
    else if (WMHelper::isLXDERunning() && UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
      cmd = ctn_LXDE_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (WMHelper::isMATERunning() && UnixCommand::hasTheExecutable(ctn_MATE_TERMINAL)){
      cmd = ctn_MATE_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (WMHelper::isLXQTRunning() && UnixCommand::hasTheExecutable(ctn_LXQT_TERMINAL)){
      cmd = ctn_LXQT_TERMINAL + " -e sh -c " + ftemp->fileName();
    }
    else if (WMHelper::isCinnamonRunning() && UnixCommand::hasTheExecutable(ctn_CINNAMON_TERMINAL)){
      cmd = ctn_CINNAMON_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
      cmd = ctn_XFCE_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
      cmd = ctn_LXDE_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (UnixCommand::hasTheExecutable(ctn_XTERM)){
      cmd = ctn_XTERM +
          " -fn \"*-fixed-*-*-*-18-*\" -fg White -bg Black -title xterm -e " + ftemp->fileName();
    }
    else {
      std::cerr << "ERROR: OctoPkg found no suitable terminal!" << std::endl;
      emit finishedTerminal(0, QProcess::CrashExit);
      return;
    }
  }
  else //User has chosen his own terminal...
  {
    if (m_selectedTerminal == ctn_RXVT_TERMINAL)
    {
      if (UnixCommand::isAppRunning("urxvtd"))
      {
        cmd = "urxvtc -name Urxvt -title Urxvt -e " + ftemp->fileName();
      }
      else
      {
        cmd = ctn_RXVT_TERMINAL + " -name Urxvt -title Urxvt -e " + ftemp->fileName();
      }
    }
    else if(m_selectedTerminal == ctn_XFCE_TERMINAL){
      cmd = ctn_XFCE_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (m_selectedTerminal == ctn_KDE_TERMINAL)
    {
      cmd = ctn_KDE_TERMINAL + " --nofork -e sh -c " + ftemp->fileName();
    }
    else if (m_selectedTerminal == ctn_TDE_TERMINAL){
      cmd = ctn_TDE_TERMINAL + " --nofork -e " + ftemp->fileName();
    }
    else if (m_selectedTerminal == ctn_LXDE_TERMINAL){
      cmd = ctn_LXDE_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (m_selectedTerminal == ctn_MATE_TERMINAL){
      cmd = ctn_MATE_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (m_selectedTerminal == ctn_CINNAMON_TERMINAL){
      cmd = ctn_CINNAMON_TERMINAL + " -e " + ftemp->fileName();
    }
    else if (m_selectedTerminal == ctn_LXQT_TERMINAL){
      cmd = ctn_LXQT_TERMINAL + " -e sh -c " + ftemp->fileName();
    }
    else if (m_selectedTerminal == ctn_XTERM){
      cmd = ctn_XTERM +
          " -fn \"*-fixed-*-*-*-18-*\" -fg White -bg Black -title xterm -e " + ftemp->fileName();
    }
  }

  //m_processWrapper->executeCommand(cmd);
}

/*
 * Retrieves the exact sudo command: doas or sudo
 */
QString Terminal::getSudoCommand()
{
  if (QFile::exists(QStringLiteral("/usr/bin/doas")) &&
      QFile::exists(QStringLiteral("/etc/doas.conf")))
    return "doas";
  else
    return "sudo";
}

/*
 * Retrives the list of available terminals in this system
 */
QStringList Terminal::getListOfAvailableTerminals()
{
  QStringList res;
  res.append(ctn_AUTOMATIC);

  if (UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL))
    res.append(ctn_XFCE_TERMINAL);

  if (UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL))
    res.append(ctn_LXDE_TERMINAL);

  if (UnixCommand::hasTheExecutable(ctn_LXQT_TERMINAL))
    res.append(ctn_LXQT_TERMINAL);

  if (UnixCommand::hasTheExecutable(ctn_KDE_TERMINAL))
    res.append(ctn_KDE_TERMINAL);

  if (UnixCommand::hasTheExecutable(ctn_TDE_TERMINAL))
    res.append(ctn_TDE_TERMINAL);

  if (UnixCommand::hasTheExecutable(ctn_CINNAMON_TERMINAL))
    res.append(ctn_CINNAMON_TERMINAL);

  if (UnixCommand::hasTheExecutable(ctn_MATE_TERMINAL))
    res.append(ctn_MATE_TERMINAL);

  if (UnixCommand::hasTheExecutable(ctn_RXVT_TERMINAL))
    res.append(ctn_RXVT_TERMINAL);

  if (UnixCommand::hasTheExecutable(ctn_XTERM))
    res.append(ctn_XTERM);

  res.removeDuplicates();
  res.sort();

  return res;
}
