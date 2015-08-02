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
    app.sendMessage("RAISE");
    return 0;
  }

  if(!UnixCommand::hasTheExecutable("xbps-query"))
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorBinaryXNotFound("xbps-query"));
  }
  if(!UnixCommand::hasTheExecutable("curl"))
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorBinaryXNotFound("curl"));
  }
  if(!UnixCommand::hasTheExecutable("sh"))
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorBinaryXNotFound("sh"));
  }

  //This sends a message just to enable the socket-based QtSingleApplication engine
  app.sendMessage("RAISE");

  QTranslator appTranslator;
  appTranslator.load(":/resources/translations/octoxbps_" +
                     QLocale::system().name());
  app.installTranslator(&appTranslator);

  if (argList->getSwitch("-help")){
    std::cout << StrConstants::getApplicationCliHelp().toLatin1().data() << std::endl;
    return(0);
  }
  else if (argList->getSwitch("-version")){
    std::cout << "\n" << StrConstants::getApplicationName().toLatin1().data() <<
                 " " << StrConstants::getApplicationVersion().toLatin1().data() << "\n" << std::endl;
    return(0);
  }

  if (UnixCommand::isRootRunning() && !WMHelper::isKDERunning()){
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorRunningWithRoot());
    return ( -2 );
  }

  MainWindow w;
  app.setActivationWindow(&w);
  app.setQuitOnLastWindowClosed(false);

  if (argList->getSwitch("-sysupgrade-noconfirm"))
  {
    w.setCallSystemUpgradeNoConfirm();
  }
  else if (argList->getSwitch("-sysupgrade"))
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
        packagesToInstall.split(",", QString::SkipEmptyParts);

    w.setPackagesToInstallList(packagesToInstallList);
  }

  w.show();

  QResource::registerResource("./resources.qrc");

  return app.exec();
}
