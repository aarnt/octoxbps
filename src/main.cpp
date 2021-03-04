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

#include "mainwindow.h"
#include "argumentlist.h"
#include "strconstants.h"
#include "unixcommand.h"
#include "wmhelper.h"
#include <iostream>

#include "QtSolutions/qtsingleapplication.h"
#include <QMessageBox>
#include <QTranslator>
#include <QResource>

int main(int argc, char *argv[])
{
  ArgumentList *argList = new ArgumentList(argc, argv);
  QString packagesToInstall;
  QtSingleApplication app( StrConstants::getApplicationName(), argc, argv );

  if (app.isRunning())
  {    
    if (argList->getSwitch(QStringLiteral("-sysupgrade")))
    {
      app.sendMessage(QStringLiteral("SYSUPGRADE"));
    }
    else if (argList->getSwitch(QStringLiteral("-close")))
    {
      app.sendMessage(QStringLiteral("CLOSE"));
    }
    else if (argList->getSwitch(QStringLiteral("-hide")))
    {
      app.sendMessage(QStringLiteral("HIDE"));
    }
    else
      app.sendMessage(QStringLiteral("RAISE"));

    return 0;
  }

  if(!QFile::exists(ctn_OCTOXBPS_SUDO))
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorBinaryXNotFound(ctn_OCTOXBPS_SUDO));
    return 1;
  }
  if(!QFile::exists(QStringLiteral("/usr/bin/xbps-query")))
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorBinaryXNotFound(QStringLiteral("xbps-query")));
    return 1;
  }
  if(!QFile::exists(QStringLiteral("/usr/bin/curl")))
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorBinaryXNotFound(QStringLiteral("curl")));
    return 1;
  }
  if(!QFile::exists(QStringLiteral("/usr/bin/sh")))
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorBinaryXNotFound(QStringLiteral("sh")));
    return 1;
  }

  //This sends a message just to enable the socket-based QtSingleApplication engine
  app.sendMessage("RAISE");

  QTranslator appTranslator;
  appTranslator.load(QLocale(), QStringLiteral("octoxbps"), QStringLiteral("_"), QStringLiteral(":/translations"));
  app.installTranslator(&appTranslator);

  if (argList->getSwitch(QStringLiteral("-help"))){
    std::cout << StrConstants::getApplicationCliHelp().toLatin1().data() << std::endl;
    return(0);
  }
  else if (argList->getSwitch(QStringLiteral("-version"))){
    std::cout << "\n" << StrConstants::getApplicationName().toLatin1().data() <<
                 " " << StrConstants::getApplicationVersion().toLatin1().data() << "\n" << std::endl;
    return(0);
  }

  if (UnixCommand::isRootRunning())
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorRunningWithRoot());
    return ( -2 );
  }

  setenv("COLORTERM", "truecolor", 1);
  setenv("TERM", "xterm-256color", 1);

  MainWindow w;
  app.setActivationWindow(&w);
  app.setQuitOnLastWindowClosed(false);

  if (argList->getSwitch(QStringLiteral("-sysupgrade")))
  {
    w.setCallSystemUpgrade();
  }

  if (argList->getSwitch("-d"))
  {
    //If user chooses to switch debug info on...
    w.turnDebugInfoOn();
  }

  if (!packagesToInstall.isEmpty())
  {
    QStringList packagesToInstallList =
        packagesToInstall.split(",", Qt::SkipEmptyParts);

    w.setPackagesToInstallList(packagesToInstallList);
  }

  w.show();

  QResource::registerResource(QStringLiteral("./resources.qrc"));

  return app.exec();
}
