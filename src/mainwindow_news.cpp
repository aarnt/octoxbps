/*
* This file is part of OctoXBPS, an open-source GUI for XBPS.
* Copyright (C) 2015 Alexandre Albuquerque Arnt
*               2013 Manuel Tortosa
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
 * This is MainWindow's Distro news related code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "searchbar.h"
#include "uihelper.h"
#include "globals.h"

#include <QTextBrowser>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

/*
 * This is the high level method that orquestrates the Distro RSS News printing in tabNews
 *
 * boolean parameter searchForLatestNews:
 *    controls whether this method tries to connect to the remote RSS News Feed (default is true)
 * boolean parameter gotoNewsTab:
 *    controls whether this method must set the mainwindow focus to NewsTab (default is true)
 */
void MainWindow::refreshDistroNews(bool searchForLatestNews, bool gotoNewsTab)
{
  if (!isHidden() && !isInternetAvailable()) return;

  ui->actionGetNews->setEnabled(false);
  m_gotoNewsTab = gotoNewsTab;

  if (searchForLatestNews)
  {
    LinuxDistro distro = UnixCommand::getLinuxDistro();

    if (gotoNewsTab)
    {
      clearTabOutput();
    }

    if (gotoNewsTab && distro == ectn_VOID)
    {
      writeToTabOutput("<b>" +
                          StrConstants::getSearchingForBSDNews().arg("Void Linux") + "</b>");
    }

    /*
     * Here, we retrieve BSD's latest news without
     * blocking OctoPkg main interface.
     */
    QFuture<QString> f;
    f = QtConcurrent::run(getLatestDistroNews);
    g_fwDistroNews.setFuture(f);
    connect(&g_fwDistroNews, SIGNAL(finished()), this, SLOT(postRefreshDistroNews()));
  }
  else
  {
    QString distroRSSXML = utils::retrieveDistroNews(searchForLatestNews);
    showDistroNews(distroRSSXML, false);
  }
}

/*
 * After we have the multithreaded RSS distro news, we parse and show it!
 */
void MainWindow::postRefreshDistroNews()
{
  showDistroNews(g_fwDistroNews.result(), true);
}

/*
 * At last we have to just show the retrivied news html
 */
void MainWindow::showDistroNews(QString distroRSSXML, bool searchForLatestNews)
{
  QString html;

  if (distroRSSXML.length() >= 200)
  {
    if (distroRSSXML.at(0)=='*')
    {
      /* If this is an updated RSS, we must warn the user!
       And if the main window is hidden... */
      if (isHidden())
      {
        show();
      }

      ui->twProperties->setTabText(ctn_TABINDEX_NEWS, "** " + StrConstants::getTabNewsName() + " **");
      if (m_gotoNewsTab)
      {
        ui->twProperties->setCurrentIndex(ctn_TABINDEX_NEWS);
      }
    }
    else
    {
      if(searchForLatestNews)
      {
        ui->twProperties->setTabText(ctn_TABINDEX_NEWS, StrConstants::getTabNewsName());
      }
    }

    //First, we have to parse the raw RSS XML...
    html = utils::parseDistroNews();
  }
  else
  {
    if(searchForLatestNews)
      ui->twProperties->setTabText(ctn_TABINDEX_NEWS, StrConstants::getTabNewsName());

    html = distroRSSXML;
  }

  //Now that we have the html table code, let's put it into TextBrowser's News tab
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_NEWS)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    text->clear();
    text->setHtml(html);
  }

  if (m_gotoNewsTab)
  {
    clearTabOutput();
  }

  if (searchForLatestNews && m_gotoNewsTab)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_NEWS);
  }

  ui->actionGetNews->setEnabled(true);
}

/*
 * This SLOT is called every time the user clicks a url inside newsTab textBrowser
 */
void MainWindow::onTabNewsSourceChanged(QUrl newSource)
{
  if(newSource.isRelative())
  {
    //If the user clicked a relative and impossible to display link...
    QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_NEWS)->findChild<QTextBrowser*>("textBrowser");
    if (text)
    {
      disconnect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));
      text->setHtml(utils::parseDistroNews());
      connect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));
    }
  }
}

/*
 * This is the TextBrowser News tab, which shows the latest news from Distro news feed
 */
void MainWindow::initTabNews()
{
  QString aux(StrConstants::getTabNewsName());
  QWidget *tabNews = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabNews);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setContentsMargins(0, 0, 0, 0);

  QTextBrowser *text = new QTextBrowser(tabNews);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget(text, 0, 0, 1, 1);
  text->show();

#if QT_VERSION > 0x050000
  int tindex = ui->twProperties->insertTab(ctn_TABINDEX_NEWS, tabNews, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0/*, QApplication::UnicodeUTF8*/ ) );
  ui->twProperties->setTabText(ui->twProperties->indexOf(tabNews), QApplication::translate(
      "MainWindow", aux.toUtf8(), 0/*, QApplication::UnicodeUTF8*/));
#else
  int tindex = ui->twProperties->insertTab(ctn_TABINDEX_NEWS, tabNews, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  ui->twProperties->setTabText(ui->twProperties->indexOf(tabNews), QApplication::translate(
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));
#endif

  SearchBar *searchBar = new SearchBar(this);

  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChangedInTextBrowser(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosedInTextBrowser()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNextInTextBrowser()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPreviousInTextBrowser()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);

  connect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));

  text->show();

  ui->twProperties->setCurrentIndex(tindex);
  text->setFocus();
}
