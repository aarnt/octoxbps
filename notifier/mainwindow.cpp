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
#include "optionsdialog.h"
#include "outputdialog.h"
#include "../src/strconstants.h"
#include "../src/uihelper.h"
#include "../src/package.h"
#include "../src/transactiondialog.h"

#include <QTimer>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QProcess>
#include <QMessageBox>
#include <QScreen>
#include <QDebug>

#ifdef KSTATUS
  #include <kstatusnotifieritem.h>
#endif

/*
 * This is OctoXBPS Notifier slim interface code :-)
 */

/*
 * The obligatory constructor...
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
  m_transactionDialog = nullptr;
  m_debugInfo = false;
  m_optionsDialog = nullptr;
  m_xbpsDatabaseSystemWatcher =
            new QFileSystemWatcher(QStringList() << ctn_XBPS_DATABASE_DIR, this);

  connect(m_xbpsDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  initSystemTrayIcon();
}

MainWindow::~MainWindow()
{
#ifdef KSTATUS
  delete m_systemTrayIcon;
#endif
}

/*
 * Changes tooltip of systray component to indicate system is upgrading
 */
void MainWindow::setUpgradingTooltip()
{
#ifdef KSTATUS
  m_systemTrayIcon->setToolTipSubTitle(StrConstants::getUpgrading());
#else
  m_systemTrayIcon->setToolTip(StrConstants::getUpgrading());
#endif
}

/*
 * Let's initialize the system tray object...
 */
void MainWindow::initSystemTrayIcon()
{
  if (m_debugInfo)
    qDebug() << "At initSystemTrayIcon()...";

  m_commandExecuting = ectn_NONE;
  m_outdatedStringList = new QMap<QString, OutdatedPackageInfo>();

#ifdef KSTATUS
  m_systemTrayIcon = new KStatusNotifierItem(0);
#else
  m_systemTrayIcon = new QSystemTrayIcon(this);
#endif

  m_systemTrayIcon->setObjectName("systemTrayIcon");

#ifdef KSTATUS
  m_systemTrayIcon->setIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipIconByPixmap(m_icon);
  m_systemTrayIcon->setTitle("OctoXBPS Notifier");
#else
  m_systemTrayIcon->setIcon(m_icon); 
#endif

  m_actionExit = new QAction(IconHelper::getIconExit(), tr("Exit"), this);
  connect(m_actionExit, SIGNAL(triggered()), this, SLOT(exitNotifier()));

  m_actionAbout = new QAction(StrConstants::getHelpAbout(), this);
  m_actionAbout->setIconVisibleInMenu(true);
  connect(m_actionAbout, SIGNAL(triggered()), this, SLOT(aboutOctoXBPSNotifier()));

  m_actionOctoXBPS = new QAction(this);
  m_actionOctoXBPS->setText("OctoXBPS ...");
  connect(m_actionOctoXBPS, SIGNAL(triggered()), this, SLOT(startOctoXBPS()));

  m_actionOptions = new QAction(this);
  m_actionOptions->setText(StrConstants::getOptions());
  connect(m_actionOptions, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));

  m_actionSyncDatabase = new QAction(this);
  m_actionSyncDatabase->setIconVisibleInMenu(true);
  m_actionSyncDatabase->setText(StrConstants::getSyncDatabase());
  m_actionSyncDatabase->setIcon(IconHelper::getIconSyncDatabase());
  connect(m_actionSyncDatabase, SIGNAL(triggered()), this, SLOT(syncDatabase()));

  m_actionSystemUpgrade = new QAction(this);
  m_actionSystemUpgrade->setIconVisibleInMenu(true);
  m_actionSystemUpgrade->setText(tr("System upgrade"));
  m_actionSystemUpgrade->setIcon(IconHelper::getIconSystemUpgrade());
  connect(m_actionSystemUpgrade, SIGNAL(triggered()), this, SLOT(runOctoXBPSSysUpgrade()));

  m_systemTrayIconMenu = new QMenu( this );

  if (UnixCommand::hasTheExecutable("octoxbps"))
    m_systemTrayIconMenu->addAction(m_actionOctoXBPS);

  m_systemTrayIconMenu->addAction(m_actionSyncDatabase);
  m_systemTrayIconMenu->addAction(m_actionSystemUpgrade);
  m_systemTrayIconMenu->addAction(m_actionOptions);
  m_systemTrayIconMenu->addSeparator();
  m_systemTrayIconMenu->addAction(m_actionAbout);
  m_systemTrayIconMenu->addAction(m_actionExit);
  m_systemTrayIcon->setContextMenu(m_systemTrayIconMenu);

  // disable "standard" actions (restore & quit)
#ifdef KSTATUS
  m_systemTrayIcon->setStandardActionsEnabled(false);
  connect (m_systemTrayIcon, SIGNAL(activateRequested(bool,QPoint)),
           this, SLOT(execSystemTrayKF5()) );
#else
  connect ( m_systemTrayIcon , SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
            this, SLOT( execSystemTrayActivated ( QSystemTrayIcon::ActivationReason ) ) );
#endif

  m_notifierTimer = new QTimer();
  m_notifierTimer->setInterval(1000);
  m_notifierTimer->start();

  connect(m_notifierTimer, SIGNAL(timeout()), this, SLOT(notifierTimerTimeout()));
}

/*
 * Whenever this timer ticks, we need to call the PacmanHelper DBus interface to sync Pacman's dbs
 */
void MainWindow::notifierTimerTimeout()
{
  static bool firstTime=true;

  if (!UnixCommand::hasInternetConnection() || m_commandExecuting != ectn_NONE) return;

  if (firstTime)
  {
    refreshAppIcon();

#ifdef KSTATUS
    m_systemTrayIcon->setToolTipTitle("OctoXBPS Notifier");
#else
    m_systemTrayIcon->show();
#endif

    //From now on, we verify if it's time to check for updates every 5 minutes
    m_notifierTimer->setInterval(60000 * 5);
    setWindowIcon(m_icon);
    firstTime=false;
  }

  //Is it time to syncdb again?
  QDateTime lastCheckTime = SettingsManager::getLastSyncDbTime();
  int syncDbInterval = SettingsManager::getSyncDbInterval();
  QDateTime now = QDateTime::currentDateTime();
  bool syncTime = false;
  int syncHour = SettingsManager::getSyncDbHour();

  //User did not set the check interval, so we assume it's once a day
  if (syncDbInterval == -1)
  {
    if (syncHour >= 0) //Once a day at a certain time?
    {
      if (m_debugInfo)
        qDebug() << "SyncDb is scheduled once a day, at " << syncHour << " hours";

      if (lastCheckTime.daysTo(now) >= 1 && now.time().hour() == syncHour)
      {
        syncTime = true;
      }
    }
    else
    {
      if (m_debugInfo)
        qDebug() << "SyncDb is scheduled once a day";
    }

    if ((syncHour == -1 && (
           lastCheckTime.isNull() ||
           lastCheckTime.daysTo(now) >= 1)) || (syncTime))
    {
      syncDatabase();
      //Then we set new LastCheckTime...
      SettingsManager::setLastSyncDbTime(now);
    }
  }
  else if (syncDbInterval != -2) //Because if it's "-2" user does NOT want any checkupdates!
  {
    if (lastCheckTime.isNull() || now.addSecs(-(syncDbInterval * 60)) >= lastCheckTime)
    {
      syncDatabase();
      //Then we set new LastCheckTime...
      SettingsManager::setLastSyncDbTime(now);
    }
    else
    {
      if (m_debugInfo)
        qDebug() << "SyncDb is scheduled once every " << syncDbInterval << " minutes.";
    }

    m_notifierTimer->stop();
    m_notifierTimer->start();
  }
}

/*
 * Helper to a runOctoXBPS with a call to SystemUpgrade
 */
void MainWindow::runOctoXBPSSysUpgrade()
{
  if (UnixCommand::isAppRunning("octoxbps", true))
    runOctoXBPS(ectn_SYSUPGRADE_EXEC_OPT);
  else
    doSystemUpgrade();
}

/*
 * Shows OctoXBPS About Dialog...
 */
void MainWindow::aboutOctoXBPSNotifier()
{
  m_actionAbout->setEnabled(false);

  //First we create a fake window to act as about dialog's parent
  //Otherwise the dialog appears at a random screen point!
  QMainWindow *fake = new QMainWindow();
  fake->setWindowIcon(m_icon);
  fake->setVisible(false);
  QScreen *sc = QGuiApplication::primaryScreen();
  fake->setGeometry(sc->geometry());

  QString aboutText =
      QLatin1String("<b>OctoXBPS Notifier</b><br>");
  aboutText += StrConstants::getVersion() + QLatin1String(": ") + StrConstants::getApplicationVersion() + QLatin1String(" - ") + StrConstants::getQtVersion() + QLatin1String("<br>");
  aboutText += StrConstants::getURL() + QLatin1String(": ") +
    QStringLiteral("<a href=\"https://github.com/aarnt/octoxbps/\">https://github.com/aarnt/octoxbps</a><br>");
  aboutText += StrConstants::getLicenses() + QLatin1String(": ") + QStringLiteral("<a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GPL v2</a><br>");
  aboutText += QStringLiteral("&copy; Alexandre Albuquerque Arnt<br><br>");
  aboutText += QStringLiteral("<b>XBPS</b><br>");
  aboutText += StrConstants::getVersion() + QLatin1String(": ") + UnixCommand::getXBPSVersion() + QStringLiteral("<br>");
  aboutText += StrConstants::getURL() + QLatin1String(": ") +
    QStringLiteral("<a href=\"https://github.com/void-linux/xbps/\">https://github.com/void-linux/xbps</a><br>");
  aboutText += QStringLiteral("&copy; Juan Romero Pardines & Void Linux team");

  QMessageBox::about(fake, StrConstants::getHelpAbout(), aboutText);

  delete fake;
  m_actionAbout->setEnabled(true);
}

/*
 * Hides OctoXBPS
 */
void MainWindow::hideOctoXBPS()
{  
  QProcess::startDetached("octoxbps -hide");
}

/*
 * Checks if some SU utility is available...
 * Returns false if not!
 */
bool MainWindow::_isSUAvailable()
{
  //If there are no means to run the actions, we must warn!
  if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND)
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(),
                           StrConstants::getErrorNoSuCommand() +
                           "\n" + StrConstants::getYoullNeedSuFrontend());
    return false;
  }
  else
    return true;
}

/*
 * Calls only the OctoXBPS system upgrade window
 */
void MainWindow::doSystemUpgrade()
{
  if (m_transactionDialog != nullptr)
  {
    if (m_transactionDialog->isMinimized())
      m_transactionDialog->setWindowState(Qt::WindowNoState);
    else
      m_transactionDialog->activateWindow();
    return;
  }

  //Shows a dialog indicating the targets needed to be retrieved and asks for the user's permission.
  TransactionInfo ti = Package::getTargetUpgradeList();
  QStringList *targets = ti.packages;

  //There are no new updates to install!
  if (targets->count() == 0 && m_outdatedStringList->count() == 0)
  {
    return;
  }
  else if (targets->count() == 0 && m_outdatedStringList->count() > 0)
  {
    return;
  }

  QString list;
  bool upgradeXBPS=false;
  foreach(QString target, *targets)
  {
    if (target == "xbps") upgradeXBPS=true;
    list = list + target + "\n";
  }

  QString ds = ti.sizeToDownload;
  m_transactionDialog = new TransactionDialog(this);

  if(targets->count()==1)
    m_transactionDialog->setText(StrConstants::getRetrievePackage() +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));
  else
    m_transactionDialog->setText(StrConstants::getRetrievePackages(targets->count()) +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));

  m_transactionDialog->setWindowTitle(StrConstants::getConfirmation());
  m_transactionDialog->setInformativeText(StrConstants::getConfirmationQuestion());
  m_transactionDialog->setDetailedText(list);

  m_systemUpgradeDialog = true;
  int result = m_transactionDialog->exec();
  m_transactionDialog = nullptr;

  if (result == QDialogButtonBox::Yes)
  {
    m_commandExecuting = ectn_SYSTEM_UPGRADE;

    m_systemUpgradeDialog = false;
    toggleEnableInterface(false);
    m_actionSystemUpgrade->setEnabled(false);

    OutputDialog *dlg = new OutputDialog(this);
    dlg->setViewAsTextBrowser(true);
    dlg->setUpgradeXBPS(upgradeXBPS);
    if (m_debugInfo)
      dlg->setDebugMode(true);

    QObject::connect(dlg, SIGNAL( finished(int)),
                     this, SLOT( doSystemUpgradeFinished() ));
    setUpgradingTooltip();
    dlg->show();
    dlg->doSystemUpgrade();
  }
  else if(result == QDialogButtonBox::AcceptRole)
  {
    m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;

    m_systemUpgradeDialog = false;
    toggleEnableInterface(false);
    m_actionSystemUpgrade->setEnabled(false);

    OutputDialog *dlg = new OutputDialog(this);
    dlg->setViewAsTextBrowser(false);
    dlg->setUpgradeXBPS(upgradeXBPS);

    QObject::connect(dlg, SIGNAL( finished(int)),
                     this, SLOT( doSystemUpgradeFinished() ));
    setUpgradingTooltip();
    dlg->show();
    dlg->doSystemUpgradeInTerminal();
  }
  else if (result == QDialogButtonBox::No)
  {   
    m_systemUpgradeDialog = false;
    toggleEnableInterface(true);
    refreshAppIcon();
  }
}

/*
 * When system upgrade process is finished...
 */
void MainWindow::doSystemUpgradeFinished()
{
  m_commandExecuting = ectn_NONE;
  refreshAppIcon();

  //Does it still need to upgrade another packages due to SyncFirst issues???
  if ((m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
      && m_outdatedStringList->count() > 0)
  {
    m_commandExecuting = ectn_NONE;
    m_unixCommand->removeTemporaryFile();

    doSystemUpgrade();

    return;
  }

  m_unixCommand->removeTemporaryFile();
  toggleEnableInterface(true);
}

/*
 * Enables and Disables some UI elements of OctoXBPS-notifier
 */
void MainWindow::toggleEnableInterface(bool state)
{
  m_actionOctoXBPS->setEnabled(state);
  m_actionSyncDatabase->setEnabled(state);
  m_actionOptions->setEnabled(state);
  m_actionExit->setEnabled(state);
}

/*
 * Called right after the pkexec xbps-install -Syy command has finished!
 */
void MainWindow::finishedPkexec(int)
{
  if (m_debugInfo)
    qDebug() << "At finishedPkexec()...";

  toggleEnableInterface(true);

#ifndef KSTATUS
  m_systemTrayIcon->setContextMenu(m_systemTrayIconMenu);
  m_systemTrayIconMenu->close();
#endif

  m_commandExecuting = ectn_NONE;

  int numberOfOutdatedPackages = m_numberOfOutdatedPackages;
  refreshAppIcon();

  if (numberOfOutdatedPackages != m_numberOfOutdatedPackages)
  {
    if (m_numberOfOutdatedPackages > 0)
    {
      QString notification;

      if (m_numberOfOutdatedPackages == 1)
      {
        notification = StrConstants::getOneNewUpdate();

        #ifdef KSTATUS
          m_systemTrayIcon->setToolTipSubTitle(notification);
          m_systemTrayIcon->showMessage("OctoXBPS",
                                        notification, m_systemTrayIcon->iconName());
        #else
          m_systemTrayIcon->setToolTip(notification);
        #endif
      }
      else if (m_numberOfOutdatedPackages > 1)
      {
        notification = StrConstants::getNewUpdates(m_numberOfOutdatedPackages);

        #ifdef KSTATUS
          m_systemTrayIcon->setToolTipSubTitle(notification);
          m_systemTrayIcon->showMessage("OctoXBPS",
                                        notification, m_systemTrayIcon->iconName());
        #else
          m_systemTrayIcon->setToolTip(notification);
        #endif
      }
    }
  }
  else
  {
    QString notification;

    if (numberOfOutdatedPackages == 1)
    {
      notification = StrConstants::getOneNewUpdate();

      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(notification);
        m_systemTrayIcon->showMessage("OctoXBPS",
                                      notification, m_systemTrayIcon->iconName());
      #else
        m_systemTrayIcon->setToolTip(notification);
      #endif
    }
    else if (numberOfOutdatedPackages > 1)
    {
      notification = StrConstants::getNewUpdates(numberOfOutdatedPackages);

      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(notification);
        m_systemTrayIcon->showMessage("OctoXBPS",
                                      notification, m_systemTrayIcon->iconName());
      #else
        m_systemTrayIcon->setToolTip(notification);
      #endif
    }
  }
}

/*
 * Called every time user selects "Sync databases..." menu option
 */
void MainWindow::syncDatabase()
{
  disconnect(m_xbpsDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  QTime now;
  if (m_debugInfo)
    qDebug() << now.currentTime().toString("HH:mm").toLatin1().data() <<  ": At syncDatabase()...";
  toggleEnableInterface(false);
  m_icon = IconHelper::getIconOctopiTransparent();

#ifdef KSTATUS
  m_systemTrayIcon->setIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipSubTitle(StrConstants::getSyncDatabases());
#else
  m_systemTrayIcon->setIcon(m_icon);
  m_systemTrayIcon->setToolTip(StrConstants::getSyncDatabases());
#endif

  qApp->processEvents();
  m_systemTrayIconMenu->close();

#ifndef KSTATUS
  m_systemTrayIcon->setContextMenu(0);
#endif

  m_commandExecuting = ectn_SYNC_DATABASE;
  startPkexec();
}

/*
 * Uses notify-send to send a notification to the systray area
 */
void MainWindow::sendNotification(const QString &msg)
{
  QString processToExec("notify-send");

  if (UnixCommand::hasTheExecutable(processToExec))
  {
    processToExec += " -i /usr/share/icons/OctoXBPS_red.png -t 5000 \"" +
        StrConstants::getApplicationName() + "\"  \"" + msg + "\"";
    QProcess::startDetached(processToExec);
  }
}

void MainWindow::startPkexec()
{
  QProcess *xbps = new QProcess();
  connect(xbps, SIGNAL(finished(int)), this, SLOT(finishedPkexec(int)));
  xbps->start("pkexec /usr/bin/xbps-install -Sy");
}

/*
 * If we have some outdated packages, let's put an angry red face icon in this app!
 */
void MainWindow::refreshAppIcon()
{
  if (m_commandExecuting != ectn_NONE) return;

  disconnect(m_xbpsDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  if (m_debugInfo)
    qDebug() << "At refreshAppIcon()...";

  m_outdatedStringList = Package::getOutdatedStringList();
  m_numberOfOutdatedPackages = m_outdatedStringList->count();

  if (m_numberOfOutdatedPackages == 0)
  {
#ifdef KSTATUS
    m_systemTrayIcon->setToolTipSubTitle("OctoXBPS Notifier");
#else
    m_systemTrayIcon->setToolTip("OctoXBPS Notifier");
#endif
  }
  else if (m_numberOfOutdatedPackages > 0)
  {
    if (m_numberOfOutdatedPackages == 1)
    {
      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(StrConstants::getOneNewUpdate());
      #else
        m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate());
      #endif
    }
    else if (m_numberOfOutdatedPackages > 1)
    {
      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(
              StrConstants::getNewUpdates(m_numberOfOutdatedPackages));
      #else
        m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates(m_numberOfOutdatedPackages));
      #endif
    }
  }

  if(m_outdatedStringList->count() > 0) //RED ICON!
  {
    if(m_commandExecuting == ectn_NONE)
    {
      m_actionSystemUpgrade->setEnabled(true);
      m_actionSystemUpgrade->setVisible(true);
    }

    if (m_debugInfo)
      qDebug() << "Got a RED icon!";
    m_icon = IconHelper::getIconOctopiRed();


#ifdef KSTATUS
    m_systemTrayIcon->setAttentionIconByPixmap(m_icon);
    m_systemTrayIcon->setStatus(KStatusNotifierItem::NeedsAttention);
#endif
  }
  else //YEAHHH... GREEN ICON!
  {
    m_actionSystemUpgrade->setVisible(false);
    m_icon = IconHelper::getIconOctopiGreen();
    if (m_debugInfo)
      qDebug() << "Got a GREEN icon!";

#ifdef KSTATUS
    m_systemTrayIcon->setStatus(KStatusNotifierItem::Passive);
#endif
  }

  setWindowIcon(m_icon);

#ifdef KSTATUS
  m_systemTrayIcon->setIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipIconByPixmap(m_icon);
#else
  m_systemTrayIcon->setIcon(m_icon);
#endif

  connect(m_xbpsDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));
}

/*
 * Whenever the user clicks on the systemTray icon...
 */
void MainWindow::execSystemTrayActivated(QSystemTrayIcon::ActivationReason ar)
{
  if (m_commandExecuting != ectn_NONE) return;

  switch (ar)
  {
  case QSystemTrayIcon::DoubleClick:
  {
    if (m_outdatedStringList->count() > 0)
    {
      runOctoXBPS(ectn_SYSUPGRADE_EXEC_OPT);
    }
    else
    {
      runOctoXBPS(ectn_NORMAL_EXEC_OPT);
    }

    break;
  }
  case QSystemTrayIcon::Trigger:
  {
    if (m_outdatedStringList->count() > 0)
    {
      runOctoXBPSSysUpgrade();
    }

    break;
  }
  default: break;
  }
}

/*
 * This slot is called only when we're using Knotifications from KF5
 */
void MainWindow::execSystemTrayKF5()
{
  if (m_commandExecuting != ectn_NONE) return;
  if (m_outdatedStringList->count() > 0)
  {
    runOctoXBPSSysUpgrade();
  }
}

/*
 * When the users quit this notifier...
 */
void MainWindow::exitNotifier()
{
  if (m_debugInfo)
    qDebug() << "At exitNotifier()...";

  qApp->quit();
}

/*
 * Execs OctoXBPS
 */
void MainWindow::runOctoXBPS(ExecOpt execOptions)
{
  if (execOptions == ectn_SYSUPGRADE_EXEC_OPT && m_outdatedStringList->count() > 0)
  {
    QProcess::startDetached("octoxbps -sysupgrade");
  }
  else if (execOptions == ectn_NORMAL_EXEC_OPT)
  {
    QProcess::startDetached("octoxbps");
  }
}

/*
 * Calls the QDialog to set notifier interval
 */
void MainWindow::showOptionsDialog()
{
  if (m_optionsDialog == nullptr)
  {
    m_optionsDialog = new OptionsDialog(this);
    utils::positionWindowAtScreenCenter(m_optionsDialog);
    m_optionsDialog->exec();
    delete m_optionsDialog;
    m_optionsDialog = nullptr;
  }
}
