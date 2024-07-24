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

#include <iostream>
#include <cassert>

#include "packagemodel.h"
#include "src/uihelper.h"
#include "src/strconstants.h"

#include <QRegularExpression>
#include <QDebug>

/*
 * The specific model which abstracts the package list data seem in the main treeview
 */

PackageModel::PackageModel(const PackageRepository& repo, QObject *parent)
: QAbstractItemModel(parent), m_installedPackagesCount(0), m_showColumnPopularity(false), m_packageRepo(repo),
  m_sortOrder(Qt::AscendingOrder), m_sortColumn(1), m_filterPackagesInstalled(false),
  m_filterPackagesNotInstalled(false), m_filterPackagesNotInThisGroup(""),
  m_filterColumn(-1), m_filterRegExp("", QRegularExpression::CaseInsensitiveOption),
  m_iconNotInstalled(IconHelper::getIconNonInstalled()), m_iconInstalled(IconHelper::getIconInstalled()),
  m_iconInstalledUnrequired(IconHelper::getIconUnrequired()),
  m_iconNewer(IconHelper::getIconNewer()), m_iconOutdated(IconHelper::getIconOutdated()),
  m_iconForeign(IconHelper::getIconForeignGreen()), m_iconForeignOutdated(IconHelper::getIconForeignRed())
{
  m_showColumnPopularity = false;
}

QModelIndex PackageModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
    return createIndex(row, column, (void*)NULL);

  return QModelIndex();
}

QModelIndex PackageModel::parent(const QModelIndex&) const
{
  return QModelIndex();
}

int PackageModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid()) {
    return m_columnSortedlistOfPackages.size();
  }
  else {
    return 0;
  }
}

int PackageModel::columnCount(const QModelIndex &parent) const
{
  if (!parent.isValid()) {
    return 3;
  }
  else return 0;
}

QVariant PackageModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid() && m_columnSortedlistOfPackages.size() > index.row()) {
    switch (role) {
      case Qt::DisplayRole: {
        const PackageRepository::PackageData*const package = getData(index);
        if (package == NULL)
          return QVariant();

        switch (index.column()) {
          case ctn_PACKAGE_ICON_COLUMN:
            break;
          case ctn_PACKAGE_NAME_COLUMN:
            return QVariant(package->name);
          case ctn_PACKAGE_VERSION_COLUMN:
            return QVariant(package->version);
          case ctn_PACKAGE_SIZE_COLUMN:
          {
            if (package->installed())
              return QVariant(Package::kbytesToSize(package->installedSize));
            else
              return QVariant(Package::kbytesToSize(package->downloadSize));
          }
          /*case ctn_PACKAGE_INSTALLEDON_COLUMN:
          {
            bool ok;
            long idate=package->installedOn.toLong(&ok);
            if (idate <= 0) return QVariant(QStringLiteral(""));

            QString dateTimeFormat = QLocale().dateTimeFormat(QLocale::ShortFormat);
            QDateTime idt = QDateTime::fromSecsSinceEpoch(idate);
            return QVariant(idt.toString(dateTimeFormat));
          }

          break;*/
          default:
            assert(false);
        }
        break;
      }
      case Qt::DecorationRole: {
        const PackageRepository::PackageData*const package = getData(index);
        if (package == NULL)
          return QVariant();

        if (index.column() == ctn_PACKAGE_ICON_COLUMN) {
          return QVariant(getIconFor(*package));
        }
        break;
      }
      case Qt::ToolTipRole:
        break;
      case Qt::StatusTipRole:
        break;
      default:
        break;
    }
  }
  return QVariant();
}

QVariant PackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
      case ctn_PACKAGE_ICON_COLUMN:
        return QVariant();
      case ctn_PACKAGE_NAME_COLUMN:
        return QVariant(StrConstants::getName());
      case ctn_PACKAGE_VERSION_COLUMN:
        return QVariant(StrConstants::getVersion());
      /*case ctn_PACKAGE_INSTALLEDON_COLUMN:
        return QVariant(StrConstants::getInstalledOn());*/
      /*case ctn_PACKAGE_ORIGIN_COLUMN:
        return QVariant(StrConstants::getOrigin());*/
      /*case ctn_PACKAGE_SIZE_COLUMN:
        return QVariant(StrConstants::getSize());*/
      default:
        break;
      }
    }
    return QVariant(section);
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

void PackageModel::sort(int column, Qt::SortOrder order)
{
//  std::cout << "sort column " << column << " in order " << order << std::endl;

  if (column != m_sortColumn || order != m_sortOrder)
  {
    m_sortColumn = column;
    m_sortOrder  = order;
    emit layoutAboutToBeChanged();
    sort();
    emit layoutChanged();
  }
}

/*
 * Resets the contents of the model
 */
void PackageModel::clear()
{
  beginResetModel();
  m_listOfPackages.clear();
  m_columnSortedlistOfPackages.clear();
}

void PackageModel::beginResetRepository()
{
  beginResetModel();
  m_listOfPackages.clear();
  m_columnSortedlistOfPackages.clear();
}

void PackageModel::endResetRepository()
{
  m_installedPackagesCount = 0;
  const QList<PackageRepository::PackageData*>& data = m_packageRepo.getPackageList(m_filterPackagesNotInThisGroup);
  m_listOfPackages.reserve(data.size());

  for (QList<PackageRepository::PackageData*>::const_iterator it = data.begin(); it != data.end(); ++it)
  {
    //if (m_filterPackagesNotInstalled && (*it)->installed()) continue;
    //else if (m_filterPackagesInstalled && !(*it)->installed()) continue;

    if (!m_filterPackagesNotInThisRepo.isEmpty() && (*it)->repository != m_filterPackagesNotInThisRepo) continue;

    if (m_filterRegExp.pattern().isEmpty()) {
      m_listOfPackages.push_back(*it);
      if ((*it)->installed()) m_installedPackagesCount++;
    }
    else {
      QRegularExpressionMatch match;

      switch (m_filterColumn) {
      case ctn_PACKAGE_NAME_COLUMN:
        match = m_filterRegExp.match((*it)->name);
        if (match.hasMatch()) //m_filterRegExp.indexIn((*it)->name) != -1)
        {
          m_listOfPackages.push_back(*it);
          if ((*it)->installed()) m_installedPackagesCount++;
        }
        break;
      case ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN:
        match = m_filterRegExp.match((*it)->comment);
        if (match.hasMatch()) //(m_filterRegExp.indexIn((*it)->comment) != -1)
        {
          m_listOfPackages.push_back(*it);
          if ((*it)->installed()) m_installedPackagesCount++;
        }
        break;
      default:
        m_listOfPackages.push_back(*it);
        if ((*it)->installed()) m_installedPackagesCount++;
      }
    }
  }

  m_columnSortedlistOfPackages.reserve(data.size());
  m_columnSortedlistOfPackages = m_listOfPackages;
  sort();
  endResetModel();
}

int PackageModel::getPackageCount() const
{
  return m_listOfPackages.size();
}

int PackageModel::getInstalledPackagesCount() const
{
  return m_installedPackagesCount;
}

bool PackageModel::isFiltered() const
{
  return (m_filterPackagesInstalled ||
          m_filterPackagesNotInstalled ||
          !m_filterPackagesNotInThisGroup.isEmpty() ||
          !m_filterPackagesNotInThisRepo.isEmpty());
}

const PackageRepository::PackageData* PackageModel::getData(const QModelIndex& index) const
{
  if (index.isValid() && index.row() < m_columnSortedlistOfPackages.size()) {
    switch (m_sortOrder) {
    case Qt::AscendingOrder:
      return m_columnSortedlistOfPackages.at(index.row());
    case Qt::DescendingOrder:
      return m_columnSortedlistOfPackages.at(m_listOfPackages.size() - index.row() - 1);
    }
  }
  return NULL;
}

void PackageModel::applyFilter(ViewOptions pkgViewOptions, const QString& repo, const QString& group)
{
  beginResetRepository();
  m_filterPackagesNotInstalled   = (pkgViewOptions == ectn_NON_INSTALLED_PKGS);
  m_filterPackagesInstalled      = (pkgViewOptions == ectn_INSTALLED_PKGS);
  m_filterPackagesNotInThisGroup = group;

  QString r = repo;
  r = r.remove(QRegularExpression("&"));
  if (r == StrConstants::getAll()) r = "";

  m_filterPackagesNotInThisRepo = r;
  endResetRepository();
}

void PackageModel::applyFilter(bool packagesNotInstalled, const QString& group)
{
  beginResetRepository();
  m_filterPackagesNotInstalled   = packagesNotInstalled;
  m_filterPackagesNotInThisGroup = group;
  endResetRepository();
}

void PackageModel::applyFilter(const int filterColumn)
{
  applyFilter(filterColumn, m_filterRegExp.pattern());
}

void PackageModel::applyFilter(const QString& filterExp)
{
  applyFilter(m_filterColumn, filterExp);
}

void PackageModel::applyFilter(const int filterColumn, const QString& filterExp)
{
  assert(filterExp.isNull() == false);
//  std::cout << "apply new column filter " << filterColumn << ", " << filterExp.toStdString() << std::endl;
  beginResetRepository();
  m_filterColumn = filterColumn;
  m_filterRegExp.setPattern(filterExp);
  endResetRepository();
}

/*
 * Toggles the view of column popularity, which shows number of votes for AUR pkgs
 */
void PackageModel::setShowColumnPopularity(bool value)
{
  m_showColumnPopularity = value;
}

const QIcon& PackageModel::getIconFor(const PackageRepository::PackageData& package) const
{
  switch (package.status)
  {
    case ectn_FOREIGN:
      return m_iconForeign;
    case ectn_FOREIGN_OUTDATED:
      return m_iconForeignOutdated;
    case ectn_OUTDATED:
    {
      //TODO: potential refactoring for performance if necessary
      /*if (Package::rpmvercmp(package.outdatedVersion.toLatin1().data(), package.version.toLatin1().data()) == 1) {
        return m_iconNewer;
      }
      else {
        return m_iconOutdated;
      }
      assert(false);*/
      return m_iconOutdated;
    }
    case ectn_NEWER:
      return m_iconNewer;
    case ectn_INSTALLED:
      // Does no other package depend on this package ? (unrequired package list)

      if (package.required)
      {
        return m_iconInstalled;
      }
      else
      {
        return m_iconInstalledUnrequired;
      }
    break;
    case ectn_NON_INSTALLED:
//      if (package.required == false) std::cout << "not installed not required" << std::endl; // doesn't happen with pacman
      return m_iconNotInstalled;
    default:
      assert(false);
  }
}

struct TSort0 {
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {
    if (a->status < b->status) return true;
    if (a->status == b->status)
    {
      /*if (a->origin == b->origin)
      {
        return (a->name < b->name);
      }*/

      if (a->outdated() == true && b->outdated() == true)
      {
        return (a->name < b->name);
      }

      if (a->required < b->required) return true;

      if (a->required == b->required)
      {
        int cmp = QString::localeAwareCompare(a->name, b->name);
        if (cmp < 0) return true;
      }
    }
    return false;
  }
};

struct TSort2 {
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {
    const int cmp = Package::rpmvercmp(a->version.toLatin1().data(), b->version.toLatin1().data());

    if (cmp < 0) return true;
    if (cmp == 0)
    {
      return a->name < b->name;
    }

    return false;
  }
};

struct TSort3 {
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {
    if (a->origin < b->origin) return true;

    if (a->origin == b->origin)
    {
      return a->name < b->name;
    }

    return false;
  }
};

struct TSort4 {
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {      
    QString mag_a, mag_b, aux_a, aux_b;
    bool installed = true;

    if (a->repository != ctn_PKGNG_FAKE_REPOSITORY)
    {
      aux_a = Package::kbytesToSize(a->installedSize);
      aux_b = Package::kbytesToSize(b->installedSize);
      installed = true;
      //qDebug() << "a is: " << aux_a;
      //qDebug() << "b is: " << aux_b;
    }
    else if (a->repository == ctn_PKGNG_FAKE_REPOSITORY)
    {
      aux_a = Package::kbytesToSize(a->downloadSize);
      aux_b = Package::kbytesToSize(b->downloadSize);
      installed = false;
      //qDebug() << "a is: " << aux_a;
      //qDebug() << "b is: " << aux_b;
    }

    QStringList s = aux_a.split(" ");
    mag_a = s.at(1);

    s = aux_b.split(" ");
    mag_b = s.at(1);

    if (mag_a == mag_b)
    {
      if (installed)
      {
        if (a->installedSize < b->installedSize) return true;

        if (a->installedSize == b->installedSize) {
          return a->name < b->name;
        }
      }
      else
      {
        if (a->downloadSize < b->downloadSize) return true;

        if (a->downloadSize == b->downloadSize) {
          return a->name < b->name;
        }
      }
    }
    else
    {
      if (mag_a < mag_b) return true;
    }

    return false;
  }
};

struct TSort5 {
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {
    if (a->installedOn < b->installedOn) return true;

    if (a->installedOn == b->installedOn)
    {
      return a->name < b->name;
    }

    return false;
  }
};

void PackageModel::sort()
{
  switch (m_sortColumn) {
  case ctn_PACKAGE_NAME_COLUMN:
    m_columnSortedlistOfPackages = m_listOfPackages;
    return;
  case ctn_PACKAGE_ICON_COLUMN:
    std::sort(m_columnSortedlistOfPackages.begin(), m_columnSortedlistOfPackages.end(), TSort0());
    return;
  case ctn_PACKAGE_VERSION_COLUMN:
    std::sort(m_columnSortedlistOfPackages.begin(), m_columnSortedlistOfPackages.end(), TSort2());
    return;
  /*case ctn_PACKAGE_ORIGIN_COLUMN:
    qSort(m_columnSortedlistOfPackages.begin(), m_columnSortedlistOfPackages.end(), TSort3());
    return;*/
  case ctn_PACKAGE_SIZE_COLUMN:
    std::sort(m_columnSortedlistOfPackages.begin(), m_columnSortedlistOfPackages.end(), TSort4());
    return;
  case ctn_PACKAGE_INSTALLEDON_COLUMN:
    std::sort(m_columnSortedlistOfPackages.begin(), m_columnSortedlistOfPackages.end(), TSort5());
    return;
  default:
    return;
  }
}
