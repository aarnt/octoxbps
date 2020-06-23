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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QDir>

/*
 * Collection of OctoXBPS constants and enums
 */

//MainWindow related
enum SystemUpgradeOptions { ectn_NO_OPT, ectn_SYNC_DATABASE_OPT, ectn_NOCONFIRM_OPT };

//UnixCommand related
const QString ctn_MIRROR_CHECK_APP("mirror-check");

enum CommandExecuting { ectn_NONE, ectn_MIRROR_CHECK, ectn_SYNC_DATABASE,
                        ectn_SYSTEM_UPGRADE, ectn_INSTALL, ectn_REMOVE,
                        ectn_REMOVE_INSTALL, ectn_REMOVE_KCP_PKG, ectn_CLEAN_CACHE,
                        ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL, ectn_RUN_IN_TERMINAL, ectn_LOCAL_PKG_REFRESH };

enum LinuxDistro { ectn_TRIDENT, ectn_VOID, ectn_UNKNOWN };

enum Language { ectn_LANG_ENGLISH, ectn_LANG_USER_DEFINED };


//PacmanExec - QTextBrowser related
enum TreatString { ectn_TREAT_STRING, ectn_DONT_TREAT_STRING };
enum TreatURLLinks { ectn_TREAT_URL_LINK, ectn_DONT_TREAT_URL_LINK };

//SettingsManager related
enum SaveSettingsReason { ectn_PackageList, ectn_CurrentTabIndex, ectn_NORMAL=30,
                          ectn_MAXIMIZE_PACKAGES=40, ectn_MAXIMIZE_PROPERTIES=50, ectn_GROUPS=5,
                          ectn_CONSOLE_FONT_SIZE };

const QString ctn_ORGANIZATION(QStringLiteral("octoxbps"));
const QString ctn_APPLICATION(QStringLiteral("octoxbps"));
const QString ctn_KEY_CURRENT_TAB_INDEX(QStringLiteral("Current_Tab_Index"));
const QString ctn_KEY_WINDOW_SIZE(QStringLiteral("Window_Size"));
const QString ctn_KEY_TRANSACTION_WINDOW_SIZE(QStringLiteral("Transaction_Window_Size"));
const QString ctn_KEY_OUTPUTDIALOG_WINDOW_SIZE(QStringLiteral("OutputDialog_Window_Size"));
const QString ctn_KEY_PANEL_ORGANIZING(QStringLiteral("Panel_Organizing"));
const QString ctn_KEY_PACKAGE_LIST_ORDERED_COL(QStringLiteral("PackageList_Ordered_Col"));
const QString ctn_KEY_PACKAGE_LIST_SORT_ORDER(QStringLiteral("PackageList_Sort_Order"));
const QString ctn_KEY_AUR_PACKAGE_LIST_ORDERED_COL(QStringLiteral("Aur_PackageList_Ordered_Col"));
const QString ctn_KEY_AUR_PACKAGE_LIST_SORT_ORDER(QStringLiteral("Aur_PackageList_Sort_Order"));
const QString ctn_KEY_SKIP_MIRRORCHECK_ON_STARTUP(QStringLiteral("Skip_Mirror_Check_At_Startup"));
const QString ctn_KEY_SPLITTER_HORIZONTAL_STATE(QStringLiteral("Splitter_Horizontal_State"));
const QString ctn_KEY_SHOW_GROUPS_PANEL(QStringLiteral("Show_Groups_Panel"));
const QString ctn_KEY_PACKAGE_ICON_COLUMN_WIDTH(QStringLiteral("Package_Icon_Column_Width"));
const QString ctn_KEY_PACKAGE_NAME_COLUMN_WIDTH(QStringLiteral("Package_Name_Column_Width"));
const QString ctn_KEY_PACKAGE_VERSION_COLUMN_WIDTH(QStringLiteral("Package_Version_Column_Width"));
const QString ctn_KEY_TERMINAL(QStringLiteral("Terminal"));
const QString ctn_KEY_INSTANT_SEARCH(QStringLiteral("Instant_Search"));
const QString ctn_AUTOMATIC(QStringLiteral("automatic"));
const QString ctn_KEY_CONSOLE_SIZE(QStringLiteral("Console_Font_Size"));
const QString ctn_KEY_TERMINAL_COLOR_SCHEME(QStringLiteral("Terminal_Color_Scheme"));
const QString ctn_KEY_TERMINAL_FONT_FAMILY(QStringLiteral("Terminal_Font_Family"));
const QString ctn_KEY_TERMINAL_FONT_POINT_SIZE(QStringLiteral("Terminal_Font_Point_Size"));

//Notifier related
const QString ctn_KEY_LAST_SYNC_DB_TIME(QStringLiteral("LastSyncDbTime"));
const QString ctn_KEY_SYNC_DB_INTERVAL(QStringLiteral("SyncDbInterval"));
const QString ctn_KEY_SYNC_DB_HOUR(QStringLiteral("SyncDbHour"));

//CacheCleaner related
const QString ctn_KEEP_NUM_INSTALLED(QStringLiteral("Keep_Num_Installed"));
const QString ctn_KEEP_NUM_UNINSTALLED(QStringLiteral("Keep_Num_Uninstalled"));

//Package related
const QString ctn_TEMP_ACTIONS_FILE(QDir::homePath() + QDir::separator() + QStringLiteral(".config/octoxbps") +
                                    QDir::separator() + QStringLiteral(".qt_temp_"));
const QString ctn_XBPS_DATABASE_DIR = QStringLiteral("/var/db/xbps");
//const QString ctn_PACMAN_CORE_DB_FILE = QStringLiteral("/var/lib/pacman/sync/core.db");

enum PackageStatus { ectn_INSTALLED, ectn_NON_INSTALLED, ectn_OUTDATED, ectn_NEWER,
                     ectn_FOREIGN, ectn_FOREIGN_OUTDATED };

enum ViewOptions { ectn_ALL_PKGS, ectn_INSTALLED_PKGS, ectn_NON_INSTALLED_PKGS };

//TransactionDialog related
const int ctn_RUN_IN_TERMINAL(328);

//WMHelper related
const QString ctn_NO_SU_COMMAND(QStringLiteral("none"));
const QString ctn_OCTOXBPS_SUDO(QStringLiteral("/usr/lib/octoxbps/octoxbps-sudo"));
const QString ctn_OCTOXBPS_SUDO_PARAMS(QStringLiteral("-d"));
const QString ctn_ROOT_SH(QStringLiteral("/bin/sh -c "));

const QString ctn_LXQTSU(QStringLiteral("lxqt-sudo"));
const QString ctn_QSUDO(QStringLiteral("qsudo"));
const QString ctn_KDESU(QStringLiteral("kdesu"));
const QString ctn_KDE_DESKTOP(QStringLiteral("kwin"));
const QString ctn_KDE_X11_DESKTOP(QStringLiteral("kwin_x11"));
const QString ctn_KDE_WAYLAND_DESKTOP(QStringLiteral("kwin_wayland"));
const QString ctn_KDE_EDITOR(QStringLiteral("kwrite"));
const QString ctn_KDE_FILE_MANAGER(QStringLiteral("kfmclient"));
const QString ctn_KDE_TERMINAL(QStringLiteral("konsole"));
const QString ctn_KDE4_OPEN(QStringLiteral("kde-open"));
const QString ctn_KDE5_OPEN(QStringLiteral("kde-open5"));
const QString ctn_KDE4_FILE_MANAGER(QStringLiteral("dolphin"));
const QString ctn_KDE4_EDITOR(QStringLiteral("kate"));

const QString ctn_TDESU(QStringLiteral("tdesu"));
const QString ctn_TDE_DESKTOP(QStringLiteral("twin"));
const QString ctn_TDE_EDITOR(QStringLiteral("kwrite"));
const QString ctn_TDE_FILE_MANAGER(QStringLiteral("kfmclient"));
const QString ctn_TDE_TERMINAL(QStringLiteral("konsole"));

const QString ctn_GKSU_1(QStringLiteral("/usr/libexec/gksu"));
const QString ctn_GKSU_2(QStringLiteral("gksu"));

const QString ctn_ANTERGOS_FILE_MANAGER(QStringLiteral("nautilus"));

const QString ctn_ARCHBANG_EDITOR(QStringLiteral("medit"));
const QString ctn_ARCHBANG_FILE_MANAGER(QStringLiteral("spacefm"));

const QString ctn_RXVT_TERMINAL(QStringLiteral("urxvt"));
const QString ctn_XFCE_DESKTOP(QStringLiteral("xfdesktop"));
const QString ctn_XFCE_EDITOR(QStringLiteral("mousepad"));
const QString ctn_XFCE_EDITOR_ALT(QStringLiteral("leafpad"));
const QString ctn_XFCE_FILE_MANAGER(QStringLiteral("thunar"));
const QString ctn_XFCE_TERMINAL(QStringLiteral("xfce4-terminal"));

const QString ctn_OPENBOX_DESKTOP(QStringLiteral("openbox"));
const QString ctn_LXDE_DESKTOP(QStringLiteral("lxsession"));
const QString ctn_LXDE_TERMINAL(QStringLiteral("lxterminal"));
const QString ctn_LXDE_FILE_MANAGER(QStringLiteral("pcmanfm"));

const QString ctn_XDG_OPEN(QStringLiteral("xdg-open"));

const QString ctn_LUMINA_DESKTOP(QStringLiteral("lumina-desktop"));
const QString ctn_LUMINA_EDITOR(QStringLiteral("lumina-textedit"));
const QString ctn_LUMINA_FILE_MANAGER(QStringLiteral("lumina-fm"));
const QString ctn_LUMINA_OPEN(QStringLiteral("lumina-open"));

const QString ctn_LXQT_DESKTOP(QStringLiteral("lxqt-session"));
const QString ctn_LXQT_TERMINAL(QStringLiteral("qterminal"));
const QString ctn_LXQT_FILE_MANAGER(QStringLiteral("pcmanfm-qt"));
const QString ctn_LXQT_EDITOR(QStringLiteral("juffed"));

const QString ctn_MATE_DESKTOP(QStringLiteral("mate-session"));
const QString ctn_MATE_EDITOR(QStringLiteral("mate-open"));
const QString ctn_MATE_FILE_MANAGER(QStringLiteral("caja"));
const QString ctn_MATE_TERMINAL(QStringLiteral("mate-terminal"));

const QString ctn_CINNAMON_DESKTOP(QStringLiteral("cinnamon-session"));
const QString ctn_CINNAMON_EDITOR(QStringLiteral("gedit"));
const QString ctn_CINNAMON_FILE_MANAGER(QStringLiteral("nemo"));
const QString ctn_CINNAMON_TERMINAL(QStringLiteral("gnome-terminal"));
const QString ctn_XTERM(QStringLiteral("xterm"));

enum EditOptions { ectn_EDIT_AS_ROOT, ectn_EDIT_AS_NORMAL_USER };


//OctoXBPS-notifier related  -------------------------------------------------------------------------------

const QString ctn_PKEXEC_BINARY = QStringLiteral("/usr/bin/pkexec");

enum ExecOpt { ectn_NORMAL_EXEC_OPT, ectn_SYSUPGRADE_EXEC_OPT, ectn_SYSUPGRADE_NOCONFIRM_EXEC_OPT };

#endif // CONSTANTS
