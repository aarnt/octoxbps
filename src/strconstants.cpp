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

#include "strconstants.h"

#include <QObject>
#include <QString>

QString StrConstants::getApplicationName(){
  return "OctoXBPS";
}

QString StrConstants::getApplicationVersion(){
  return "0.3.4";
}

QString StrConstants::getQtVersion(){
  return "Qt " + QString(QT_VERSION_STR);
}

QString StrConstants::getApplicationCliHelp(){
  QString str =
      "\n" + QObject::tr("OctoXBPS usage help:") + "\n\n" +
      "-version: " + QObject::tr("show application version."); /*+ "\n" +
        //"-removecmd <Remove-command>: " + QObject::tr("use a different remove command (ex: -removecmd R).") + "\n" +
        "-sysupgrade: " + QObject::tr("force a system upgrade at startup.") + "\n";*/

  return str;
}

QString StrConstants::getAll(){
  return QObject::tr("All");
}

QString StrConstants::getForeignRepositoryName(){
  return "ALL";
}

QString StrConstants::getForeignPkgRepositoryName(){
  return QLatin1String( "NONE" );
}

QString StrConstants::getForeignRepositoryToolName()
{
  bool first=true;
  QString ret;

  if (first)
  {
    ret = QLatin1String( "NONE" );

    first = false;
  }

  return ret;
}

QString StrConstants::getTridentNews(){
  return QObject::tr("Project Trident news");
}

QString StrConstants::getVoidNews(){
  return QObject::tr("Void Linux news");
}

QString StrConstants::getNewsErrorMessage(){
  return QObject::tr("No news could be found! Press Ctrl+G to download the latest news.");
}

QString StrConstants::getIncompatibleDistroError(){
  return QObject::tr("This distro seems to be incompatible with OctoXBPS!");
}

QString StrConstants::getInternetUnavailableError(){
  return QObject::tr("Internet seems unavailable!");
}

/*QString StrConstants::getDisplayAllCategories(){
  return QObject::tr("Display all categories");
}*/

QString StrConstants::getForeignToolGroup(){
  /*QString StrConstants::tool = getForeignRepositoryToolName();
    tool[0] = tool[0].toUpper();
    tool = "<" + tool + ">";
    return tool.toLatin1();*/

  return "NONE";
}

QString StrConstants::getHelpUsage(){
  return QObject::tr("Usage");
}

QString StrConstants::getHelpAbout(){
  return QObject::tr("About");
}

QString StrConstants::getName(){
  return QObject::tr("Name");
}

QString StrConstants::getVersion(){
  return QObject::tr("Version");
}

QString StrConstants::getInstalledOn(){
  return QObject::tr("Installed on");
}

QString StrConstants::getOutdatedVersion(){
  return QObject::tr("Outdated version");
}

QString StrConstants::getAvailableVersion(){
  return QObject::tr("Available version");
}

QString StrConstants::getNoDescriptionAvailabe(){
  return QObject::tr("No description available.");
}

QString StrConstants::getURL(){
  return QObject::tr("URL");
}

QString StrConstants::getOrigin(){
  return QObject::tr("Origin");
}

QString StrConstants::getLicenses(){
  return QObject::tr("Licenses");
}

QString StrConstants::getCategory(){
  return QObject::tr("Category");
}

QString StrConstants::getCategories(){
  return QObject::tr("Categories");
}

QString StrConstants::getProvides(){
  return QObject::tr("Provides");
}

QString StrConstants::getDependencies(){
  return QObject::tr("Dependencies");
}

/*QString StrConstants::getDependsOn(){
  return QObject::tr("Depends On");
}

QString StrConstants::getRequiredBy(){
  return QObject::tr("Required By");
}

QString StrConstants::getOptionalFor(){
  return QObject::tr("Optional For");
}

QString StrConstants::getOptionalDeps(){
  return QObject::tr("Optional Deps");
}

QString StrConstants::getConflictsWith(){
  return QObject::tr("Conflicts With");
}

QString StrConstants::getReplaces(){
  return QObject::tr("Replaces");
}

QString StrConstants::getPopularityHeader(){
  return QObject::tr("Popularity");
}*/

QString StrConstants::getOptions(){
  return QObject::tr("Options");
}

/*QString StrConstants::getVotes(){
  return QObject::tr("votes");
}*/

QString StrConstants::getSize(){
  return QObject::tr("Size");
}

QString StrConstants::getDownloadSize(){
  return QObject::tr("Download Size");
}

QString StrConstants::getInstalledSize(){
  return QObject::tr("Installed Size");
}

QString StrConstants::getPackager(){
  return QObject::tr("Packager");
}

QString StrConstants::getMaintainer(){
  return QObject::tr("Maintainer");
}

QString StrConstants::getPackage(){
  return QObject::tr("Package");
}

QString StrConstants::getPackages(){
  return QObject::tr("Packages");
}

QString StrConstants::getArchitecture(){
  return QObject::tr("Architecture");
}

QString StrConstants::getBuildDate(){
  return QObject::tr("Build date");
}

QString StrConstants::getInstallDate(){
  return QObject::tr("Install date");
}

QString StrConstants::getDescription(){
  return QObject::tr("Description");
}

QString StrConstants::getAttention(){
  return QObject::tr("Attention");
}

QString StrConstants::getAutomaticSuCommand(){
  return QObject::tr("automatic");
}

QString StrConstants::getPassword(){
  return QObject::tr("Password");
}

QString StrConstants::getTabInfoName(){
  return QObject::tr("Info");
}

QString StrConstants::getTabFilesName(){
  return QObject::tr("Files");
}

QString StrConstants::getTabTransactionName(){
  return QObject::tr("Actions");
}

QString StrConstants::getTabOutputName(){
  return QObject::tr("Output");
}

QString StrConstants::getTabNewsName(){
  return QObject::tr("News");
}

QString StrConstants::getTabTerminal()
{
  return QObject::tr("Terminal");
}

QString StrConstants::getContentsOf(){
  return QObject::tr("Contents of \"%1\"");
}

QString StrConstants::getFind(){
  return QObject::tr("Find");
}

QString StrConstants::getClear(){
  return QObject::tr("Clear");
}

QString StrConstants::getOutdatedInstalledVersion(){
  return QObject::tr(" (outdated installed version is %1)");
}

QString StrConstants::getNewerInstalledVersion(){
  return QObject::tr(" (newer installed version is %1)");
}

QString StrConstants::getBuildingPackageList(){
  return QObject::tr("Building package list...");
}

QString StrConstants::getSearchingForBSDNews(){
  return QObject::tr("Searching for %1 latest news...");
}

QString StrConstants::getOneOutdatedPackage(){
  return QObject::tr("There is one outdated package in your system:");
}

QString StrConstants::getOutdatedPackages(int outdatedPackagesCount){
  return QObject::tr("There are %n outdated packages in your system:", 0, outdatedPackagesCount);
}

QString StrConstants::getNewVersionAvailable(){
  return QObject::tr("(version %1 is available)");
}

QString StrConstants::getTotalPackages(int packageCount){
  return QObject::tr("%n packages", 0, packageCount);
}

QString StrConstants::getSelectedPackages(int selectedCount){
  return QObject::tr("%n selected", 0, selectedCount);
}

QString StrConstants::getNumberInstalledPackages(int installedPackagesCount){
  return QObject::tr("%n installed", 0, installedPackagesCount);
}

QString StrConstants::getNumberOutdatedPackages(int outdatedPackagesCount){
  return QObject::tr("%n outdated", 0, outdatedPackagesCount);
}

QString StrConstants::getNumberAvailablePackages(int availablePackagesCount){
  return QObject::tr("%n available", 0, availablePackagesCount);
}

QString StrConstants::getCleaningPackageCache(){
  return QObject::tr("Cleaning package cache...");
}

/*QString StrConstants::getRemovingPacmanTransactionLockFile(){
  return QObject::tr("Removing Pacman's transaction lock file...");
}*/

#ifdef UNIFIED_SEARCH
QString StrConstants::getLineEditTextLocal(){
  return QObject::tr("Filter packages");
}
#else
QString StrConstants::getLineEditTextLocal(){
  return QObject::tr("Filter installed packages");
}
#endif

QString StrConstants::getLineEditTextRemote(){
  return QObject::tr("Enter your search and press ENTER");
}

QString StrConstants::getRemotePackageSearchTip(){
  return QObject::tr("Press ENTER to start your search");
}

QString StrConstants::getSyncing(){
  return QObject::tr("Syncing");
}

QString StrConstants::getPressAnyKey(){
  return QObject::tr("Press any key to continue...");
}

QString StrConstants::getSyncDatabase(){
  return QObject::tr("Sync database");
}

QString StrConstants::getSyncDatabases(){
  return QObject::tr("Synchronizing databases...");
}

QString StrConstants::getIsUpToDate(){
  return QObject::tr("is up to date");
}

QString StrConstants::getSystemUpgrade(){
  return QObject::tr("Installing updates...");
}

QString StrConstants::getInstallingPackages(){
  return QObject::tr("Installing selected packages...");
}

QString StrConstants::getRemovingPackages(){
  return QObject::tr("Removing selected packages...");
}

QString StrConstants::getRemovingAndInstallingPackages(){
  return QObject::tr("Removing/installing selected packages...");
}

QString StrConstants::getChooseATerminal(){
  return QObject::tr("Choose a terminal");
}

QString StrConstants::getRunningCommandInTerminal(){
  return QObject::tr("Running command in terminal...");
}

QString StrConstants::getCommandFinishedOK(){
  return QObject::tr("Command finished OK!");
}

QString StrConstants::getCommandFinishedWithErrors(){
  return QObject::tr("Command finished with errors!");
}

QString StrConstants::geRetrievingPackage(){
  return QObject::tr("Retrieving %1");
}

QString StrConstants::getTotalDownloadSize(){
  return QObject::tr("Total download size: %1 KB");
}

QString StrConstants::getRetrievePackage(){
  return QObject::tr("The following package needs to be retrieved");
}

QString StrConstants::getRemovePackage(){
  return QObject::tr("The following package will be removed");
}

QString StrConstants::getRetrievePackages(int retrievePackagesCount){
  return QObject::tr("The following %n packages need to be retrieved", 0, retrievePackagesCount);
}

QString StrConstants::getRemovePackages(int removePackagesCount){
  return QObject::tr("The following %n packages will be removed", 0, removePackagesCount);
}

/*QString StrConstants::getWarnHoldPkgFound() {
  return QObject::tr("There are forbidden packages in the removal list!");
}*/

QString StrConstants::getWarnTransactionAborted() {
  return QObject::tr("Transaction aborted due to unresolved dependencies.");
}

QString StrConstants::getNoNewUpdatesAvailable(){
  return QObject::tr("There are no new updates available!");
}

QString StrConstants::getOneNewUpdate(){
  return QObject::tr("There is an update available!");
}

QString StrConstants::getNewUpdates(int newUpdatesCount){
  return QObject::tr("There are %n updates available!", 0, newUpdatesCount);
}

QString StrConstants::getConfirmationQuestion(){
  return QObject::tr("Confirm?");
}

QString StrConstants::getWarning(){
  return QObject::tr("Warning!!!");
}

QString StrConstants::getConfirmation(){
  return QObject::tr("Confirmation");
}

QString StrConstants::getThereHasBeenATransactionError(){
  return QObject::tr("There has been an action error!");
}

QString StrConstants::getConfirmExecuteTransactionInTerminal(){
  return QObject::tr("Do you want to execute this action in a Terminal?");
}

QString StrConstants::getCleanCacheConfirmation(){
  return QObject::tr("Do you really want to clean the package cache?");
}

/*QString StrConstants::getRemovePacmanTransactionLockFileConfirmation(){
  return QObject::tr("Do you really want to remove Pacman's transaction lock file?");
}*/

QString StrConstants::getCancelTransactionConfirmation(){
  return QObject::tr("Do you really want to cancel the actions?");
}

QString StrConstants::getPkgNotAvailable(){
  return QObject::tr("This package is not available in the database!");
}

QString StrConstants::getEnterAdministratorsPassword(){
  return QObject::tr(
        "Please, enter the administrator's password");
}

QString StrConstants::getErrorNoSuCommand(){
  return
      QObject::tr("There are no means to get administrator's credentials.");
}

QString StrConstants::getYoullNeedSuFrontend(){
  return QObject::tr("You'll need to install a su frontend like gksu or kdesu.");
}

QString StrConstants::getErrorBinaryXNotFound(const QString &binName){
  return QObject::tr("'%1' binary was not found.").arg(binName);
}

QString StrConstants::getErrorRunningWithRoot(){
  return QObject::tr("You can not run OctoXBPS with administrator's credentials.");
}

QString StrConstants::getThereIsARunningTransaction(){
  return QObject::tr("Canceling the running transaction may damage your system!");
}

QString StrConstants::getThereIsAPendingTransaction(){
  return QObject::tr("There is a pending action");
}

QString StrConstants::getDoYouReallyWantToQuit(){
  return QObject::tr("Do you really want to quit?");
}

QString StrConstants::getExecutingCommand(){
  return QObject::tr("Executing command");
}

QString StrConstants::getRunInTerminal(){
  return QObject::tr("Run in terminal");
}

QString StrConstants::getZoomIn()
{
  return QObject::tr("Zoom in");
}

QString StrConstants::getZoomOut()
{
  return QObject::tr("Zoom out");
}

QString StrConstants::getMaximize()
{
  return QObject::tr("Maximize");
}

QString StrConstants::getCopy()
{
  return QObject::tr("Copy");
}

QString StrConstants::getPaste()
{
  return QObject::tr("Paste");
}

/*QString StrConstants::getNeedsAppRestart(){
  return QObject::tr("Needs application restart to take effect");
}

QString StrConstants::getWarnNeedsAppRestart(){
  return QObject::tr("These changes need application restart to take effect!");
}*/

QString StrConstants::getFileChooserTitle(){
  return QObject::tr("Select the packages you want to install");
}

QString StrConstants::getThisIsNotATextFile(){
  return QObject::tr("This file does not appear to be a simple text.\n"
                     "Are you sure you want to open it?");
}

QString StrConstants::getTransactionInstallText(){
  return QObject::tr("To be installed");
}

QString StrConstants::getTransactionRemoveText(){
  return QObject::tr("To be removed");
}

QString StrConstants::getRemove(){
  return QObject::tr("remove");
}

QString StrConstants::getInstall(){
  return QObject::tr("install");
}

QString StrConstants::getRemoveItem(){
  return QObject::tr("Remove item");
}

QString StrConstants::getRemoveItems(){
  return QObject::tr("Remove items");
}

/*QString StrConstants::getPressCtrlAToSelectAll(){
  return QObject::tr("Press Ctrl+A to select/deselect all");
}*/

/*QString StrConstants::getUseAURTool(){
  return QObject::tr("Use \"%1\" tool").arg(getForeignRepositoryToolName());
}*/

QString StrConstants::getFilterLocalPackages(){
  return QObject::tr("Filter installed packages");
}

QString StrConstants::getSearchForPackages(){
  return QObject::tr("Search for remote packages");
}

QString StrConstants::getSearchStringIsShort(){
  return QObject::tr("Search string must have at least 2 characters.");
}

QString StrConstants::getCopyFullPath(){
  return QObject::tr("Copy path to clipboard");
}

QString StrConstants::getNotifierOptionsDialogTitle(){
  return QObject::tr("Options");
}

QString StrConstants::getNotiferSetupDialogGroupBoxTitle(){
  return QObject::tr("Sync database interval");
}

QString StrConstants::getOnceADay(){
  return QObject::tr("Once a day");
}

QString StrConstants::getOnceADayAt(){
  return QObject::tr("Once a day, at");
}

QString StrConstants::getOnceADayAtDesc(){
  return QObject::tr("(value in 24-hour format: 0 to 23)");
}

QString StrConstants::getOnceEvery(){
  return QObject::tr("Once every");
}

QString StrConstants::getOnceEveryDesc(){
  return QObject::tr("(value in minutes: %1 to %2)");
}

QString StrConstants::getNever(){
  return QObject::tr("Never");
}

QString StrConstants::getSetInterval(){
  return QObject::tr("Set interval...");
}

QString StrConstants::getUpgrading()
{
  return QObject::tr("Upgrading...");
}

//Style Sheets ---------------------------------

QString StrConstants::getToolBarCSS(){
  return QString("QToolBar { border: 5px; } "
                 "QToolTip {}");
}

QString StrConstants::getFilterPackageNotFoundCSS(){
  return QString("QLineEdit{ color: white; "
                 "background-color: rgb(207, 135, 142);"
                 "border-color: rgb(206, 204, 197);}"
                 );
}

QString StrConstants::getFilterPackageFoundCSS(){
  return QString("QLineEdit, SearchLineEdit{ color: black; "
                 "background-color: rgb(255, 255, 200);"
                 "border-color: rgb(206, 204, 197);}"
                 );
}

QString StrConstants::getDockWidgetTitleCSS(){
  return QString("QDockWidget::title { "
                 "text-align: right;"
                 "background: transparent;"
                 "padding-right: 5px;}"
                 );
}

QString StrConstants::getTabBarCSS(){
  return QString("QTabBar::close-button {"
                 "image: url(:/resources/images/window-close.png);"
                 "border-radius: 4px}"
                 "QTabBar::close-button:hover {"
                 "background-color: palette(light)}"
                 "QTabBar::close-button:pressed {"
                 "background-color: palette(mid)}"
                 );
}

QString StrConstants::getTreeViewCSS(){
  return QString("QTreeView::branch:has-siblings:!adjoins-item {"
                 "   border-image: url(:/resources/styles/vline.png) 0;}"
                 "QTreeView::branch:has-siblings:adjoins-item {"
                 "    border-image: url(:/resources/styles/branch-more.png) 0;}"
                 "QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
                 "   border-image: url(:/resources/styles/branch-end.png) 0;}"
                 "QTreeView::branch:has-children:!has-siblings:closed,"
                 "QTreeView::branch:closed:has-children:has-siblings {"
                 "       border-image: none;"
                 "        image: url(:/resources/styles/branch-closed_BW.png);}"
                 "QTreeView::branch:open:has-children:!has-siblings,"
                 "QTreeView::branch:open:has-children:has-siblings  {"
                 "       border-image: none;"
                 "       image: url(:/resources/styles/branch-open_BW.png);}");
}

QString StrConstants::getSysUpgrade()
{
  return QObject::tr("System upgrade");
}

QString StrConstants::getExit()
{
  return QObject::tr("Exit");
}
