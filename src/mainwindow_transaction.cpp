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

/*
 * This is MainWindow's Transaction related code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "uihelper.h"
#include "wmhelper.h"
#include "strconstants.h"
#include "transactiondialog.h"
#include "multiselectiondialog.h"
#include "globals.h"
#include <iostream>
#include <cassert>
#include "searchlineedit.h"
#include "xbpsexec.h"
#include "termwidget.h"

#include <QComboBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QStandardItem>
#include <QTextBrowser>
#include <QDebug>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

/*
 * Disables transaction buttons
 */
void MainWindow::disableTransactionButtons()
{
  ui->actionCommit->setEnabled(false);
  ui->actionCancel->setEnabled(false);
}

/*
 * Watches the state of tvTransaction treeview to see if Commit/Cancel actions must be activated/deactivated
 */
void MainWindow::changeTransactionActionsState()
{
  bool state = isThereAPendingTransaction();
  ui->actionCommit->setEnabled(state);
  ui->actionCancel->setEnabled(state);
  ui->actionSyncPackages->setEnabled(!state);

  if (state == false && m_outdatedStringList->count() > 0)
    ui->actionSystemUpgrade->setEnabled(true);
  else if (state == true)
    ui->actionSystemUpgrade->setEnabled(false);
}

/*
 * Removes all packages from the current transaction
 */
void MainWindow::clearTransactionTreeView()
{
  removePackagesFromRemoveTransaction();
  removePackagesFromInstallTransaction();
}

/*
 * Looks if the Transaction's Remove or Install parent items have any package inside them
 */
bool MainWindow::isThereAPendingTransaction()
{
  return (getRemoveTransactionParentItem()->hasChildren() ||
          getInstallTransactionParentItem()->hasChildren());
}

/*
 * Retrieves the Remove parent item of the Transaction treeview
 */
QStandardItem * MainWindow::getRemoveTransactionParentItem()
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());
  QStandardItem *si = 0;

  if(sim)
  {
    si = sim->item(0, 0);
  }

  return si;
}

/*
 * Retrieves the Install parent item of the Transaction treeview
 */
QStandardItem * MainWindow::getInstallTransactionParentItem()
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());
  QStandardItem *si = 0;

  if(sim)
  {
    si = sim->item(1, 0);
  }

  return si;
}

/*
 * Inserts the given package into the Remove parent item of the Transaction treeview
 */
void MainWindow::insertRemovePackageIntoTransaction(const QString &pkgName)
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToRemove = new QStandardItem(IconHelper::getIconRemoveItem(), pkgName);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siRemoveParent->model());
  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  int slash = pkgName.indexOf("/");
  QString pkg = pkgName.mid(slash+1);
  siPackageToRemove->setText(pkg);

  if (foundItems.size() == 0)
  {
    int slash = pkgName.indexOf("/");
    QString pkg = pkgName.mid(slash+1);
    QList<QStandardItem *> aux = sim->findItems(pkg, Qt::MatchRecursive | Qt::MatchExactly);

    if (aux.count() == 0) siRemoveParent->appendRow(siPackageToRemove);
  }
  else if (foundItems.size() == 1 && foundItems.at(0)->parent() == siInstallParent)
  {
    siInstallParent->removeRow(foundItems.at(0)->row());
    siRemoveParent->appendRow(siPackageToRemove);
  }

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_TRANSACTION);
  tvTransaction->expandAll();
  changeTransactionActionsState();
}

/*
 * Inserts the given package into the Install parent item of the Transaction treeview
 */
void MainWindow::insertInstallPackageIntoTransaction(const QString &pkgName)
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToInstall = new QStandardItem(IconHelper::getIconInstallItem(), pkgName);
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siInstallParent->model());
  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  if (foundItems.size() == 0)
  {
    int slash = pkgName.indexOf("/");
    QString pkg = pkgName.mid(slash+1);
    QList<QStandardItem *> aux = sim->findItems(pkg, Qt::MatchRecursive | Qt::MatchExactly);

    if (aux.count() > 0) siRemoveParent->removeRow(aux.at(0)->row());
    siInstallParent->appendRow(siPackageToInstall);
  }
  else if (foundItems.size() == 1 && foundItems.at(0)->parent() == siRemoveParent)
  {
    siRemoveParent->removeRow(foundItems.at(0)->row());
    siInstallParent->appendRow(siPackageToInstall);
  }

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_TRANSACTION);
  tvTransaction->expandAll();
  changeTransactionActionsState();
}

/*
 * Removes all packages from the Remove parent item of the Transaction treeview
 */
void MainWindow::removePackagesFromRemoveTransaction()
{
  QStandardItem * siRemove = getRemoveTransactionParentItem();
  siRemove->removeRows(0, siRemove->rowCount());
  changeTransactionActionsState();
}

/*
 * Removes all packages from the Install parent item of the Transaction treeview
 */
void MainWindow::removePackagesFromInstallTransaction()
{
  QStandardItem * siInstall = getInstallTransactionParentItem();
  siInstall->removeRows(0, siInstall->rowCount());
  changeTransactionActionsState();
}

/*
 * Retrieves the number of "to be removed" packages
 */
int MainWindow::getNumberOfTobeRemovedPackages()
{
  QStandardItem * siRemoval = getRemoveTransactionParentItem();
  return siRemoval->rowCount();
}

/*
 * Retrieves the list of all packages scheduled to be removed
 */
QString MainWindow::getTobeRemovedPackages()
{
  QStandardItem * siRemoval = getRemoveTransactionParentItem();
  QString res;

  for(int c=0; c < siRemoval->rowCount(); c++)
  {
    res += siRemoval->child(c)->text() + " ";
  }

  res = res.trimmed();
  return res;
}

/*
 * Retrieves the list of all packages scheduled to be installed
 */
QString MainWindow::getTobeInstalledPackages()
{
  QStandardItem * siInstall = getInstallTransactionParentItem();
  QString res;

  for(int c=0; c < siInstall->rowCount(); c++)
  {
    res += siInstall->child(c)->text() + " ";
  }

  res = res.trimmed();
  return res;
}

/*
 * Inserts the current selected packages for removal into the Transaction Treeview
 * This is the SLOT, it needs to call insertRemovePackageIntoTransaction(PackageName) to work!
 */
void MainWindow::insertIntoRemovePackage()
{
  qApp->processEvents();
  //bool checkDependencies=false;
  //QStringList dependencies;

  ensureTabVisible(ctn_TABINDEX_TRANSACTION);
  QModelIndexList selectedRows = ui->tvPackages->selectionModel()->selectedRows();

  //First, let's see if we are dealing with a package group
  if(!isAllCategoriesSelected())
  {
    //If we are trying to remove all the group's packages, why not remove the entire group?
    if(selectedRows.count() == m_packageModel->getPackageCount())
    {
      insertRemovePackageIntoTransaction(getSelectedCategory());
      return;
    }
  }

  foreach(QModelIndex item, selectedRows)
  {
    const PackageRepository::PackageData*const package = m_packageModel->getData(item);
    if (package == NULL) {
      assert(false);
      continue;
    }

    /*if(checkDependencies)
      {
        QStringList *targets = Package::getTargetRemovalList(package->name);

        foreach(QString target, *targets)
        {
          int separator = target.lastIndexOf("-");
          QString candidate = target.left(separator);
          separator = candidate.lastIndexOf("-");
          candidate = candidate.left(separator);

          if (candidate != package->name)
          {
            dependencies.append(candidate);
          }
        }

        if (dependencies.count() > 0)
        {
          if (!dependencies.at(0).contains("HoldPkg was found in"))
          {
            if (!insertIntoRemovePackageDeps(dependencies))
              return;
          }
        }
      }*/

    insertRemovePackageIntoTransaction(package->repository + "/" + package->name);
  }
}

/*
 * Inserts the current selected group for removal into the Transaction Treeview
 */
void MainWindow::insertGroupIntoRemovePackage()
{
  ensureTabVisible(ctn_TABINDEX_TRANSACTION);
  insertRemovePackageIntoTransaction(getSelectedCategory());
}

/*
 * Inserts the current selected packages for installation into the Transaction Treeview
 * This is the SLOT, it needs to call insertInstallPackageIntoTransaction(PackageName) to work!
 */
void MainWindow::insertIntoInstallPackage()
{
  qApp->processEvents();
  ensureTabVisible(ctn_TABINDEX_TRANSACTION);
  QModelIndexList selectedRows = ui->tvPackages->selectionModel()->selectedRows();

  foreach(QModelIndex item, selectedRows)
  {
    const PackageRepository::PackageData*const package = m_packageModel->getData(item);
    if (package == NULL) {
      assert(false);
      continue;
    }

    insertInstallPackageIntoTransaction(package->name);
  }
}

/*
 * Searches in Install Transaction queue for the given package
 */
bool MainWindow::isPackageInInstallTransaction(const QString &pkgName)
{
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siInstallParent->model());
  const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkgName);
  QString repo;

  if (package != NULL) repo = package->repository;

  QList<QStandardItem *> foundItems = sim->findItems(repo + "/" + pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  return (foundItems.size() > 0);
}

/*
 * Searches in Remove Transaction queue for the given package
 */
bool MainWindow::isPackageInRemoveTransaction(const QString &pkgName)
{
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siRemoveParent->model());
  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  return (foundItems.size() > 0);
}

/*
 * Inserts all optional deps of the current select package into the Transaction Treeview
 */
/*void MainWindow::insertIntoInstallPackageOptDeps(const QString &packageName)
{
  CPUIntensiveComputing *cic = new CPUIntensiveComputing;

  //Does this package have non installed optional dependencies?
  QStringList optDeps = Package::getOptionalDeps(packageName); //si->text());
  QList<const PackageRepository::PackageData*> optionalPackages;

  foreach(QString optDep, optDeps)
  {
    QString candidate = optDep;
    int points = candidate.indexOf(":");
    candidate = candidate.mid(0, points).trimmed();

    const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(candidate);
    if(!isPackageInInstallTransaction(candidate) &&
       !isPackageInstalled(candidate) && package != 0)
    {
      optionalPackages.append(package);
    }
  }

  if(optionalPackages.count() > 0)
  {
    MultiSelectionDialog *msd = new MultiSelectionDialog(this);
    msd->setWindowTitle(packageName + ": " + StrConstants::getOptionalDeps());
    msd->setWindowIcon(windowIcon());
    QStringList selectedPackages;

    foreach(const PackageRepository::PackageData* candidate, optionalPackages)
    {
      QString desc = candidate->description;
      int space = desc.indexOf(" ");
      desc = desc.mid(space+1);

      msd->addPackageItem(candidate->name, candidate->description, candidate->repository);
    }

    delete cic;
    if (msd->exec() == QMessageBox::Ok)
    {
      selectedPackages = msd->getSelectedPackages();
      foreach(QString pkg, selectedPackages)
      {
        insertInstallPackageIntoTransaction(pkg);
      }
    }

    delete msd;
  }
  else
  {
    delete cic;
  }
}*/

/*
 * Inserts all remove dependencies of the current select package into the Transaction Treeview
 * Returns TRUE if the user click OK or ENTER and number of selected packages > 0.
 * Returns FALSE otherwise.
 */
/*bool MainWindow::insertIntoRemovePackageDeps(const QStringList &dependencies)
{
  QList<const PackageRepository::PackageData*> newDeps;
  foreach(QString dep, dependencies)
  {
    const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(dep);
    if (package != NULL && package->installed() && !isPackageInRemoveTransaction(dep))
    {
      newDeps.append(package);
    }
  }

  if (newDeps.count() > 0)
  {
    CPUIntensiveComputing *cic = new CPUIntensiveComputing;
    MultiSelectionDialog *msd = new MultiSelectionDialog(this);
    msd->setWindowTitle(StrConstants::getRemovePackages(newDeps.count()));
    msd->setWindowIcon(windowIcon());
    QStringList selectedPackages;

    foreach(const PackageRepository::PackageData* dep, newDeps)
    {
      QString desc = dep->description;
      int space = desc.indexOf(" ");
      desc = desc.mid(space+1);

      if(dep->repository == StrConstants::getForeignRepositoryName() && dep->description.isEmpty())
      {
        desc = Package::getInformationDescription(dep->name, true);
      }

      msd->addPackageItem(dep->name, desc, dep->repository);
    }

    msd->setAllSelected();
    delete cic;
    int res = msd->exec();

    if (res == QMessageBox::Ok)
    {
      selectedPackages = msd->getSelectedPackages();
      foreach(QString pkg, selectedPackages)
      {
        insertRemovePackageIntoTransaction(pkg);
      }
    }

    delete msd;

    return (res == QMessageBox::Ok && selectedPackages.count() >= 0);
  }
  else return true;
}*/

/*
 * Inserts the current selected group for removal into the Transaction Treeview
 */
void MainWindow::insertGroupIntoInstallPackage()
{
  ensureTabVisible(ctn_TABINDEX_TRANSACTION);
  insertInstallPackageIntoTransaction(getSelectedCategory());
}

/*
 * Adjust the count and selection count status of the selected tvTransaction item (Remove or Insert parents)
 */
void MainWindow::tvTransactionAdjustItemText(QStandardItem *item)
{
  int countSelected=0;

  QTreeView *tvTransaction =
      ui->twProperties->currentWidget()->findChild<QTreeView*>("tvTransaction");
  if (!tvTransaction) return;

  for(int c=0; c < item->rowCount(); c++)
  {
    if(tvTransaction->selectionModel()->isSelected(item->child(c)->index()))
    {
      countSelected++;
    }
  }

  QString itemText = item->text();
  int slash = itemText.indexOf("/");
  int pos = itemText.indexOf(")");

  if (slash > 0){
    itemText.remove(slash, pos-slash);
  }

  pos = itemText.indexOf(")");
  itemText.insert(pos, "/" + QString::number(countSelected));
  item->setText(itemText);
}

/*
 * SLOT called each time the selection of items in tvTransaction is changed
 */
void MainWindow::tvTransactionSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  tvTransactionAdjustItemText(getRemoveTransactionParentItem());
  tvTransactionAdjustItemText(getInstallTransactionParentItem());
}

/*
 * Method called every time some item is inserted or removed in tvTransaction treeview
 */
void MainWindow::tvTransactionRowsChanged(const QModelIndex& parent)
{
  QStandardItem *item = m_modelTransaction->itemFromIndex(parent);
  QString count = QString::number(item->rowCount());

  QStandardItem * itemRemove = getRemoveTransactionParentItem();
  QStandardItem * itemInstall = getInstallTransactionParentItem();

  if (item == itemRemove)
  {
    if (item->rowCount() > 0)
    {
      itemRemove->setText(StrConstants::getTransactionRemoveText() + " (" + count + ")");
      tvTransactionAdjustItemText(itemRemove);
    }
    else itemRemove->setText(StrConstants::getTransactionRemoveText());
  }
  else if (item == itemInstall)
  {
    if (item->rowCount() > 0)
    {
      itemInstall->setText(StrConstants::getTransactionInstallText() + " (" + count + ")");
      tvTransactionAdjustItemText(itemInstall);
    }
    else itemInstall->setText(StrConstants::getTransactionInstallText());
  }
}

/*
 * SLOT called each time some item is inserted into tvTransaction
 */
void MainWindow::tvTransactionRowsInserted(const QModelIndex& parent, int, int)
{
  tvTransactionRowsChanged(parent);
}

/*
 * SLOT called each time some item is removed from tvTransaction
 */
void MainWindow::tvTransactionRowsRemoved(const QModelIndex& parent, int, int)
{
  tvTransactionRowsChanged(parent);
}

/*
 * Whenever the user presses DEL over the Transaction TreeView, we:
 * - Delete the package if it's bellow of "To be removed" or "To be installed" parent;
 * - Delete all the parent's packages if the user clicked in "To be removed" or "To be installed" items.
 */
void MainWindow::onPressDelete()
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");

  if (tvTransaction->hasFocus())
  {
    if(tvTransaction->currentIndex() == getRemoveTransactionParentItem()->index()){
      removePackagesFromRemoveTransaction();
    }
    else if(tvTransaction->currentIndex() == getInstallTransactionParentItem()->index()){
      removePackagesFromInstallTransaction();
    }
    else
    {
      for(int c=tvTransaction->selectionModel()->selectedIndexes().count()-1; c>=0; c--)
      {
        const QModelIndex mi = tvTransaction->selectionModel()->selectedIndexes().at(c);
        if (m_modelTransaction->itemFromIndex(mi)->parent() != 0)
        {
          m_modelTransaction->removeRow(mi.row(), mi.parent());
        }
      }
    }

    changeTransactionActionsState();
  }
}

/*
 * Checks if some SU utility is available...
 * Returns false if not!
 */
bool MainWindow::isSUAvailable()
{
  //If there are no means to run the actions, we must warn!
  if (UnixCommand::isRootRunning() && WMHelper::isKDERunning())
  {
    return true;
  }
  else if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
    QMessageBox::critical( 0, StrConstants::getApplicationName(),
                           StrConstants::getErrorNoSuCommand() +
                           "\n" + StrConstants::getYoullNeedSuFrontend());
    return false;
  }
  else
    return true;
}

/*
 * Does a repository sync with "pkg update -f" !
 */
void MainWindow::doSyncDatabase()
{
  m_progressWidget->setRange(0, 100);
  if (!doRemovePacmanLockFile()) return;

  m_commandExecuting = ectn_SYNC_DATABASE;
  disableTransactionActions();
  clearTabOutput();

  m_xbpsExec = new XBPSExec();
  if (m_debugInfo)
    m_xbpsExec->setDebugMode(true);

  QObject::connect(m_xbpsExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( xbpsProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_xbpsExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_xbpsExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));

  m_xbpsExec->doSyncDatabase();
}

/*
 * doSystemUpgrade shared code ...
 */
void MainWindow::prepareSystemUpgrade()
{
  m_systemUpgradeDialog = false;
  if (!doRemovePacmanLockFile()) return;

  clearTabOutput();

  m_xbpsExec = new XBPSExec();
  if (m_debugInfo)
    m_xbpsExec->setDebugMode(true);

  QObject::connect(m_xbpsExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( xbpsProcessFinished(int, QProcess::ExitStatus) ));
  QObject::connect(m_xbpsExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_xbpsExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_xbpsExec, SIGNAL(commandToExecInQTermWidget(QString)),
                   this, SLOT(onExecCommandInTabTerminal(QString)));

  disableTransactionActions();
}

/*
 * Prepares the Package::getTargetUpgradeList() transaction info in a separate thread
 */
void MainWindow::prepareTargetUpgradeList(const QString& pkgName, CommandExecuting type)
{
  QFuture<TransactionInfo> f;
  f = QtConcurrent::run(getTargetUpgradeList, pkgName);

  disconnect(&g_fwTargetUpgradeList, SIGNAL(finished()), this, SLOT(doInstall()));
  disconnect(&g_fwTargetUpgradeList, SIGNAL(finished()), this, SLOT(doRemoveAndInstall()));
  disconnect(&g_fwTargetUpgradeList, SIGNAL(finished()), this, SLOT(doSystemUpgrade()));

  if (type == ectn_INSTALL)
  {
    toggleSystemActions(false);
    ui->actionCommit->setEnabled(false);
    ui->actionCancel->setEnabled(false);

    connect(&g_fwTargetUpgradeList, SIGNAL(finished()), this, SLOT(doInstall()));
  }
  else if (type == ectn_REMOVE_INSTALL)
  {
    toggleSystemActions(false);
    ui->actionCommit->setEnabled(false);
    ui->actionCancel->setEnabled(false);

    connect(&g_fwTargetUpgradeList, SIGNAL(finished()), this, SLOT(doRemoveAndInstall()));
  }
  else if (type == ectn_SYSTEM_UPGRADE)
  {
    toggleSystemActions(false);

    connect(&g_fwTargetUpgradeList, SIGNAL(finished()), this, SLOT(doSystemUpgrade()));
  }

  g_fwTargetUpgradeList.setFuture(f);
}

/*
 * Prepares the 'pkg upgrade' transaction information
 */
void MainWindow::doPreSystemUpgrade()
{
  prepareTargetUpgradeList();
}

/*
 * Does a system upgrade with "pacman -Su" !
 */
void MainWindow::doSystemUpgrade(SystemUpgradeOptions)
{
  if (m_systemUpgradeDialog) return;

  if(m_callSystemUpgrade && m_numberOfOutdatedPackages == 0)
  {
    m_callSystemUpgrade = false;
    return;
  }
  else if (m_callSystemUpgradeNoConfirm && m_numberOfOutdatedPackages == 0)
  {
    m_callSystemUpgrade = false;
    return;
  }

  if (!isSUAvailable()) return;

  m_progressWidget->setRange(0, 100);

  //Shows a dialog indicating the targets needed to be retrieved and asks for the user's permission.
  TransactionInfo ti = g_fwTargetUpgradeList.result(); //Package::getTargetUpgradeList();
  QStringList *targets = ti.packages;

  //There are no new updates to install!
  if (targets->count() == 0 && m_outdatedStringList->count() == 0)
  {
    clearTabOutput();
    writeToTabOutput("<b>" + StrConstants::getNoNewUpdatesAvailable() + "</b>");
    return;
  }
  else if (targets->count() == 0 && m_outdatedStringList->count() > 0)
  {
    //This is a bug and should be shown to the user!
    clearTabOutput();
    return;
  }

  QString list;
  bool upgradeXBPS=false;
  foreach(QString target, *targets)
  {
    if (target == "xbps") upgradeXBPS=true;
    list = list + target + "\n";
  }

  //Let's build the system upgrade transaction dialog...
  QString ds = ti.sizeToDownload;

  TransactionDialog question(this);

  if(targets->count()==1)
    question.setText(StrConstants::getRetrievePackage() +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));
  else
    question.setText(StrConstants::getRetrievePackages(targets->count()) +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));

  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);

  m_systemUpgradeDialog = true;
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    prepareSystemUpgrade();

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_SYSTEM_UPGRADE;
      m_xbpsExec->doSystemUpgrade(upgradeXBPS);
      m_commandQueued = ectn_NONE;
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
      m_xbpsExec->doSystemUpgradeInTerminal(upgradeXBPS);
      m_commandQueued = ectn_NONE;
    }
  }
  else if (result == QDialogButtonBox::No)
  {
    m_systemUpgradeDialog = false;
    enableTransactionActions();
    toggleSystemActions(true);
  }
}

/*
 * Prepares the Remove and Install transaction information
 */
void MainWindow::doPreRemoveAndInstall()
{
  QString listOfInstallTargets = getTobeInstalledPackages();
  prepareTargetUpgradeList(listOfInstallTargets, ectn_REMOVE_INSTALL);
}

/*
 * Removes and Installs all selected packages in two transactions using
 * a QUEUED remove command: *
 */
void MainWindow::doRemoveAndInstall()
{
  m_progressWidget->setRange(0, 100);
  QString listOfRemoveTargets = getTobeRemovedPackages();
  QString removeList;
  QString allLists;
  TransactionDialog question(this);
  QString dialogText;

  QStringList removeTargets = listOfRemoveTargets.split(" ", Qt::SkipEmptyParts);
  foreach(QString target, removeTargets)
  {
    removeList = removeList + StrConstants::getRemove() + " "  + target + "\n";
  }

  QString listOfInstallTargets = getTobeInstalledPackages();
  TransactionInfo ti = g_fwTargetUpgradeList.result();
  QStringList *installTargets = ti.packages;
  QString ds = ti.sizeToDownload;

  if (ti.sizeToDownload == 0) ds = "0.00 Bytes";

  QString installList;

  foreach(QString target, *installTargets)
  {
    installList = installList + StrConstants::getInstall() + " " + target + "\n";
  }

  allLists.append(removeList);
  allLists.append(installList);

  if (removeTargets.count() == 1)
  {
    dialogText = StrConstants::getRemovePackage() + "\n";
  }
  else if (removeTargets.count() > 1)
  {
    dialogText = StrConstants::getRemovePackages(removeTargets.count()) + "\n";
  }
  if (installTargets->count() == 1)
  {
    dialogText += StrConstants::getRetrievePackage() +
      "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB");
  }
  else if (installTargets->count() > 1)
  {
    dialogText += StrConstants::getRetrievePackages(installTargets->count()) +
      "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB");
  }

  question.setText(dialogText);
  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(allLists);
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    if (!doRemovePacmanLockFile()) return;

    disableTransactionButtons();
    clearTabOutput();

    m_xbpsExec = new XBPSExec();
    if (m_debugInfo)
      m_xbpsExec->setDebugMode(true);

    QObject::connect(m_xbpsExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( xbpsProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_xbpsExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_xbpsExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_xbpsExec, SIGNAL(commandToExecInQTermWidget(QString)),
                     this, SLOT(onExecCommandInTabTerminal(QString)));

    disableTransactionActions();

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_REMOVE;
      m_xbpsExec->doRemoveAndInstall(listOfRemoveTargets, listOfInstallTargets);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_xbpsExec->doRemoveAndInstallInTerminal(listOfRemoveTargets, listOfInstallTargets);
    }
  }
  else
  {
    m_commandExecuting = ectn_NONE;
    enableTransactionActions();
  }
}

/*
 * Removes ALL the packages selected by the user with "pacman -Rcs (CASCADE)" !
 */
void MainWindow::doRemove()
{
  m_progressWidget->setRange(0, 100);
  QString listOfTargets = getTobeRemovedPackages();
  QStringList *_targets = Package::getTargetRemovalList(listOfTargets);
  listOfTargets = "";      
  QString list;

  if (_targets->count() == 0)
  {
    QMessageBox::warning(
          this, StrConstants::getAttention(), StrConstants::getWarnTransactionAborted(), QMessageBox::Ok);
    return;
  }

  foreach(QString target, *_targets)
  {
    list = list + target + "\n";
    listOfTargets += target + " ";
  }

  TransactionDialog question(this);

  //Shows a dialog indicating the targets which will be removed and asks for the user's permission.  
  if(_targets->count()==1)
  {
    question.setText(StrConstants::getRemovePackage());
  }
  else
    question.setText(StrConstants::getRemovePackages(_targets->count()));

  if (getNumberOfTobeRemovedPackages() < _targets->count())
    question.setWindowTitle(StrConstants::getWarning());
  else
    question.setWindowTitle(StrConstants::getConfirmation());

  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    if (!doRemovePacmanLockFile()) return;

    //disableTransactionButtons();
    clearTabOutput();

    m_xbpsExec = new XBPSExec();
    if (m_debugInfo)
      m_xbpsExec->setDebugMode(true);

    QObject::connect(m_xbpsExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( xbpsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_xbpsExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_xbpsExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_xbpsExec, SIGNAL(commandToExecInQTermWidget(QString)),
                     this, SLOT(onExecCommandInTabTerminal(QString)));

    disableTransactionActions();

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_REMOVE;
      m_xbpsExec->doRemove(listOfTargets);
    }

    if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_xbpsExec->doRemoveInTerminal(listOfTargets);
    }
  }
  else
  {
    m_commandExecuting = ectn_NONE;
    enableTransactionActions();
  }
}

/*
 * Prepares the Install transaction information
 */
void MainWindow::doPreInstall()
{
  QString listOfTargets = getTobeInstalledPackages();
  prepareTargetUpgradeList(listOfTargets, ectn_INSTALL);
}

/*
 * Installs ALL the packages selected by the user with "xbps-install (INCLUDING DEPENDENCIES)" !
 */
void MainWindow::doInstall()
{
  m_progressWidget->setRange(0, 100);
  QString listOfTargets = getTobeInstalledPackages();

  TransactionInfo ti = g_fwTargetUpgradeList.result();
  QStringList *targets = ti.packages;

  if (targets->count() == 0)
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(),
                           StrConstants::getPkgNotAvailable());
    enableTransactionActions();
    return;
  }

  QString list;
  QString ds = ti.sizeToDownload;

  if (ti.sizeToDownload == 0) ds = "0.00 Bytes";

  TransactionDialog question(this);

  foreach(QString target, *targets)
  {
    list = list + target + "\n";
  }

  if(targets->count()==1)
  {
    question.setText(StrConstants::getRetrievePackage() +
                          "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));
  }
  else if (targets->count() > 1)
    question.setText(StrConstants::getRetrievePackages(targets->count()) +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));

  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    if (!doRemovePacmanLockFile()) return;

    disableTransactionButtons();
    disableTransactionActions();
    clearTabOutput();

    m_xbpsExec = new XBPSExec();
    if (m_debugInfo)
      m_xbpsExec->setDebugMode(true);

    QObject::connect(m_xbpsExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( xbpsProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_xbpsExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_xbpsExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_xbpsExec, SIGNAL(commandToExecInQTermWidget(QString)),
                     this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_INSTALL;
      m_xbpsExec->doInstall(listOfTargets);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_xbpsExec->doInstallInTerminal(listOfTargets);
    }
  }
  else
  {
    m_commandExecuting = ectn_NONE;
    enableTransactionActions();
  }
}

/*
 * If the Pacman lock file exists ("/var/run/pacman.lck"), removes it!
 */
bool MainWindow::doRemovePacmanLockFile()
{
  //If there are no means to run the actions, we must warn!
  if (!isSUAvailable()) return false;

  /*QString lockFilePath("/var/lib/pacman/db.lck");
  QFile lockFile(lockFilePath);

  if (lockFile.exists())
  {
    int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                    StrConstants::getRemovePacmanTransactionLockFileConfirmation(),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
      qApp->processEvents();

      clearTabOutput();
      writeToTabOutputExt("<b>" + StrConstants::getRemovingPacmanTransactionLockFile() + "</b>");
      UnixCommand::execCommand("rm " + lockFilePath);
      writeToTabOutputExt("<b>" + StrConstants::getCommandFinishedOK() + "</b>");
    }
  }*/

  return true;
}

/*
 * Installs ALL the packages manually selected by the user with "pacman -U (INCLUDING DEPENDENCIES)" !
 */
void MainWindow::doInstallLocalPackages()
{
  QString listOfTargets;
  QString list;
  QFileInfo fi;

  m_progressWidget->setRange(0, 100);

  foreach(QString target, m_packagesToInstallList)
  {
    fi.setFile(target);
    list = list + fi.fileName() + "\n";
  }

  foreach(QString pkgToInstall, m_packagesToInstallList)
  {
    fi.setFile(pkgToInstall);
    pkgToInstall = fi.fileName();
    pkgToInstall = pkgToInstall.remove(".xbps");
    int i = pkgToInstall.lastIndexOf(".");
    pkgToInstall = pkgToInstall.mid(0, i);
    listOfTargets += pkgToInstall + " ";
  }

  QString targetPath = fi.path();
  TransactionDialog question(this);
  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);

  if(m_packagesToInstallList.count()==1)
  {
    /*if (m_packagesToInstallList.at(0).indexOf("HoldPkg was found in") != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }*/
    question.setText(StrConstants::getRetrievePackage());
  }
  else
    question.setText(StrConstants::getRetrievePackages(m_packagesToInstallList.count()));

  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    disableTransactionButtons();
    disableTransactionActions();
    clearTabOutput();

    m_xbpsExec = new XBPSExec();
    if (m_debugInfo)
      m_xbpsExec->setDebugMode(true);

    QObject::connect(m_xbpsExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( xbpsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_xbpsExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_xbpsExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_xbpsExec, SIGNAL(commandToExecInQTermWidget(QString)),
                     this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_INSTALL;
      m_xbpsExec->doInstallLocal(targetPath, listOfTargets);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_xbpsExec->doInstallLocalInTerminal(targetPath, listOfTargets);
    }
  }
}

/*
 * Clears the local package cache using "pacman -Sc"
 */
void MainWindow::doCleanCache()
{
  if (!doRemovePacmanLockFile()) return;

  int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                  StrConstants::getCleanCacheConfirmation(),
                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

  if (res == QMessageBox::Yes)
  {
    clearTabOutput();

    m_xbpsExec = new XBPSExec();
    if (m_debugInfo)
      m_xbpsExec->setDebugMode(true);

    QObject::connect(m_xbpsExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( xbpsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_xbpsExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_xbpsExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));

    disableTransactionActions();
    m_commandExecuting = ectn_CLEAN_CACHE;
    m_xbpsExec->doCleanCache();
  }
}

/*
 * Disables all Transaction related actions
 */
void MainWindow::disableTransactionActions()
{
  toggleSystemActions(false);
  toggleTransactionActions(false);
}

/*
 * Enables all Transaction related actions
 */
void MainWindow::enableTransactionActions()
{
  toggleTransactionActions(true);
}

/*
 * Sets with the given boolean the state of all Transaction related actions
 */
void MainWindow::toggleTransactionActions(const bool value)
{
  bool state = isThereAPendingTransaction();
  if (value == true && state == true)
  {
    ui->actionCommit->setEnabled(true);
    ui->actionCancel->setEnabled(true);

    m_actionSwitchToLocalFilter->setEnabled(true);
    m_actionSwitchToRemoteSearch->setEnabled(true);
    ui->actionSyncPackages->setEnabled(false);
    ui->actionSystemUpgrade->setEnabled(false);
  }
  else if (value == true && state == false)
  {
    ui->actionCommit->setEnabled(false);
    ui->actionCancel->setEnabled(false);

    m_actionSwitchToLocalFilter->setEnabled(true);
    m_actionSwitchToRemoteSearch->setEnabled(true);

    ui->actionSyncPackages->setEnabled(true);
    if (value == true && m_outdatedStringList->count() > 0)
      ui->actionSystemUpgrade->setEnabled(true);
  }
  else if (value == false && state == false)
  {
    m_actionSwitchToLocalFilter->setEnabled(false);
    m_actionSwitchToRemoteSearch->setEnabled(false);

    ui->actionSyncPackages->setEnabled(false);
    ui->actionSystemUpgrade->setEnabled(false);
  }

  ui->actionInstall->setEnabled(value);
  m_actionInstallPacmanUpdates->setEnabled(value);
  ui->actionRemoveTransactionItem->setEnabled(value);
  ui->actionRemoveTransactionItems->setEnabled(value);
  ui->actionRemove->setEnabled(value);
  ui->actionPacmanLogViewer->setEnabled(value);
  ui->actionCacheCleaner->setEnabled(value);
  ui->actionCleanXBPSCache->setEnabled(value);
  ui->actionRepositoryEditor->setEnabled(value);  
  m_actionSysInfo->setEnabled(value);
  ui->actionGetNews->setEnabled(value);
  ui->actionOpenRootTerminal->setEnabled(value);
  ui->actionHelpUsage->setEnabled(value);
  ui->actionDonate->setEnabled(value);
  ui->actionHelpAbout->setEnabled(value);
  ui->actionInstallLocalPackage->setEnabled(value);

  //Search menu
  ui->actionSearchByFile->setEnabled(value);
  ui->actionSearchByName->setEnabled(value);
  ui->actionSearchByDescription->setEnabled(value);
  ui->actionUseInstantSearch->setEnabled(value);

  m_actionPackageInfo->setEnabled(value);
  ui->actionFindFileInPackage->setEnabled(value);
  m_leFilterPackage->setEnabled(value);

  disconnect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
  ui->twProperties->setTabEnabled(ctn_TABINDEX_INFORMATION, value);
  ui->twProperties->setTabEnabled(ctn_TABINDEX_FILES, value);
  connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));

  //We have to toggle the combobox groups as well
  if (m_initializationCompleted) ui->twGroups->setEnabled(value);
}

/*
 * Enables / Disables important interface actions
 */
void MainWindow::toggleSystemActions(const bool value)
{
  if (value == true && m_commandExecuting != ectn_NONE) return;

  bool state = isThereAPendingTransaction();

  if (isRemoteSearchSelected() && !state)
  {
    ui->actionSyncPackages->setEnabled(true);
  }
  else if ((value == true && !state) || value == false)
  {
    ui->actionSyncPackages->setEnabled(value);
  }

  ui->actionInstallLocalPackage->setEnabled(value);
  ui->actionCleanXBPSCache->setEnabled(value);
  ui->actionGetNews->setEnabled(value);

  if (value == true && !state && m_outdatedStringList->count() > 0)
    ui->actionSystemUpgrade->setEnabled(true);
  else
    ui->actionSystemUpgrade->setEnabled(false);
}

/*
 * Triggers the especific methods that need to be called given the packages in the transaction
 */
void MainWindow::commitTransaction()
{
  //Are there any remove actions to be commited?
  if(getRemoveTransactionParentItem()->rowCount() > 0 && getInstallTransactionParentItem()->rowCount() > 0)
  {
    doPreRemoveAndInstall();
  }
  else if(getRemoveTransactionParentItem()->rowCount() > 0)
  {
    doRemove();
  }
  else if(getInstallTransactionParentItem()->rowCount() > 0)
  {
    doPreInstall();
  }
}

/*
 * Clears the transaction treeview
 */
void MainWindow::cancelTransaction()
{
  int res = QMessageBox::question(this,
                        StrConstants::getConfirmation(),
                        StrConstants::getCancelTransactionConfirmation(),
                        QMessageBox::Yes|QMessageBox::No,
                        QMessageBox::No);

  if(res == QMessageBox::Yes)
  {
    clearTransactionTreeView();
  }
}

/*
 * This SLOT is called when Pacman's process has finished execution
 *
 */
void MainWindow::xbpsProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  m_progressWidget->close();
  ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName());

  //mate-terminal is returning code 255 sometimes...
  if ((exitCode == 0 || exitCode == 255) && exitStatus == QProcess::NormalExit)
  {
    //First, we empty the tabs cache!
    m_cachedPackageInInfo = "";
    m_cachedPackageInFiles = "";

    writeToTabOutput("<br><b>" +
                     StrConstants::getCommandFinishedOK() + "</b><br>");
  }
  else
  {
    writeToTabOutput("<br><b>" +
                     StrConstants::getCommandFinishedWithErrors() + "</b><br>");
  }

  if(m_commandQueued == ectn_INSTALL)
  {
    m_commandQueued = ectn_NONE;
    //Let's first remove all remove targets...
    removePackagesFromRemoveTransaction();
    doPreInstall();
    return;
  }
  else if (m_commandQueued == ectn_NONE)
  {
    if(exitCode == 0 || exitCode == 255) //mate-terminal is returning code 255 sometimes...
    {
      clearTransactionTreeView();

      if (m_commandExecuting == ectn_CLEAN_CACHE)
      {
        resetTransaction();
      }
      //After the command, we can refresh the package list, so any change can be seem.
      else if (m_commandExecuting == ectn_SYNC_DATABASE)
      {
        //Retrieves the RSS News from respective Distro site...
        if (isRemoteSearchSelected())
        {
          m_leFilterPackage->clear();
          m_actionSwitchToRemoteSearch->setChecked(false);
          m_actionSwitchToLocalFilter->setChecked(true);
          refreshDistroNews(true, false);
          m_commandExecuting = ectn_LOCAL_PKG_REFRESH;
          remoteSearchClicked();
        }
        else
        {
          m_leFilterPackage->clear();
          refreshDistroNews(true, false);
          metaBuildPackageList();
          connect(this, SIGNAL(buildPackageListDone()), this, SLOT(resetTransaction()));
        }
      }
      else if (m_commandExecuting == ectn_SYSTEM_UPGRADE ||
               m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
      {
        if (isRemoteSearchSelected())
        {
          //bRefreshGroups = false;
          m_leFilterPackage->clear();
          m_actionSwitchToRemoteSearch->setChecked(false);
          m_actionSwitchToLocalFilter->setChecked(true);
          m_commandExecuting = ectn_LOCAL_PKG_REFRESH;
          remoteSearchClicked();
        }
        else
        {
          m_leFilterPackage->clear();
          metaBuildPackageList();
          connect(this, SIGNAL(buildPackageListDone()), this, SLOT(resetTransaction()));
        }
      }
      else //For any other command...
      {
        m_leFilterPackage->clear();
        metaBuildPackageList();
        connect(this, SIGNAL(buildPackageListDone()), this, SLOT(resetTransaction()));
      }
    }
  }

  if (exitCode != 0)
  {
    resetTransaction();
  }
}

/*
 * THIS IS THE COUNTERPART OF "xbpsProcessFinished" FOR QTERMWIDGET AUR COMMANDS
 * Whenever the terminal transaction has finished, we can update the UI
 */
void MainWindow::onPressAnyKeyToContinue()
{
  if (m_commandExecuting == ectn_NONE) return;

  m_progressWidget->setValue(0);
  m_progressWidget->show();
  clearTransactionTreeView();

  metaBuildPackageList();
  enableTransactionActions();

  if (m_xbpsExec != nullptr)
    delete m_xbpsExec;

  m_commandExecuting = ectn_NONE;
  m_console->execute("");
  m_console->setFocus();

  if (m_cic)
  {
    delete m_cic;
    m_cic = nullptr;
  }
}

/*
 * Whenever a user strikes Ctrl+C, Ctrl+D or Ctrl+Z in the terminal
 */
void MainWindow::onCancelControlKey()
{
  if (m_commandExecuting != ectn_NONE)
  {
    clearTransactionTreeView();
    enableTransactionActions();

    if (m_xbpsExec != nullptr)
      delete m_xbpsExec;

    m_xbpsExec = nullptr;
    m_commandExecuting = ectn_NONE;
  }
}

/*
 * Garbage collects some transaction objects and reset others
 */
void MainWindow::resetTransaction()
{
  enableTransactionActions();
  if (m_xbpsExec != nullptr)
  {
    delete m_xbpsExec;
  }
  m_commandExecuting = ectn_NONE;
  disconnect(this, SIGNAL(buildPackageListDone()), this, SLOT(resetTransaction()));
}

/*
 * Searches the given msg for a series of verbs that a Pacman transaction may produce
 */
bool MainWindow::searchForKeyVerbs(const QString &msg)
{
  return (msg.contains(QRegularExpression("Fetching ")) ||
          msg.contains(QRegularExpression("Updating ")) ||
          msg.contains(QRegularExpression("Processing ")));
}

/*
 * A helper method which writes the given string to OutputTab's textbrowser
 */
void MainWindow::writeToTabOutput(const QString &msg, TreatURLLinks treatURLLinks)
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");

  if (text)
  {
    ensureTabVisible(ctn_TABINDEX_OUTPUT);
    utils::writeToTextBrowser(text, msg, treatURLLinks);
  }
}

/*
 * Sets new percentage value to the progressbar
 */
void MainWindow::incrementPercentage(int percentage)
{
  if (!m_progressWidget->isVisible()) m_progressWidget->show();

  m_progressWidget->setValue(percentage);
}

/*
 * A helper method which writes the given string to OutputTab's textbrowser
 */
void MainWindow::outputText(const QString &output)
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    ensureTabVisible(ctn_TABINDEX_OUTPUT);
    positionTextEditCursorAtEnd();

    text->insertHtml(output);
    text->ensureCursorVisible();
  }
}
