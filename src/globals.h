/*
* This file is part of OctoPkg, an open-source GUI for pkgng.
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

#ifndef MAINWINDOW_GLOBALS_H
#define MAINWINDOW_GLOBALS_H

#include "strconstants.h"
#include "model/packagemodel.h"

#include <QStandardItem>
#include <QFutureWatcher>

struct AUROutdatedPackages
{
  public:
    QHash<QString, QString> content;
};

typedef std::pair<QString, QStringList*> GroupMemberPair;

extern QFutureWatcher<QString> g_fwToolTip;
extern QFutureWatcher<QString> g_fwToolTipInfo;
extern QFutureWatcher<QList<PackageListData> *> g_fwPacman;
extern QFutureWatcher<QList<PackageListData> *> g_fwForeignPacman;
extern QFutureWatcher<QSet<QString> *> g_fwUnrequiredPacman;
extern QFutureWatcher<GroupMemberPair>          g_fwPacmanGroup;
extern QFutureWatcher<QList<PackageListData> *> g_fwRemote;
extern QFutureWatcher<QList<PackageListData> *> g_fwRemoteMeta;
//extern QFutureWatcher<QList<PackageListData> *> g_fwMarkForeignPackages;
extern QFutureWatcher<QMap<QString, OutdatedPackageInfo> *> g_fwOutdatedList;
extern QFutureWatcher<QString> g_fwDistroNews;
extern QFutureWatcher<QString> g_fwPackageOwnsFile;
extern QFutureWatcher<TransactionInfo> g_fwTargetUpgradeList;

QString showPackageInfo(QString pkgName);
TransactionInfo getTargetUpgradeList(const QString &pkgName);
QList<PackageListData> * searchPkgPackages();
QSet<QString> * searchUnrequiredPacmanPackages();
QList<PackageListData> * searchForeignPackages();
QList<PackageListData> * searchRemotePackages(QString searchString);
QString searchPacmanPackagesByFile(const QString &file);
GroupMemberPair          searchPacmanPackagesFromGroup(QString groupName);
QMap<QString, OutdatedPackageInfo> * getOutdatedList();
QString getLatestDistroNews();

#endif // MAINWINDOW_GLOBALS_H
