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

/*
 * This is MainWindow's refresh UI components related code
 */

#include "mainwindow.h"
#include "searchlineedit.h"
#include "ui_mainwindow.h"
#include "strconstants.h"
#include "uihelper.h"
#include "globals.h"
#include <iostream>
#include <cassert>
#include "src/ui/octopitabinfo.h"

#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QTextBrowser>
#include <QStandardItem>
#include <QFutureWatcher>
#include <QMutableListIterator>
#include <QtConcurrent/QtConcurrentRun>

/*
 * If we have some outdated packages, let's put an angry red face icon in this app!
 */
void MainWindow::refreshAppIcon()
{
  bool enableSystemUpgrade=false;

  if ((m_outdatedStringList->count() > 0))
  {
    setWindowIcon(IconHelper::getIconOctopiRed());

    if(m_commandExecuting != ectn_MIRROR_CHECK && !isThereAPendingTransaction())
      enableSystemUpgrade=true;
  }
  else
  {
    setWindowIcon(IconHelper::getIconOctopiGreen());
  }

  ui->actionSystemUpgrade->setEnabled(enableSystemUpgrade);
}

/*
 * Reconfigure Tools menu according available tools in the system
 */
/*void MainWindow::refreshMenuTools()
{
  static bool connectorPlv=false;
  int availableTools=0;

  if(UnixCommand::hasTheExecutable("plv"))
  {
    availableTools++;
    ui->menuTools->menuAction()->setVisible(true);
    ui->actionPacmanLogViewer->setVisible(true);
    ui->actionPacmanLogViewer->setIcon(QIcon::fromTheme("plv"));

    if (!connectorPlv)
    {
      connect(ui->actionPacmanLogViewer, SIGNAL(triggered()), this, SLOT(launchPLV()));
      connectorPlv=true;
    }
  }
  else
    ui->actionPacmanLogViewer->setVisible(false);

  foreach (QAction * act,  ui->menuBar->actions())
  {
    QString text = act->text();
    text = text.remove("&");
    act->setText(qApp->translate("MainWindow", text.toUtf8(), 0));
  }
}*/

/*
 * Inserts the group names into the Groups treeView
 */
void MainWindow::refreshGroupsWidget()
{
  /*
  disconnect(ui->twGroups, SIGNAL(itemSelectionChanged()), this, SLOT(groupItemSelected()));

  QList<QTreeWidgetItem *> items;
  ui->twGroups->clear();

  items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList("<" + StrConstants::getDisplayAllCategories() + ">")));
  m_AllGroupsItem = items.at(0);
  const QStringList*const packageGroups = Package::getPackageGroups();
  foreach(QString group, *packageGroups)
  {
    items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(group)));
  }
  m_packageRepo.checkAndSetGroups(*packageGroups); // update Package Repository as well
  delete packageGroups;

  ui->twGroups->insertTopLevelItems(0, items);
  ui->twGroups->setCurrentItem(items.at(0));
  connect(ui->twGroups, SIGNAL(itemSelectionChanged()), this, SLOT(groupItemSelected()));
  */
}

/*
 * User clicked AUR tool button in the toolbar
 */
void MainWindow::remoteSearchClicked()
{
  if (m_commandExecuting != ectn_NONE && m_commandExecuting != ectn_LOCAL_PKG_REFRESH) return;

  static bool lastPkgButtonClickedWasRemote = false;

  if (lastPkgButtonClickedWasRemote && m_actionSwitchToRemoteSearch->isChecked())
  {
    disconnect(m_actionGroupSearch, SIGNAL(triggered(QAction*)), this, SLOT(remoteSearchClicked()));
    m_actionSwitchToRemoteSearch->setChecked(false);
    m_actionSwitchToLocalFilter->setChecked(true);
    connect(m_actionGroupSearch, SIGNAL(triggered(QAction*)), this, SLOT(remoteSearchClicked()));
  }
  else if (!lastPkgButtonClickedWasRemote && m_actionSwitchToLocalFilter->isChecked())
  {
    disconnect(m_actionGroupSearch, SIGNAL(triggered(QAction*)), this, SLOT(remoteSearchClicked()));
    m_actionSwitchToLocalFilter->setChecked(false);
    m_actionSwitchToRemoteSearch->setChecked(true);
    connect(m_actionGroupSearch, SIGNAL(triggered(QAction*)), this, SLOT(remoteSearchClicked()));
  }

  m_leFilterPackage->clear();

  if (m_actionSwitchToRemoteSearch->isChecked())
  {
    m_leFilterPackage->setPlaceholderText(StrConstants::getLineEditTextRemote());
    m_leFilterPackage->setToolTip(StrConstants::getRemotePackageSearchTip());
    ui->menuSearch->setEnabled(false);
  }
  else
  {
    m_leFilterPackage->setPlaceholderText(StrConstants::getLineEditTextLocal());
    m_leFilterPackage->setToolTip("");
    ui->menuSearch->setEnabled(true);
  }

  m_selectedRepository = "";

  if (m_commandExecuting == ectn_NONE)
    m_refreshPackageLists = false;
  else if (m_commandExecuting == ectn_LOCAL_PKG_REFRESH)
    m_refreshPackageLists = true;

  metaBuildPackageList();

  if (m_commandExecuting == ectn_LOCAL_PKG_REFRESH)
  {
    connect(this, SIGNAL(buildPackageListDone()), this, SLOT(resetTransaction()));
  }

  if (m_actionSwitchToRemoteSearch->isChecked())
  {
    lastPkgButtonClickedWasRemote = true;
  }
  else
  {
    lastPkgButtonClickedWasRemote = false;
  }
}

/*
 * Whenever user selects another package group...
 */
void MainWindow::groupItemSelected()
{
  //Let us select ALL pkgs from ALL repos!
  switchToViewAllPackages();
  m_selectedRepository = "";
  m_actionRepositoryAll->setChecked(true);

  if (isAllCategoriesSelected())
  {
    m_refreshPackageLists = false;
  }
  else
  {
    m_refreshPackageLists = true;
  }

  m_leFilterPackage->clear();
  metaBuildPackageList();
}

/*
 * Given the package name that owns the file user has just searched, goes to that package in the list
 */
void MainWindow::positionInPkgListSearchByFile()
{
  if (m_cic) {
    delete m_cic;
    m_cic = NULL;
  }

  QString pkgName = g_fwPackageOwnsFile.result();

  if (!pkgName.isEmpty())
  {
    QModelIndex searchColumn = m_packageModel->index(0,
                                                     PackageModel::ctn_PACKAGE_NAME_COLUMN,
                                                     QModelIndex());
    QModelIndexList fi = m_packageModel->match(searchColumn, Qt::DisplayRole, pkgName, -1, Qt::MatchExactly);

    if (fi.count() >= 1)
    {
      ui->tvPackages->setCurrentIndex(fi.at(0));
      ui->tvPackages->scrollTo(fi.at(0), QAbstractItemView::PositionAtCenter);
    }
  }
  else //The pkg was not found, so we position on the first item of the list!
  {
    ui->tvPackages->setCurrentIndex(m_packageModel->index(0,0,QModelIndex()));
  }
}

/*
 * Populates the list of available packages from the given groupName
 */
void MainWindow::buildPackagesFromGroupList(const QString group)
{
  CPUIntensiveComputing cic;
  const QList<QString>*const list = m_listOfPackagesFromGroup.get();
  QList<QString>::const_iterator it = list->begin();

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);
  m_progressWidget->show();

  int installedCount = 0;
  int counter=0;

  while(it != list->end())
  {
    if (isPackageInstalled(*it)) {
      ++installedCount;
    }

    counter++;
    m_progressWidget->setValue(counter);
    ++it;
  }

  counter = list->count();
  m_progressWidget->setValue(counter);
  m_progressWidget->close();

  m_packageRepo.checkAndSetMembersOfGroup(group, *list);
  m_packageModel->applyFilter(m_selectedViewOption, m_selectedRepository, isAllCategories(group) ? "" : group);

  //Refresh counters
  m_numberOfInstalledPackages = installedCount;
  //Refresh statusbar widget
  refreshStatusBar();

  reapplyPackageFilter();

  QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->setCurrentIndex(maux);

  m_listOfPackagesFromGroup.reset();
  refreshTabInfo();
  refreshTabFiles();
  ui->tvPackages->setFocus();

  refreshToolBar();
  refreshStatusBarToolButtons();

  tvPackagesSelectionChanged(QItemSelection(),QItemSelection());
}

/*
 * Executes QFuture to retrive Outdated AUR list of packages
 */
/*void MainWindow::retrieveForeignPackageList()
{
  QEventLoop el;
  QFuture<QList<PackageListData> *> f;
  f = QtConcurrent::run(searchForeignPackages);
  connect(&g_fwForeignPacman, SIGNAL(finished()), this, SLOT(preBuildForeignPackageList()));
  connect(&g_fwForeignPacman, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwForeignPacman.setFuture(f);
  el.exec();

  assert(m_foreignPackageList != NULL);
}*/

/*
 * Executes QFuture to retrive Unrequired Pacman list of packages
 */
void MainWindow::retrieveUnrequiredPackageList()
{
  QEventLoop el;
  QFuture<QSet<QString> *> f;
  f = QtConcurrent::run(searchUnrequiredPacmanPackages);
  connect(&g_fwUnrequiredPacman, SIGNAL(finished()), this, SLOT(preBuildUnrequiredPackageList()));
  connect(&g_fwUnrequiredPacman, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwUnrequiredPacman.setFuture(f);
  el.exec();
  assert(m_unrequiredPackageList != NULL);
}

/*
 * Helper method to assign QFuture for list of outdated packages
 */
/*void MainWindow::preBuildForeignPackageList()
{
  m_foreignPackageList = g_fwForeignPacman.result();

  if(m_debugInfo)
    std::cout << "Time elapsed obtaining Foreign pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;
}*/

/*
 * Helper method to assign QFuture for Unrequired Pacman list of packages
 */
void MainWindow::preBuildUnrequiredPackageList()
{
  m_unrequiredPackageList = g_fwUnrequiredPacman.result();

  if(m_debugInfo)
    std::cout << "Time elapsed obtaining Unrequired pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * Pacman package list building method
 */
void MainWindow::preBuildPackageList()
{
  m_listOfPackages.reset(g_fwPacman.result());

  if(m_debugInfo)
    std::cout << "Time elapsed obtaining pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;

  buildPackageList();
  toggleSystemActions(true);

  if (!m_initializationCompleted)
  {
    remoteSearchClicked();
    m_initializationCompleted = true;
  }
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * Pacman packages from group list building method
 */
void MainWindow::preBuildPackagesFromGroupList()
{
  GroupMemberPair result = g_fwPacmanGroup.result();
  m_listOfPackagesFromGroup.reset(result.second);
  buildPackagesFromGroupList(result.first);

  toggleSystemActions(true);

  emit buildPackagesFromGroupListDone();
}

/*
 * Slot to fire the search for all installed packages
 */
void MainWindow::searchForPkgPackages()
{
  QFuture<QList<PackageListData> *> f;
  f = QtConcurrent::run(searchPkgPackages);
  disconnect(&g_fwPacman, SIGNAL(finished()), this, SLOT(preBuildPackageList()));
  connect(&g_fwPacman, SIGNAL(finished()), this, SLOT(preBuildPackageList()));
  g_fwPacman.setFuture(f);
}

/*
 * Decides which SLOT to call: buildPackageList, buildAURPackageList or buildPackagesFromGroupList
 */
void MainWindow::metaBuildPackageList()
{
  m_time->start();

  if (isSearchByFileSelected())
    m_leFilterPackage->setRefreshValidator(ectn_FILE_VALIDATOR);
  else if (isRemoteSearchSelected())
    m_leFilterPackage->setRefreshValidator(ectn_AUR_VALIDATOR);
  else
    m_leFilterPackage->setRefreshValidator(ectn_DEFAULT_VALIDATOR);

  ui->tvPackages->setSelectionMode(QAbstractItemView::ExtendedSelection);

  //if (ui->twGroups->topLevelItemCount() == 0 || isAllCategoriesSelected())
  if (m_actionSwitchToRemoteSearch->isChecked())
  {
    ui->actionSearchByFile->setEnabled(false);
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    clearStatusBar();

    m_cic = new CPUIntensiveComputing();

    if(!m_leFilterPackage->text().isEmpty())
    {
      m_toolButtonPacman->hide();
      disconnect(&g_fwRemoteMeta, SIGNAL(finished()), this, SLOT(preBuildRemotePackageListMeta()));

      QFuture<QList<PackageListData> *> f;
      f = QtConcurrent::run(searchRemotePackages, m_leFilterPackage->text());
      connect(&g_fwRemoteMeta, SIGNAL(finished()), this, SLOT(preBuildRemotePackageListMeta()));
      g_fwRemoteMeta.setFuture(f);
    }
    else
    {
      m_listOfRemotePackages = new QList<PackageListData>();
      buildRemotePackageList();
      delete m_cic;
      m_cic = 0;
      m_leFilterPackage->setFocus();
    }
  }
  else
  {
    ui->actionSearchByFile->setEnabled(true);
    ui->actionSearchByName->setChecked(true);

    //toggleSystemActions(false);
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    //reapplyPackageFilter();
    disconnect(&g_fwPacman, SIGNAL(finished()), this, SLOT(preBuildPackageList()));

    if (m_refreshPackageLists)
    {
      QFuture<QMap<QString, OutdatedPackageInfo> *> f;
      f = QtConcurrent::run(getOutdatedList);
      disconnect(&g_fwOutdatedList, SIGNAL(finished()), this, SLOT(searchForPkgPackages()));
      connect(&g_fwOutdatedList, SIGNAL(finished()), this, SLOT(searchForPkgPackages()));
      g_fwOutdatedList.setFuture(f);
    }
    else
    {
      searchForPkgPackages();
    }

    if(m_debugInfo)
      std::cout << m_packageModel->getPackageCount() << " pkgs => " <<
                 "Time elapsed building pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;
  }
  /*
  else //pkg category clicked!
  {
    ui->actionSearchByFile->setEnabled(false);
    toggleSystemActions(false);
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    reapplyPackageFilter();
    disconnect(&g_fwPacmanGroup, SIGNAL(finished()), this, SLOT(preBuildPackagesFromGroupList()));

    QEventLoop el;
    QFuture<GroupMemberPair> f;
    f = QtConcurrent::run(searchPacmanPackagesFromGroup, getSelectedCategory());
    connect(&g_fwPacmanGroup, SIGNAL(finished()), this, SLOT(preBuildPackagesFromGroupList()));
    disconnect(this, SIGNAL(buildPackagesFromGroupListDone()), &el, SLOT(quit()));
    connect(this, SIGNAL(buildPackagesFromGroupListDone()), &el, SLOT(quit()));

    g_fwPacmanGroup.setFuture(f);
    el.exec();

    if(m_debugInfo)
      std::cout << m_packageModel->getPackageCount() << " pkgs => " <<
                 "Time elapsed building pkgs from '" << getSelectedCategory().toLatin1().data() << " group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;
  }
  */
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * AUR package list building method
 */
void MainWindow::preBuildRemotePackageListMeta()
{
  m_listOfRemotePackages = g_fwRemoteMeta.result();

  //Let's iterate over those found packages to change their status
  for (QList<PackageListData>::iterator it = m_listOfRemotePackages->begin();
       it != m_listOfRemotePackages->end(); ++it)
  {
    const PackageRepository::PackageData*const package =
        m_packageRepo.getFirstPackageByName((*it).name);

    if (package != NULL)
    {
      (*it).status = package->status;
    }
  }

  buildRemotePackageList();

  if (m_cic) {
    delete m_cic;
    m_cic = 0;
  }

  if (m_packageModel->getPackageCount() == 0)
  {
    m_leFilterPackage->setFocus();
  }
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * AUR package list building method
 */
void MainWindow::preBuildRemotePackageList()
{
  m_listOfRemotePackages = g_fwRemote.result();

  if (m_commandExecuting == ectn_NONE)
  {
    //Let's iterate over those found packages to change their status
    for (QList<PackageListData>::iterator it = m_listOfRemotePackages->begin();
         it != m_listOfRemotePackages->end(); ++it)
    {
      const PackageRepository::PackageData*const package =
          m_packageRepo.getFirstPackageByName((*it).name);

      if (package != NULL)
      {
        (*it).status = package->status;
      }
    }
  }

  buildRemotePackageList();

  if (m_cic) {
    delete m_cic;
    m_cic = NULL;
  }

  if (m_packageModel->getPackageCount() == 0)
  {
    m_leFilterPackage->setFocus();
  }

  emit buildRemotePackageListDone();
}

/*
 * Populates the list of found AUR packages (installed [+ non-installed])
 * given the searchString parameter passed.
 *
 */
void MainWindow::buildRemotePackageList()
{
  //ui->actionSearchByDescription->setChecked(true);
  m_progressWidget->show();

  const QSet<QString>*const unrequiredPackageList = Package::getUnrequiredPackageList();
  QList<PackageListData> *list = m_listOfRemotePackages;

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);
  int counter=0;
  int installedCount = 0;
  QList<PackageListData>::const_iterator it = list->begin();

  while(it != list->end())
  {
    if (isPackageInstalled(it->name)) {
      ++installedCount;
    }
    counter++;
    m_progressWidget->setValue(counter);
    ++it;
  }

  m_packageRepo.setAURData(list, *unrequiredPackageList);
  m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN);
  m_packageModel->applyFilter(ectn_ALL_PKGS, "", "NONE");

  if (list->count() > 0)
  {
    QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
    ui->tvPackages->setCurrentIndex(maux);
    ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
    ui->tvPackages->setCurrentIndex(maux);

    refreshTabInfo();
    refreshTabFiles();

    if (isPackageTreeViewVisible())
    {
      ui->tvPackages->setFocus();
    }
  }

  list->clear();

  //Refresh counters
  m_numberOfInstalledPackages = installedCount;

  if (isRemoteSearchSelected())
  {
    m_leFilterPackage->initStyleSheet();
    QString search = Package::parseSearchString(m_leFilterPackage->text());
    m_packageModel->applyFilter(search);

    ui->tvPackages->selectionModel()->clear();
    QModelIndex mi = m_packageModel->index(0, PackageModel::ctn_PACKAGE_NAME_COLUMN, QModelIndex());
    ui->tvPackages->setCurrentIndex(mi);
    ui->tvPackages->scrollTo(mi);
    invalidateTabs();
  }
  else
  {
    reapplyPackageFilter();
  }

  //Refresh statusbar widget
  refreshStatusBar();

  //Refresh application icon
  refreshAppIcon();

  counter = list->count();
  m_progressWidget->setValue(counter);
  m_progressWidget->close();

  //ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_REPOSITORY_COLUMN, true);

  refreshToolBar();
  refreshStatusBarToolButtons();

  //If we found no packages, let's make another search, this time 'by name'...
  if (!m_leFilterPackage->text().isEmpty() &&
      (!m_leFilterPackage->text().contains(QRegularExpression("\\s"))) &&
      m_packageModel->getPackageCount() == 0 &&
      ui->actionSearchByDescription->isChecked())
  {
    if (m_cic != NULL)
    {
      delete m_cic;
      m_cic = 0;
    }

    ui->actionSearchByName->setChecked(true);
    metaBuildPackageList();
  }

  emit buildPackageListDone();
}

/*
 * Outputs a list of packages that don't have a description
 */
void MainWindow::showPackagesWithNoDescription()
{
  bool printHeader = false;
  QList<PackageListData> *list = Package::getPackageList();
  QList<PackageListData>::const_iterator it = list->begin();

  while(it != list->end())
  {
    PackageListData pld = *it;

    if (pld.description == (pld.name + "  "))
    {
      if (!printHeader)
      {
        std::cout << std::endl << "List of packages without description:" << std::endl << std::endl;
        printHeader=true;
      }

      std::cout << pld.name.toLatin1().data() << std::endl;
    }

    ++it;
  }
}

/*
 * Populates the list of available packages (installed [+ non-installed])
 *
 * It's called Only: when the selected group is <All> !
 */
void MainWindow::buildPackageList()
{
  CPUIntensiveComputing cic;
  static bool firstTime = true;

  if(m_refreshPackageLists) //If it's not the starting of the app...
  {
    //Let's get outdatedPackages list again!
    m_outdatedStringList->clear();
    m_outdatedList = g_fwOutdatedList.result(); //Package::getOutdatedStringList();

    if(m_debugInfo)
      std::cout << "Time elapsed retrieving outdated pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;

    foreach(QString k, m_outdatedList->keys())
    {
      m_outdatedStringList->append(k);
    }

    if(m_debugInfo)
      std::cout << "Time elapsed refreshing outdated pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;

    m_numberOfOutdatedPackages = m_outdatedStringList->count();

    delete m_unrequiredPackageList;
    m_unrequiredPackageList = NULL;

    m_unrequiredPackageList = Package::getUnrequiredPackageList();   

    if(m_debugInfo)
      std::cout << "Time elapsed obtaining unrequired pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;
  }

  // Fetch package list
  QList<PackageListData> *list;
  list = m_listOfPackages.release();

  //qDebug() << "PASSED...";

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);
  m_progressWidget->show();

  int counter=0;
  QList<PackageListData>::const_iterator it = list->begin();

  while(it != list->end())
  {
    counter++;
    m_progressWidget->setValue(counter);
    ++it;
  }

  if(m_debugInfo)
    std::cout << "Time elapsed iterating over all pkgs from  'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;

  counter = list->count();
  m_progressWidget->setValue(counter);
  m_progressWidget->close();
  m_packageRepo.setData(list, *m_unrequiredPackageList);

  if(m_debugInfo)
    std::cout << "Time elapsed setting the list to the treeview: " << m_time->elapsed() << " mili seconds." << std::endl;

  if (ui->actionSearchByDescription->isChecked())
  {
    m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN);
  }
  else
  {
    m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_NAME_COLUMN);
  }

  delete list;
  list = NULL;

  foreach(QString k, m_outdatedList->keys())
  {
    OutdatedPackageInfo opi = m_outdatedList->value(k);
    PackageRepository::PackageData* package = m_packageRepo.getFirstPackageByNameEx(k);
    if (package != NULL)
    {
      package->status = ectn_OUTDATED;
      package->outdatedVersion = opi.oldVersion;
    }
  }

  if (isAllCategoriesSelected()) m_packageModel->applyFilter(m_selectedViewOption, m_selectedRepository, "");

  reapplyPackageFilter();

  QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->setCurrentIndex(maux);

  refreshTabInfo();
  refreshTabFiles();

  if (isPackageTreeViewVisible())
  {
    ui->tvPackages->setFocus();
  }

  //Refresh statusbar widget
  refreshStatusBar();

  //Refresh application icon
  refreshAppIcon();

  if (firstTime)
  {
    if (isPackageTreeViewVisible())
    {
      m_leFilterPackage->setFocus();
    }

    //m_initializationCompleted = true;
    firstTime = false;

    if (m_callSystemUpgrade)
    {
      //First, let us throw away that 'wainting cursor'...
      QApplication::restoreOverrideCursor();
      doSystemUpgrade();
    }
    else if (m_callSystemUpgradeNoConfirm)
    {
      //First, let us throw away that 'wainting cursor'...
      QApplication::restoreOverrideCursor();
      doSystemUpgrade(ectn_NOCONFIRM_OPT);
    }
    else if (m_packagesToInstallList.count() > 0)
    {
      //First, let us throw away that 'wainting cursor'...
      QApplication::restoreOverrideCursor();
      doInstallLocalPackages();
    }
  }

  ui->tvPackages->setColumnWidth(PackageModel::ctn_PACKAGE_SIZE_COLUMN, 10);
  refreshToolBar();
  refreshStatusBarToolButtons();
  m_refreshPackageLists = true;  

  emit buildPackageListDone();
}

/*
 * Repopulates the list of available packages (installed [+ non-installed])
 */
/*void MainWindow::refreshPackageList()
{
  CPUIntensiveComputing cic;
  const std::unique_ptr<const QSet<QString> > unrequiredPackageList(Package::getUnrequiredPackageList());
  QList<PackageListData> *list = Package::getPackageList();

  // Fetch foreign package list
  std::unique_ptr<QList<PackageListData> > listForeign(Package::getForeignPackageList());
  PackageListData pld;
  QList<PackageListData>::const_iterator itForeign = listForeign->begin();

  if (!isSearchByFileSelected())
  {
    while (itForeign != listForeign->end())
    {
      if (!m_hasAURTool || !m_outdatedAURStringList->contains(itForeign->name))
      {
        pld = PackageListData(
              itForeign->name, itForeign->repository, itForeign->version,
              itForeign->name + " " + Package::getInformationDescription(itForeign->name, true),
              ectn_FOREIGN);
      }
      else
      {
        pld = PackageListData(
              itForeign->name, itForeign->repository, itForeign->version,
              itForeign->name + " " + Package::getInformationDescription(itForeign->name, true),
              ectn_FOREIGN_OUTDATED);
      }

      list->append(pld);
      ++itForeign;
    }
  }

  m_packageRepo.setData(list, *unrequiredPackageList);
  delete list;
  list = NULL;
}
*/

/*
 * Refreshes toolbar in order to insert/remove AUR tool button
 */
void MainWindow::refreshToolBar()
{
  /*m_hasAURTool =
      UnixCommand::hasTheExecutable(StrConstants::getForeignRepositoryToolName()) && !UnixCommand::isRootRunning();

  if (m_hasAURTool)
  {
    if (!ui->mainToolBar->actions().contains(m_actionSwitchToPkgSearch))
    {
      ui->mainToolBar->insertAction(m_dummyAction, m_actionSwitchToPkgSearch);
      m_separatorForActionPkgSearch = ui->mainToolBar->insertSeparator(m_actionSwitchToPkgSearch);
    }
  }
  else
  {
    if (ui->mainToolBar->actions().contains(m_actionSwitchToPkgSearch))
    {
      bool wasChecked = (m_actionSwitchToPkgSearch->isChecked());

      ui->mainToolBar->removeAction(m_actionSwitchToPkgSearch);
      ui->mainToolBar->removeAction(m_separatorForActionPkgSearch);

      if (wasChecked)
      {
        m_actionSwitchToPkgSearch->setChecked(false);
        ui->twGroups->setEnabled(true);
        groupItemSelected();
      }
    }
  }*/
}

/*
 * Refreshes the toolButtons which indicate outdated packages
 */
void MainWindow::refreshStatusBarToolButtons()
{
  if (!isSearchByFileSelected() && !m_actionSwitchToRemoteSearch->isChecked())
    ui->twGroups->setEnabled(true);
}

/*
 * Prints the values of the package counters at the left of the statusBar
 */
void MainWindow::refreshStatusBar()
{
  m_lblSelCounter->setVisible(true);
  m_lblTotalCounters->setVisible(true);
  QString text;
  ui->statusBar->removeWidget(m_toolButtonPacman);

  int numberOfInstalledPackages = m_packageModel->getInstalledPackagesCount();

  if(numberOfInstalledPackages > 0)
  {
    text = StrConstants::getNumberInstalledPackages(numberOfInstalledPackages);
  }
  else if (m_leFilterPackage->text().isEmpty() && !m_packageModel->isFiltered())
  {
    text = StrConstants::getNumberInstalledPackages(m_numberOfInstalledPackages);
  }
  else
  {
    if (isRemoteSearchSelected())
    {
      if (m_packageModel->getPackageCount() == 0)
      {
        m_lblSelCounter->setText("");
        m_lblSelCounter->setVisible(false);
        m_lblTotalCounters->setVisible(false);
      }

      text = "";
    }
    else
    {
      text = StrConstants::getNumberInstalledPackages(0);
    }
  }

  m_lblTotalCounters->setText(text);
  ui->statusBar->addWidget(m_lblTotalCounters);

  if (m_numberOfOutdatedPackages > 0)
  {
    m_toolButtonPacman->show();

    if (m_numberOfOutdatedPackages == 1)
    {
      m_toolButtonPacman->setText("(1)");
      m_toolButtonPacman->setToolTip(StrConstants::getOneNewUpdate());
    }
    else
    {
      m_toolButtonPacman->setText("(" + QString::number(m_numberOfOutdatedPackages) + ")");
      m_toolButtonPacman->setToolTip(StrConstants::getNewUpdates(m_numberOfOutdatedPackages));
    }

    ui->statusBar->addWidget(m_toolButtonPacman);
  }
  else
  {
    m_toolButtonPacman->hide();
    m_toolButtonPacman->setText("");
    m_toolButtonPacman->setToolTip("");
  }
}

/*
 * Clears the counters at the left of the Statusbar
 */
void MainWindow::clearStatusBar()
{
  m_lblSelCounter->clear();
  m_lblTotalCounters->clear();
}

/*
 * Re-populates the HTML view with 'pkgName' package information (tab ONE)
 */
void MainWindow::refreshTabInfo(QString pkgName)
{
  const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkgName);
  if (package == NULL)
    return;

  CPUIntensiveComputing cic;
  QTextBrowser *text = ui->twProperties->widget(
        ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

  if (text)
  {
    text->clear();    
    text->setHtml(OctopiTabInfo::formatTabInfo(*package, *m_outdatedList));
    text->scrollToAnchor(OctopiTabInfo::anchorBegin);

    //We have to clear the cached Info contents...
    m_cachedPackageInInfo = "";
  }
}

/*
 * Re-populates the HTML view with selected package's information (tab ONE)
 */
void MainWindow::refreshTabInfo(bool clearContents, bool neverQuit)
{
  if(neverQuit == false &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_INFORMATION || !isPropertiesTabWidgetVisible())) return;

  QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (clearContents || selectionModel == NULL ||
      selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

    if (text)
    {
      text->clear();
    }

    m_cachedPackageInInfo = "";
    return;
  }

  QModelIndex item = selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).first();
  const PackageRepository::PackageData*const package = m_packageModel->getData(item);
  if (package == NULL) {
    assert(false);
    return;
  }

  //If we are trying to refresh an already displayed package...
  if (m_cachedPackageInInfo == package->repository+"#"+package->name+"#"+package->version)
  {
    if (neverQuit)
    {
      changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
    }

    return;
  }

  CPUIntensiveComputing cic;

  /* Appends all info from the selected package! */
  QTextBrowser *text = ui->twProperties->widget(
        ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    text->clear();
    text->setHtml(OctopiTabInfo::formatTabInfo(*package, *m_outdatedList));
    text->scrollToAnchor(OctopiTabInfo::anchorBegin);
  }

  m_cachedPackageInInfo = package->repository+"#"+package->name+"#"+package->version;

  if (neverQuit)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
  }
}

/*
 * Navigate thru directories to build a hierarquic directory list
 */
void MainWindow::navigateThroughDirs(QStringList parts, QStringList& auxList, int ind)
{
  static QString dir="";
  dir = dir + "/" + parts[ind];

  if (!auxList.contains(dir+"/"))
  {
    auxList.append(dir+"/");
  }

  if (ind < parts.count()-2)
  {
    ind++;
    navigateThroughDirs(parts, auxList, ind);
  }
  else
  {
    dir="";
    return;
  }
}

/*
 * Re-populates the treeview which contains the file list of selected package (tab TWO)
 */
void MainWindow::refreshTabFiles(bool clearContents, bool neverQuit)
{  
  if(neverQuit == false &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_FILES || !isPropertiesTabWidgetVisible()))
  {
    return;
  }

  QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (clearContents || selectionModel == NULL ||
      selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTreeView*const tvPkgFileList =
        ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");

    if(tvPkgFileList)
    {
      QStandardItemModel*const modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
      modelPkgFileList->clear();
      m_cachedPackageInFiles = "";
      bool filterHasFocus = m_leFilterPackage->hasFocus();
      bool tvPackagesHasFocus = ui->tvPackages->hasFocus();
      closeTabFilesSearchBar();
      if (filterHasFocus) m_leFilterPackage->setFocus();
      else if (tvPackagesHasFocus) ui->tvPackages->setFocus();

      return;
    }
  }

  QModelIndex item = selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).first();
  const PackageRepository::PackageData*const package = m_packageModel->getData(item);

  if (package == NULL) {
    assert(false);
    return;
  }

  //If we are trying to refresh an already displayed package...
  if (m_cachedPackageInFiles == package->repository+"#"+package->name+"#"+package->version)
  {
    if (neverQuit)
    {
      changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
      selectFirstItemOfPkgFileList();
    }
    else
    {
      QTreeView*const tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
      if (tv)
        tv->scrollTo(tv->currentIndex());
    }

    return;
  }

  //Maybe this is a non-installed package...
  bool nonInstalled = (package->installed() == false);

  QTreeView*const tvPkgFileList =
      ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");

  if (tvPkgFileList)
  {
    QString pkgName = package->name;
    QStringList fileList;
    QStandardItemModel *fakeModelPkgFileList = new QStandardItemModel(this);
    QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());

    modelPkgFileList->clear();
    QStandardItem *fakeRoot = fakeModelPkgFileList->invisibleRootItem();
    QStandardItem *root = modelPkgFileList->invisibleRootItem();
    QStandardItem *lastDir, *item, *lastItem=root, *parent;
    bool first=true;
    lastDir = root;
    QEventLoop el;
    QFuture<QStringList> f;
    QFutureWatcher<QStringList> fwPackageContents;
    f = QtConcurrent::run(Package::getContents, pkgName, !nonInstalled);
    connect(&fwPackageContents, SIGNAL(finished()), &el, SLOT(quit()));
    fwPackageContents.setFuture(f);

    //Let's wait before we get the pkg file list from the other thread...
    el.exec();
    fileList = fwPackageContents.result();

    //Let's change that listing a bit...
    QStringList auxList;
    foreach(QString file, fileList)
    {
      QStringList parts = file.split("/", QString::SkipEmptyParts);
      navigateThroughDirs(parts, auxList, 0);
    }

    fileList = fileList + auxList;
    fileList.sort();

    if (fileList.count() > 0) CPUIntensiveComputing cic;

    QString fullPath;
    bool isSymLinkToDir = false;

    foreach ( QString file, fileList )
    {
      bool isDir = file.endsWith('/');
      isSymLinkToDir = false;
      QString baseFileName = extractBaseFileName(file);

      //Let's test if it is not a symbolic link to a dir
      if(!isDir)
      {
        QFileInfo fiTestForSymLink(file);
        if(fiTestForSymLink.isSymLink())
        {
          QFileInfo fiTestForDir(fiTestForSymLink.symLinkTarget());
          isSymLinkToDir = fiTestForDir.isDir();
        }
      }

      if(isDir){
        if ( first == true ){
          item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
          item->setAccessibleDescription("directory " + item->text());
          fakeRoot->appendRow ( item );
        }
        else{
          fullPath = utils::showFullPathOfItem(lastDir->index());
          //std::cout << "Testing if " << file.toLatin1().data() << " contains " << fullPath.toLatin1().data() << std::endl;
          if ( file.contains ( fullPath )) {
            //std::cout << "It contains !!! So " << fullPath.toLatin1().data() << " is its parent." << std::endl;
            item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
            item->setAccessibleDescription("directory " + item->text());
            lastDir->appendRow ( item );
          }
          else{
            //std::cout << "It doens't contain..." << std::endl;
            parent = lastItem->parent();
            if (parent != 0) fullPath = utils::showFullPathOfItem(parent->index());

            do{
              //if (parent != 0) std::cout << "Testing if " << file.toLatin1().data() << " contains " << fullPath.toLatin1().data() << std::endl;
              if ( parent == 0 || file.contains ( fullPath )) break;
              parent = parent->parent();
              if (parent != 0) fullPath = utils::showFullPathOfItem(parent->index());
            }
            while ( parent != fakeRoot );

            item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
            item->setAccessibleDescription("directory " + item->text());

            if ( parent != 0 )
            {
              //std::cout << item->text().toLatin1().data() << " is son of " << fullPath.toLatin1().data() << std::endl;
              parent->appendRow ( item );
            }
            else
            {
              //std::cout << item->text().toLatin1().data() << " is son of <FAKEROOT>" << std::endl;
              fakeRoot->appendRow ( item );
            }
          }
        }

        lastDir = item;
      }            
      else if (isSymLinkToDir)
      {
        item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
        item->setAccessibleDescription("directory " + item->text());
        parent = lastDir;
        if (parent != 0) fullPath = utils::showFullPathOfItem(parent->index());

        do{
          if ( parent == 0 || file.contains ( fullPath )) break;
          parent = parent->parent();
          if (parent != 0) fullPath = utils::showFullPathOfItem(parent->index());
        }
        while ( parent != fakeRoot );

        if (parent != 0)
        {
          parent->appendRow ( item );
        }
        else
        {
          fakeRoot->appendRow ( item );
        }
      }
      else
      {
        item = new QStandardItem ( IconHelper::getIconBinary(), baseFileName );
        item->setAccessibleDescription("file " + item->text());
        parent = lastDir;
        if (parent != 0) fullPath = utils::showFullPathOfItem(parent->index());

        do{
          if ( parent == 0 || file.contains ( fullPath )) break;
          parent = parent->parent();
          if (parent != 0) fullPath = utils::showFullPathOfItem(parent->index());
        }
        while ( parent != fakeRoot );

        parent->appendRow ( item );
      }

      lastItem = item;
      first = false;
    }

    root = fakeRoot;
    fakeModelPkgFileList->sort(0);
    modelPkgFileList = fakeModelPkgFileList;
    tvPkgFileList->setModel(modelPkgFileList);
    tvPkgFileList->header()->setDefaultAlignment( Qt::AlignCenter );
    modelPkgFileList->setHorizontalHeaderLabels( QStringList() << StrConstants::getContentsOf().arg(pkgName));
  }

  m_cachedPackageInFiles = package->repository+"#"+package->name+"#"+package->version;

  if (neverQuit)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
    selectFirstItemOfPkgFileList();
  }

  closeTabFilesSearchBar();
}

/*
 * This SLOT is called every time we press a key at FilterLineEdit
 */
void MainWindow::reapplyPackageFilter()
{
  if (!isSearchByFileSelected())
  {
    bool isFilterPackageSelected = m_leFilterPackage->hasFocus();
    QString search = Package::parseSearchString(m_leFilterPackage->text());
    m_packageModel->applyFilter(search);

    int numPkgs = m_packageModel->getPackageCount();

    if (m_leFilterPackage->text() != ""){
      if (numPkgs > 0) m_leFilterPackage->setFoundStyle();
      else m_leFilterPackage->setNotFoundStyle();
    }
    else{
      m_leFilterPackage->initStyleSheet();
      m_packageModel->applyFilter("");
    }

    if (isFilterPackageSelected || numPkgs == 0)
    {
      m_leFilterPackage->setFocus();
    }

    if (numPkgs == 0)
      tvPackagesSelectionChanged(QItemSelection(),QItemSelection());

    ui->tvPackages->selectionModel()->clear();
    QModelIndex mi = m_packageModel->index(0, PackageModel::ctn_PACKAGE_NAME_COLUMN, QModelIndex());
    ui->tvPackages->setCurrentIndex(mi);
    ui->tvPackages->scrollTo(mi);
    invalidateTabs();
  }
  //If we are using "Search By file...
  else
  {
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));

    m_leFilterPackage->initStyleSheet();

    //We need to provide QCompleter data to the SearchLineEdit...
    if (!m_leFilterPackage->text().isEmpty())
      m_leFilterPackage->refreshCompleterData();

    connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
  }
}

/*
 * Whenever user selects View/All we show him all the available packages
 */
void MainWindow::selectedAllPackagesMenu()
{  
  m_selectedViewOption = ectn_ALL_PKGS;
  changePackageListModel(ectn_ALL_PKGS, m_selectedRepository);
}

/*
 * Whenever user selects View/Installed we show him only the installed packages
 */
void MainWindow::selectedInstalledPackagesMenu()
{
  m_selectedViewOption = ectn_INSTALLED_PKGS;
  changePackageListModel(ectn_INSTALLED_PKGS, m_selectedRepository);
}

/*
 * Whenever user selects View/Non Installed we show him only the non installed packages
 */
void MainWindow::selectedNonInstalledPackagesMenu()
{
  m_selectedViewOption = ectn_NON_INSTALLED_PKGS;
  changePackageListModel(ectn_NON_INSTALLED_PKGS, m_selectedRepository);
}

/*
 * Whenever user selects any of the available repositories in View/Repository submenu,
 * we filter the packages from that chosen repo
 */
void MainWindow::selectedRepositoryMenu(QAction *actionRepoSelected)
{
  if (actionRepoSelected->text() == StrConstants::getAll())
    m_selectedRepository = "";
  else
    m_selectedRepository = actionRepoSelected->text();

  changePackageListModel(m_selectedViewOption, m_selectedRepository);
}
