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

#include "xbpsexec.h"
#include "strconstants.h"
#include "unixcommand.h"
#include "wmhelper.h"

#include <QRegularExpression>
#include <QDebug>

/*
 * This class decouples xbps commands executing and parser code from OctoXBPS's interface
 */

/*
 * Let's create the needed unixcommand object that will ultimately execute xbps commands
 */
XBPSExec::XBPSExec(QObject *parent) : QObject(parent)
{
  m_unixCommand = new UnixCommand(parent);
  m_debugMode = false;

  QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( onStarted()));

  QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( onFinished(int, QProcess::ExitStatus)));

  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                   this, SLOT( onReadOutput()));

  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                   this, SLOT( onReadOutputError()));

  QObject::connect(m_unixCommand, SIGNAL(commandToExecInQTermWidget(QString)),
                   this, SIGNAL(commandToExecInQTermWidget(QString)));
}

/*
 * Let's remove UnixCommand temporary file...
 */
XBPSExec::~XBPSExec()
{
  m_unixCommand->removeTemporaryFile();
}

/*
 * Turns DEBUG MODE on or off
 */
void XBPSExec::setDebugMode(bool value)
{
  m_debugMode = value;
}

/*
 * Removes Octopi's temporary transaction file
 */
void XBPSExec::removeTemporaryFile()
{
  m_unixCommand->removeTemporaryFile();
}

/*
 * Searches the given output for a series of verbs that a xbps transaction may produce
 */
bool XBPSExec::searchForKeyVerbs(QString output)
{
  return (output.contains(QRegularExpression("checking ")) ||
          output.contains(QRegularExpression("loading ")) ||
          output.contains(QRegularExpression("installing ")) ||
          output.contains(QRegularExpression("upgrading ")) ||
          output.contains(QRegularExpression("downgrading ")) ||
          output.contains(QRegularExpression("resolving ")) ||
          output.contains(QRegularExpression("looking ")) ||
          output.contains(QRegularExpression("removing ")));
}

/*
 * Breaks the output generated by QProcess so we can parse the strings
 * and give a better feedback to our users (including showing percentages)
 *
 * Returns true if the given output was split
 */
bool XBPSExec::splitOutputStrings(QString output)
{
  bool res = true;
  QString msg = output.trimmed();
  QStringList msgs = msg.split(QRegularExpression("\\n"), Qt::SkipEmptyParts);

  foreach (QString m, msgs)
  {
    QStringList m2 = m.split(QRegularExpression("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "), Qt::SkipEmptyParts);

    if (m2.count() == 1)
    {
      //Let's try another test... if it doesn't work, we give up.
      QStringList maux = m.split(QRegularExpression("%"), Qt::SkipEmptyParts);
      if (maux.count() > 1)
      {
        foreach (QString aux, maux)
        {
          aux = aux.trimmed();
          if (!aux.isEmpty())
          {
            if (aux.at(aux.length()-1).isDigit())
            {
              aux += "%";
            }

            if (m_debugMode) qDebug() << "_split - case1: " << aux;
            parseXBPSProcessOutput(aux);
          }
        }
      }
      else if (maux.count() == 1)
      {
        if (!m.isEmpty())
        {
          if (m_debugMode) qDebug() << "_split - case2: " << m;
          parseXBPSProcessOutput(m);
        }
      }
    }
    else if (m2.count() > 1)
    {
      foreach (QString m3, m2)
      {
        if (!m3.isEmpty())
        {
          if (m_debugMode) qDebug() << "_split - case3: " << m3;
          parseXBPSProcessOutput(m3);
        }
      }
    }
    else res = false;
  }

  return res;
}

/*
 * Processes the output of the 'xbps process' so we can update percentages and messages at real time
 */
void XBPSExec::parseXBPSProcessOutput(QString output)
{
  if (m_commandExecuting == ectn_RUN_IN_TERMINAL ||
      m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL) return;

  bool continueTesting = false;
  QString perc;
  QString msg = output;
  QString progressRun;
  QString progressEnd;
  QString target;
  msg.remove(QRegularExpression(".+\\[Y/n\\].+"));

  //Let's remove color codes from strings...
  msg.remove("\033[0;1m");
  msg.remove("\033[0m");
  msg.remove("[1;33m");
  msg.remove("[00;31m");
  msg.remove("\033[1;34m");
  msg.remove("\033[0;1m");
  msg.remove("c");
  msg.remove("C");
  msg.remove("");
  msg.remove("[m[0;37m");
  msg.remove("o");
  msg.remove("[m");
  msg.remove(";37m");
  msg.remove("[c");
  msg.remove("[mo");
  msg.remove("[1A[K");

  if (m_debugMode) qDebug() << "_treat: " << msg;

  progressRun = "%";
  progressEnd = "100%";

  //If it is a percentage, we are talking about curl output...
  if(msg.indexOf(progressEnd) != -1)
  {
    perc = "100%";
    emit percentage(100);
    continueTesting = true;
  }

  if ((msg.contains(".xbps:") || msg.contains(".xbps.sig:")) && msg.contains("%"))
  {
    //We're dealing with packages being downloaded
    int colon = msg.indexOf(":");
    target = msg.left(colon);
    target = target.remove("]").trimmed();

    if(!m_textPrinted.contains(target))
      prepareTextToPrint("<b><font color=\"#B4AB58\">" + target + "</font></b>");
  }
  else if (msg.contains("Updating") &&
            (!msg.contains(QRegularExpression("B/s")) && (!msg.contains(QRegularExpression("configuration file")))))
  {
    int p = msg.indexOf("'");
    if (p == -1) return; //Guard!

    if (msg.left(p).contains("Updating repository `"))
      target = msg.left(p).remove("Updating repository `").trimmed();
    else if (msg.left(p).contains("Updating `"))           //Legacy code xbps < 0.59
      target = msg.left(p).remove("Updating `").trimmed(); //Legacy code xbps < 0.59

    target.remove("[*] ");
    target.remove("'");

    if(!m_textPrinted.contains(target))
    {
      prepareTextToPrint("Updating repository " + target, ectn_TREAT_STRING, ectn_DONT_TREAT_URL_LINK);
    }

    return;
  }
  else if (msg.contains("[*] Verifying package integrity") ||
           msg.contains("[*] Collecting package files") ||
           msg.contains("[*] Unpacking packages") ||
           msg.contains("[*] Configuring unpacked packages"))
  {
    emit percentage(100);
  }

  if (msg.indexOf(progressRun) != -1 || continueTesting)
  {
    int p = msg.indexOf("%");
    if (p == -1 || (p-3 < 0) || (p-2 < 0)) return; //Guard!

    if (msg.at(p-2).isSpace())
      perc = msg.mid(p-1, 2).trimmed();
    else if (msg.at(p-3).isSpace())
      perc = msg.mid(p-2, 3).trimmed();

    if (m_debugMode) qDebug() << "percentage is: " << perc;

    //Here we print the transaction percentage updating
    if(!perc.isEmpty() && perc.indexOf("%") > 0)
    {
      int ipercentage = perc.left(perc.size()-1).toInt();
      emit percentage(ipercentage);
    }
  }
  //It's another error, so we have to output it
  else
  {
    if (msg.contains(QRegularExpression("ETA")) ||
      msg.contains(QRegularExpression("KiB")) ||
      msg.contains(QRegularExpression("MiB")) ||
      //msg.contains(QRegularExpression("KB/s")) ||
      msg.contains(QRegularExpression("B/s")) ||
      msg.contains(QRegularExpression("[0-9]+ B")) ||
      msg.contains(QRegularExpression("[0-9]{2}:[0-9]{2}"))) return;

    //Let's supress some annoying string bugs...
    msg.remove(QRegularExpression("\\(process.+"));
    msg.remove(QRegularExpression("Using the fallback.+"));
    msg.remove(QRegularExpression("Gkr-Message:.+"));
    msg.remove(QRegularExpression("kdesu.+"));
    msg.remove(QRegularExpression("kbuildsycoca.+"));
    msg.remove(QRegularExpression("Connecting to deprecated signal.+"));
    msg.remove(QRegularExpression("QVariant.+"));
    msg.remove(QRegularExpression("libGL.+"));
    msg.remove(QRegularExpression("Password.+"));
    msg.remove(QRegularExpression("GConf Error:.+"));
    msg.remove(QRegularExpression(":: Do you want.+"));
    msg.remove(QRegularExpression("org\\.kde\\."));
    msg.remove(QRegularExpression("QCommandLineParser"));
    msg.remove(QRegularExpression("QCoreApplication.+"));
    msg.remove(QRegularExpression("Fontconfig warning.+"));
    msg.remove(QRegularExpression("reading configurations from.+"));
    msg.remove(QRegularExpression(".+annot load library.+"));
    msg.remove(QRegularExpression("pci id for fd \\d+.+"));
    msg.remove(QRegularExpression("qt.qpa.xcb.+"));
    msg.remove(QRegularExpression("qt.qpa.plugin: Could not.+"));
    msg.remove(QRegularExpression("qt5ct: using qt5ct plugin"));

    QString order;
    int ini = msg.indexOf(QRegularExpression("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "));

    if (ini == 0)
    {
      int rp = msg.indexOf(")");
      if (rp == -1) return; //Guard!

      order = msg.left(rp+2);
      msg = msg.remove(0, rp+2);
    }

    if (!msg.isEmpty())
    {
      if (msg.contains(QRegularExpression("removing ")) && !m_textPrinted.contains(msg + " "))
      {
        //Does this package exist or is it a proccessOutput buggy string???
        QString pkgName = msg.mid(9).trimmed();

        if (pkgName.indexOf("...") != -1 || UnixCommand::isPackageInstalled(pkgName))
        {
          prepareTextToPrint("<b><font color=\"#E55451\">" + msg + "</font></b>"); //RED
        }
      }
      else
      {
        QString altMsg = msg;
        prepareTextToPrint(altMsg); //BLACK
      }
    }
  }
}

/*
 * Prepares a string parsed from xbps output to be printed by the UI
 */
void XBPSExec::prepareTextToPrint(QString str, TreatString ts, TreatURLLinks tl)
{
  if (m_debugMode) qDebug() << "_print: " << str;

  if (ts == ectn_DONT_TREAT_STRING)
  {
    emit textToPrintExt(str);
    return;
  }

  //If the msg waiting to being print is from curl status OR any other unwanted string...
  if ((str.contains(QRegularExpression("\\(\\d")) &&
       (!str.contains("target", Qt::CaseInsensitive)) &&
       (!str.contains("package", Qt::CaseInsensitive))) ||
      (str.contains(QRegularExpression("\\d\\)")) &&
       (!str.contains("target", Qt::CaseInsensitive)) &&
       (!str.contains("package", Qt::CaseInsensitive))) ||
      str.indexOf("Enter a selection", Qt::CaseInsensitive) == 0 ||
      str.indexOf("Proceed with", Qt::CaseInsensitive) == 0 ||
      str.indexOf("%") != -1 ||
      str.indexOf("---") != -1 ||
      str.indexOf("removed obsolete entry") != -1 ||
      str.indexOf("avg rate") != -1)
  {
    return;
  }

  //If the msg waiting to being print has not yet been printed...
  if(m_textPrinted.contains(str))
  {
    return;
  }

  QString newStr = str;

  if (newStr.contains(QRegularExpression("\\d+ downloaded, \\d+ installed, \\d+ updated, \\d+ configured, \\d+ removed")))
  {
    newStr = "<b>" + newStr + "</b>";
  }
  else if(newStr.contains(QRegularExpression("<font color")))
  {
    newStr += "<br>";
  }
  else
  {
    if(newStr.contains(QRegularExpression("removing ")) ||
       newStr.contains(QRegularExpression("could not ")) ||
       newStr.contains(QRegularExpression("failed")) ||
       newStr.contains(QRegularExpression("is not synced")) ||
       newStr.contains(QRegularExpression("[Rr]emoving")) ||
       newStr.contains(QRegularExpression("[Dd]einstalling")) ||
       newStr.contains(QRegularExpression("could not be found")))
    {
      newStr = "<b><font color=\"#E55451\">" + newStr + "&nbsp;</font></b>"; //RED
    }
    else if(newStr.contains(QRegularExpression("Verifying")) ||
            newStr.contains(QRegularExpression("Building")) ||
            newStr.contains(QRegularExpression("Checking")) ||
            newStr.contains(QRegularExpression("Configuring")) ||
            newStr.contains(QRegularExpression("Downloading")) ||
            newStr.contains(QRegularExpression("Reinstalling")) ||
            newStr.contains(QRegularExpression("Installing")) ||
            newStr.contains(QRegularExpression("Updating")) ||
            newStr.contains(QRegularExpression("Upgrading")) ||
            newStr.contains(QRegularExpression("Loading")) ||
            newStr.contains(QRegularExpression("Resolving")) ||
            newStr.contains(QRegularExpression("Extracting")) ||
            newStr.contains(QRegularExpression("Unpacking")) ||
            newStr.contains(QRegularExpression("Running")) ||
            newStr.contains(QRegularExpression("Looking")))
    {
      if (newStr.startsWith("Updating repository", Qt::CaseInsensitive))
      {
        //Updating repository
        QString target = newStr.mid(20);
        newStr = "<b><font color=\"#4BC413\">Updating repository</font></b> " + target; //GREEN
      }
      else newStr = "<b><font color=\"#4BC413\">" + newStr + "</font></b>"; //GREEN
    }
    else if (newStr.contains(QRegularExpression("warning")) ||
             newStr.contains(QRegularExpression("downgrading")) ||
             newStr.contains(QRegularExpression("options changed")))
    {
      newStr = "<b><font color=\"#FF8040\">" + newStr + "</font></b>"; //ORANGE
    }
    else if (newStr.contains("-") &&
             (!newStr.contains(QRegularExpression("(is|are) up-to-date"))) &&
             (!newStr.contains(QRegularExpression("\\s"))))
    {
      newStr = "<b><font color=\"#B4AB58\">" + newStr + "</font></b>"; //IT'S A PKGNAME!
    }
  }

  if (newStr.contains("::"))
  {
    newStr = "<br><B>" + newStr + "</B><br><br>";
  }

  if (!newStr.contains(QRegularExpression("<br"))) //It was an else!
  {
    newStr += "<br>";
  }

  if (tl == ectn_TREAT_URL_LINK)
    newStr = Package::makeURLClickable(newStr);

  m_textPrinted.append(str);

  emit textToPrintExt(newStr);
}

/*
 * Whenever QProcess starts the xbps command...
 */
void XBPSExec::onStarted()
{
  //First we output the name of action we are starting to execute!
  if (m_commandExecuting == ectn_CLEAN_CACHE)
  {
    prepareTextToPrint("<b>" + StrConstants::getCleaningPackageCache() + "</b><br><br>", ectn_DONT_TREAT_STRING, ectn_DONT_TREAT_URL_LINK);
  }
  else if (m_commandExecuting == ectn_SYNC_DATABASE)
  {
    prepareTextToPrint("<b>" + StrConstants::getSyncDatabases() + "</b><br><br>", ectn_DONT_TREAT_STRING, ectn_DONT_TREAT_URL_LINK);
  }
  else if (m_commandExecuting == ectn_SYSTEM_UPGRADE || m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
  {
    prepareTextToPrint("<b>" + StrConstants::getSystemUpgrade() + "</b><br><br>", ectn_DONT_TREAT_STRING, ectn_DONT_TREAT_URL_LINK);
  }
  else if (m_commandExecuting == ectn_REMOVE)
  {
    prepareTextToPrint("<b>" + StrConstants::getRemovingPackages() + "</b><br><br>", ectn_DONT_TREAT_STRING, ectn_DONT_TREAT_URL_LINK);
  }
  else if (m_commandExecuting == ectn_INSTALL)
  {
    prepareTextToPrint("<b>" + StrConstants::getInstallingPackages() + "</b><br><br>", ectn_DONT_TREAT_STRING, ectn_DONT_TREAT_URL_LINK);
  }
  else if (m_commandExecuting == ectn_REMOVE_INSTALL)
  {
    prepareTextToPrint("<b>" + StrConstants::getRemovingAndInstallingPackages() + "</b><br><br>", ectn_DONT_TREAT_STRING, ectn_DONT_TREAT_URL_LINK);
  }
  else if (m_commandExecuting == ectn_RUN_IN_TERMINAL)
  {
    prepareTextToPrint("<b>" + StrConstants::getRunningCommandInTerminal() + "</b><br><br>", ectn_DONT_TREAT_STRING, ectn_DONT_TREAT_URL_LINK);
  }

  QString output = m_unixCommand->readAllStandardOutput();
  output = output.trimmed();

  if (!output.isEmpty())
  {
    prepareTextToPrint(output);
  }

  emit started();
}

/*
 * Whenever QProcess' read output is retrieved...
 */
void XBPSExec::onReadOutput()
{
  if (WMHelper::getSUCommand().contains("kdesu"))
  {
    QString output = m_unixCommand->readAllStandardOutput();

    if (m_commandExecuting == ectn_SYNC_DATABASE &&
        output.contains("Usage: /usr/bin/kdesu [options] command"))
    {
      emit readOutput();
      return;
    }

    output = output.remove("Fontconfig warning: \"/etc/fonts/conf.d/50-user.conf\", line 14:");
    output = output.remove("reading configurations from ~/.fonts.conf is deprecated. please move it to /home/arnt/.config/fontconfig/fonts.conf manually");

    if (!output.trimmed().isEmpty())
    {
      splitOutputStrings(output);
    }
  }
  else if (WMHelper::getSUCommand().contains("gksu"))
  {
    QString output = m_unixCommand->readAllStandardOutput();
    output = output.trimmed();

    if(!output.isEmpty() &&
       output.indexOf(":: Synchronizing package databases...") == -1 &&
       output.indexOf(":: Starting full system upgrade...") == -1)
    {
      prepareTextToPrint(output);
    }
  }

  emit readOutput();
}

/*
 * Whenever QProcess' read error output is retrieved...
 */
void XBPSExec::onReadOutputError()
{
  QString msg = m_unixCommand->readAllStandardError();
  msg = msg.remove("Fontconfig warning: \"/etc/fonts/conf.d/50-user.conf\", line 14:");
  msg = msg.remove("reading configurations from ~/.fonts.conf is deprecated. please move it to /home/arnt/.config/fontconfig/fonts.conf manually");

  if (!msg.trimmed().isEmpty())
  {
    splitOutputStrings(msg);
  }

  emit readOutputError();
}

/*
 * Whenever QProcess finishes the xbps command...
 */
void XBPSExec::onFinished(int exitCode, QProcess::ExitStatus es)
{
  emit finished(exitCode, es);
}

// --------------------- DO METHODS ------------------------------------

/*
 * Cleans XBPS's package cache.
 */
void XBPSExec::doCleanCache()
{
  QString command = UnixCommand::getXBPSRemoveBin() + " -O";
  m_lastCommandList.clear();

  m_commandExecuting = ectn_CLEAN_CACHE;
  m_unixCommand->executeCommand(command);
}

/*
 * Calls xbps to install given packages and returns output to UI
 */
void XBPSExec::doInstall(const QString &listOfPackages)
{
  //QString command = "/usr/bin/xbps-install -y " + listOfPackages;
  QString command = UnixCommand::getXBPSInstallBin() + " -y " + listOfPackages;


  m_lastCommandList.clear();
  //m_lastCommandList.append("/usr/bin/xbps-install " + listOfPackages + ";");
  m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " " + listOfPackages + ";");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_commandExecuting = ectn_INSTALL;
  m_unixCommand->executeCommand(command);
}

/*
 * Calls xbps to install given packages inside a terminal
 */
void XBPSExec::doInstallInTerminal(const QString &listOfPackages)
{
  m_lastCommandList.clear();
  //m_lastCommandList.append("/usr/bin/xbps-install " + listOfPackages + ";");
  m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " " + listOfPackages + ";");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_unixCommand->runCommandInTerminal(m_lastCommandList);
}

/*
 * Calls xbps to install given LOCAL packages and returns output to UI
 */
void XBPSExec::doInstallLocal(const QString &targetPath, const QString &listOfPackages)
{
  //QString command = "xbps-install -S -y --repository " + targetPath + " " + listOfPackages;
  QString command = UnixCommand::getXBPSInstallBin() + " -S -y --repository " + targetPath + " " + listOfPackages;

  m_lastCommandList.clear();
  //m_lastCommandList.append("xbps-install -S --repository " + targetPath + " " + listOfPackages + ";");
  m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " -S --repository " + targetPath + " " + listOfPackages + ";");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_commandExecuting = ectn_INSTALL;
  m_unixCommand->executeCommand(command);
}

/*
 * Calls xbps to install given LOCAL packages inside a terminal
 */
void XBPSExec::doInstallLocalInTerminal(const QString &targetPath, const QString &listOfPackages)
{
  //QString command = "xbps-install -S -y --repository " + targetPath + " " + listOfPackages;
  QString command = UnixCommand::getXBPSInstallBin() + " -S -y --repository " + targetPath + " " + listOfPackages;

  m_lastCommandList.clear();
  //m_lastCommandList.append("xbps-install -S --repository " + targetPath + " " + listOfPackages + ";");
  m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " -S --repository " + targetPath + " " + listOfPackages + ";");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_unixCommand->runCommandInTerminal(m_lastCommandList);
}

/*
 * Calls xbps to remove given packages and returns output to UI
 */
void XBPSExec::doRemove(const QString &listOfPackages)
{
  //QString command = "/usr/bin/xbps-remove -R -y " + listOfPackages;
  QString command = UnixCommand::getXBPSRemoveBin() + " -R -y " + listOfPackages;

  m_lastCommandList.clear();
  //m_lastCommandList.append("/usr/bin/xbps-remove -R " + listOfPackages + ";");
  m_lastCommandList.append(UnixCommand::getXBPSRemoveBin() + " -R " + listOfPackages + ";");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_commandExecuting = ectn_REMOVE;
  m_unixCommand->executeCommand(command);
}

/*
 * Calls xbps to remove given packages inside a terminal
 */
void XBPSExec::doRemoveInTerminal(const QString &listOfPackages)
{
  m_lastCommandList.clear();
  //m_lastCommandList.append("/usr/bin/xbps-remove -R " + listOfPackages + ";");
  m_lastCommandList.append(UnixCommand::getXBPSRemoveBin() + " -R " + listOfPackages + ";");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_unixCommand->runCommandInTerminal(m_lastCommandList);
}

/*
 * Calls xbps to remove and install given packages and returns output to UI
 */
void XBPSExec::doRemoveAndInstall(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall)
{
  QStringList params;
  params << UnixCommand::getShell();
  params << "-c";
  params << UnixCommand::getXBPSRemoveBin() + " -R -y " + listOfPackagestoRemove + "; " +
            UnixCommand::getXBPSInstallBin() + " -y " + listOfPackagestoInstall;

  m_lastCommandList.clear();
  //m_lastCommandList.append("/usr/bin/xbps-remove -R " + listOfPackagestoRemove + ";");
  m_lastCommandList.append(UnixCommand::getXBPSRemoveBin() + " -R " + listOfPackagestoRemove + ";");
  //m_lastCommandList.append("/usr/bin/xbps-install " + listOfPackagestoInstall + ";");
  m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " " + listOfPackagestoInstall + ";");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_commandExecuting = ectn_REMOVE_INSTALL;
  m_unixCommand->executeCommand(params);
}

/*
 * Calls xbps to remove and install given packages inside a terminal
 */
void XBPSExec::doRemoveAndInstallInTerminal(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall)
{
  m_lastCommandList.clear();
  //m_lastCommandList.append("/usr/bin/xbps-remove -R " + listOfPackagestoRemove + ";");
  m_lastCommandList.append(UnixCommand::getXBPSRemoveBin() + " -R " + listOfPackagestoRemove + ";");

  //m_lastCommandList.append("/usr/bin/xbps-install " + listOfPackagestoInstall + ";");
  m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " " + listOfPackagestoInstall + ";");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_unixCommand->runCommandInTerminal(m_lastCommandList);
}

/*
 * Calls xbps to upgrade the entire system and returns output to UI
 *
 * param upgradeXBPS = true upgrades XBPS pkg manager first!
 */
void XBPSExec::doSystemUpgrade(bool upgradeXBPS)
{
  //QString command="/usr/bin/xbps-install -u -y";
  QString command= UnixCommand::getXBPSInstallBin() + " -u -y";
  //if (upgradeXBPS) command = "/usr/bin/xbps-install -u -y xbps";
  if (upgradeXBPS) command = UnixCommand::getXBPSInstallBin() + " -u -y xbps";

  m_lastCommandList.clear();
  if(upgradeXBPS)
    //m_lastCommandList.append("/usr/bin/xbps-install -u xbps;");
    m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " -u xbps;");
  else
    //m_lastCommandList.append("/usr/bin/xbps-install -u;");
    m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " -u;");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_commandExecuting = ectn_SYSTEM_UPGRADE;
  m_unixCommand->executeCommand(command);
}

/*
 * Calls xbps to upgrade the entire system inside a terminal
 */
void XBPSExec::doSystemUpgradeInTerminal(bool upgradeXBPS)
{
  m_lastCommandList.clear();

  if(upgradeXBPS)
    //m_lastCommandList.append("/usr/bin/xbps-install -u xbps;");
    m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " -u xbps;");
  else
    //m_lastCommandList.append("/usr/bin/xbps-install -u;");
    m_lastCommandList.append(UnixCommand::getXBPSInstallBin() + " -u;");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n 1 -p \"" + StrConstants::getPressAnyKey() + "\"");
  m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
  m_unixCommand->runCommandInTerminal(m_lastCommandList);
}

/*
 * Calls xbps to sync databases and returns output to UI
 */
void XBPSExec::doSyncDatabase()
{
  //QString command = "/usr/bin/xbps-install -Syy";
  QString command = UnixCommand::getXBPSInstallBin() + " -Syy";

  m_commandExecuting = ectn_SYNC_DATABASE;
  m_unixCommand->executeCommand(command);
}

/*
 * Runs latest command inside a terminal (probably due to some previous error)
 */
void XBPSExec::runLastestCommandInTerminal()
{
  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_unixCommand->runCommandInTerminal(m_lastCommandList);
}
