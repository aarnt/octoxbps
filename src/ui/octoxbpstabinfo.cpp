/*
* This file is part of OctoXBPS, an open-source GUI for XBPS.
* Copyright (C) 2014 Thomas Binkau
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

#include "octoxbpstabinfo.h"
#include "src/strconstants.h"
#include "src/package.h"

#include <QDebug>
#include <QHash>
#include <QRegularExpression>
#include <QLocale>

/*
 * The OctoXBPSTabInfo class provides functionality for the Tab "Info"
 */

/**
 * @brief OctopiTabInfo::anchorBegin for navigation
 */
const QString OctoXBPSTabInfo::anchorBegin("anchorBegin");

OctoXBPSTabInfo::OctoXBPSTabInfo()
{
}

/**
 * This function has been extracted from src/mainwindow_refresh.cpp void MainWindow::refreshTabInfo(QString pkgName)
 */
QString OctoXBPSTabInfo::formatTabInfo(const PackageRepository::PackageData& package,
                                     const QMap<QString, OutdatedPackageInfo>& outdatedPkgList)
{
  PackageInfoData pid = Package::getInformation(package.name);

  QString version = StrConstants::getVersion();
  QString url = StrConstants::getURL();
  QString licenses = StrConstants::getLicenses();
  QString downloadSize = StrConstants::getDownloadSize();
  QString installedSize = StrConstants::getInstalledSize();
  QString maintainer = StrConstants::getMaintainer();
  QString architecture = StrConstants::getArchitecture();
  QString buildDate = StrConstants::getBuildDate();
  QString installDate = StrConstants::getInstallDate();
  QString options = StrConstants::getOptions();
  QString dependencies = StrConstants::getDependencies();
  //Let's put package description in UTF-8 format  
  QString pkgDescription;

  if (!pid.comment.isEmpty())
    pkgDescription = pid.comment;
  else
  {
    int ind = package.comment.indexOf(" ");
    pkgDescription = package.comment.right(package.comment.size() - ind).trimmed();
  }

  QString html;
  html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
  html += "<a id=\"" + anchorBegin + "\"></a>";
  html += "<h2>" + package.name + "</h2>";
  html += pkgDescription;
  html += "<table border=\"0\">";
  html += "<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>";

  if (package.repository.isEmpty() && pid.url != "<a href=\"http://\"></a>UNKNOWN")
    html += "<tr><td>" + url + "</td><td>" + pid.url + "</td></tr>";
  else if (!package.repository.isEmpty())
    html += "<tr><td>" + url + "</td><td>" + Package::makeURLClickable(package.www) + "</td></tr>";

  if (package.outdated())
  {
    if (package.status != ectn_NEWER)
    {
      const OutdatedPackageInfo opi = outdatedPkgList.value(package.name);
      html += "<tr><td>" + version + "</td><td><b><font color=\"#E55451\">" + opi.oldVersion + "</font></b>  <b>" +
          StrConstants::getNewVersionAvailable().arg(opi.newVersion) + "</b></td></tr>";
    }
    /*else
    {
      QString newerVersion = package.outdatedVersion;
      html += "<tr><td>" + version + "</td><td>" + package.version + " <b><font color=\"#FF8040\">"
          + StrConstants::getNewerInstalledVersion().arg(newerVersion) +
          "</b></font></td></tr>";
    }*/
  }
  else
  {
    html += "<tr><td>" + version + "</td><td>" + package.version + "</td></tr>";
  }

  if (!pid.arch.isEmpty())
    html += "<tr><td>" + architecture + "</td><td>" + pid.arch + "</td></tr>";

  //This is needed as packager names could be encoded in different charsets, resulting in an error
  QString packagerName;
  packagerName = pid.maintainer;
  packagerName = packagerName.replace("<", "&lt;");
  packagerName = packagerName.replace(">", "&gt;");

  if(! pid.license.isEmpty())
    html += "<tr><td>" + licenses + "</td><td>" + pid.license + "</td></tr>";

  QString dateTimeFormat = QLocale().dateTimeFormat();
  QDateTime auxDate;
  auxDate.setTimeSpec(Qt::UTC);
  QString auxBuild = pid.buildDate;
  int lastSpace = auxBuild.lastIndexOf(" ");
  auxBuild = auxBuild.remove(lastSpace, auxBuild.length()-(lastSpace));
  auxDate = QDateTime::fromString(auxBuild, Qt::ISODate);

  if(! pid.buildDate.isEmpty())
    html += "<tr><td>" + buildDate + "</td><td>" + auxDate.toString(dateTimeFormat);

  if (package.installed())
  {
    QDateTime inst;
    inst.setTimeSpec(Qt::UTC);
    QString auxInst = pid.installDate;
    int lastSpace = auxInst.lastIndexOf(" ");
    auxInst = auxInst.remove(lastSpace, auxInst.length()-(lastSpace));
    inst = QDateTime::fromString(auxInst, Qt::ISODate);
    html += "<tr><td>" + installDate + "</td><td>" + inst.toString(dateTimeFormat);
  }

  if(!pid.downloadSizeAsString.isEmpty())
    html += "<tr><td>" + downloadSize + "</td><td>" + (pid.downloadSizeAsString) + "</td></tr>";

  if(!pid.installedSizeAsString.isEmpty())
    html += "<tr><td>" + installedSize + "</td><td>" + pid.installedSizeAsString + "</td></tr>";

  if (!packagerName.isEmpty())
    html += "<tr><td>" + maintainer + "</td><td>" + packagerName + "</td></tr>";

  QString dependenciesList = Package::getDependencies(package.name);
  if ( !dependenciesList.isEmpty())
  {
    html += "<br><tr><td>" + dependencies + "</td><td>" + dependenciesList + "</td></tr>";
    if (! pid.options.isEmpty()) html += "<br>";
  }

  if(! pid.options.isEmpty())
  {
    if (dependenciesList.isEmpty()) html += "<br>";

    html += "<tr><td>" + options + "</td><td>" + pid.options + "</td></tr>";
  }

  html += "</table><br>";

  return html;
}
