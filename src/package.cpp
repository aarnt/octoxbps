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

#include "package.h"
#include "unixcommand.h"
#include "stdlib.h"
#include "strconstants.h"
#include <iostream>

#include <QTextStream>
#include <QList>
#include <QFile>
#include <QDebug>
#include <QRegularExpression>

/*
 * This class abstracts all the relevant package information and services
 */

/*
 * Retrieves the basic package name, without version numbers
 */
QString Package::getBaseName( const QString& p )
{
	QString packageBaseName="";
	QString aux(p);
	int numberOfSegments = p.count('-')+1;
	int nameSegment = numberOfSegments-3;
	int a;

	for (int i=1; i<= nameSegment; i++){
		a=aux.indexOf("-");
		packageBaseName += aux.left(a);
		if (i<nameSegment) packageBaseName += "-";
		aux = aux.mid(a+1);
	}

	if (packageBaseName == "") packageBaseName += p.left(p.indexOf("-"));
	return packageBaseName;
}

/*
 * Given a QString, this method searches for a link pattern and inserts an URL html/ftp link tag
 * Returns the modified (or not) QString
 */
QString Package::makeURLClickable( const QString &s )
{
	QString sb = s;
  QRegExp rx("((ht|f)tp(s?))://(\\S)+[^\"|)|(|.|\\s|\\n]");
  rx.setCaseSensitivity( Qt::CaseInsensitive );
  int search = 0;
	int ini = 0;

	//First we search for the 1st pattern: rx
	while ( (ini = rx.indexIn( sb, search )) != -1 ){
		QString s1 = rx.cap();
    QString ns;

    s1.remove(QRegularExpression("</font></b><br>"));
    s1.remove("`");
    s1.remove("'");

    ns = "<a href=\"" + s1.trimmed() + "\">" + s1.trimmed() + "</a>";
    sb.replace( ini, s1.length(), ns);
		search = ini + (2*s1.length()) + 15;	
	}

  return sb;
}

/*
 * This function was copied from ArchLinux Pacman project
 * A pow() implementation that is specialized for an integer base and small,
 * positive-only integer exponents.
 */
double Package::simplePow(int base, int exp)
{
  double result = 1.0;
  for(; exp > 0; exp--) {
    result *= base;
  }

  return result;
}

/*
 * Converts a size in kbytes to a readable QString representation
 */
QString Package::kbytesToSize( float Bytes )
{
  float tb = 1073741824;
  float gb = 1048576;
  float mb = 1024;
  float kb = 1;
  QString res;

  if( Bytes >= tb )
    res = res.asprintf("%.2f TiB", (float)Bytes/tb);
  else if( Bytes >= gb && Bytes < tb )
    res = res.asprintf("%.2f GiB", (float)Bytes/gb);
  else if( Bytes >= mb && Bytes < gb )
    res = res.asprintf("%.2f MiB", (float)Bytes/mb);
  else if( Bytes >= kb && Bytes < mb )
    res = res.asprintf("%.2f KiB", (float)Bytes/kb);
  else if ( Bytes < kb)
    res = res.asprintf("%.2f Bytes", Bytes * 1024);
  else
    res = res.asprintf("%.2f Bytes", Bytes);

  return res;
}

/*
 * Converts the size in String type to double
 */
double Package::strToKBytes(QString size)
{
  double res=0;
  if (size == "0.00B") res = 0;
  else if (size.contains("kB", Qt::CaseInsensitive))
  {
    bool ok;
    int p = size.indexOf("kB", Qt::CaseInsensitive);
    double value = size.left(p).toDouble(&ok);
    if (ok)
    {
      res = value / 1.024;
    }
  }
  else if (size.contains("MB"))
  {
    bool ok;
    int p = size.indexOf("MB");
    double value = size.left(p).toDouble(&ok);
    if (ok)
    {
      res = (value * 1024) / 1.048576;
    }
  }
  else if (size.contains("B"))
  {
    bool ok;
    int p = size.indexOf("B");
    double value = size.left(p).toDouble(&ok);
    if (ok)
    {
      res = value;
    }
  }

  return res;
}

/*
 * Converts the size in String type to double
 */
double Package::strToKBytes2(QString size)
{
  double res=0;
  if (size == "0.00B") res = 0;
  else if (size.contains("KiB", Qt::CaseInsensitive))
  {
    bool ok;
    int p = size.indexOf("KiB", Qt::CaseInsensitive);
    double value = size.left(p).toDouble(&ok);
    if (ok)
    {
      res = value;
    }
  }
  else if (size.contains("MiB"))
  {
    bool ok;
    int p = size.indexOf("MiB");
    double value = size.left(p).toDouble(&ok);
    if (ok)
    {
      res = value * 1024;
    }
  }
  else if (size.contains("B"))
  {
    bool ok;
    int p = size.indexOf("B");
    double value = size.left(p).toDouble(&ok);
    if (ok)
    {
      res = value / 1024;
    }
  }

  return res;
}

/*
 * Retrieves the list of unrequired packages (those no other packages depends on)
 */
QSet<QString>* Package::getUnrequiredPackageList()
{
  QString pkgName;
  QString unrequiredPkgList = UnixCommand::getUnrequiredPackageList();
  QStringList packageTuples = unrequiredPkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QSet<QString>* res = new QSet<QString>();

  foreach(QString packageTuple, packageTuples)
  {
    int dash = packageTuple.lastIndexOf("-");
    if (dash != -1)
    {
      pkgName = packageTuple.left(dash);
      res->insert(pkgName);
    }
  }

  return res;
}

/*
 * Retrieves the list of outdated packages (those which have newer versions available to download)
 */
QMap<QString, OutdatedPackageInfo> *Package::getOutdatedStringList()
{
  QString pkgAux, pkgName;
  QString outPkgList = UnixCommand::getOutdatedPackageList();
  QStringList packageTuples = outPkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QMap<QString, OutdatedPackageInfo>* res = new QMap<QString, OutdatedPackageInfo>();

  foreach(QString packageTuple, packageTuples)
  {
    if (packageTuple.contains("update"))
    {
      QStringList parts = packageTuple.split(' ');
      {
        OutdatedPackageInfo opi;

        pkgAux = parts[0];
        int dash = pkgAux.lastIndexOf("-");

        if (dash != -1)
        {
          pkgName = pkgAux.left(dash);
          opi.newVersion = pkgAux.right(pkgAux.length() - (dash+1));          
          opi.oldVersion = getVersionByName(pkgName);
          res->insert(pkgName, opi);
        }
      }
    }
  }

  return res;
}

/*
 * Retrieves the list of targets needed to upgrade the entire system OR
 * to install/reinstall/upgrade a given package
 */
TransactionInfo Package::getTargetUpgradeList(const QString &pkgName)
{
  QString targets = UnixCommand::getTargetUpgradeList(pkgName);
  QString pkg;
  QStringList infoTuples = targets.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  TransactionInfo res;
  res.packages = new QStringList();
  int pos;
  QString strSum;
  double number;
  double sum=0;

  foreach(QString infoTuple, infoTuples)
  {
    QStringList t = infoTuple.split(" ");
    pkg = t.at(0);
    pos = pkg.lastIndexOf("-");
    pkg = pkg.left(pos);
    res.packages->append(pkg);

    if (t.size() == 5){
      QString ds = t.at(4);
      number = ds.toDouble() / 1024;
    }

    if (t.size() == 6){
      QString ds = t.at(5);
      number = ds.toDouble() / 1024;
    }

    sum += number;
  }

  if (sum > 1024)
  {
    sum /= 1024;
    strSum = QString::number(sum, 'f', 2) + "MB";
  }
  else
  {
    strSum = QString::number(sum, 'f', 2) + "KB";
  }

  res.sizeToDownload = strSum;
  res.packages->sort();
  return res;
}

/*
 * Retrieves the list of targets needed to be removed with the given package
 */
QStringList *Package::getTargetRemovalList(const QString &pkgName)
{
  QString targets = UnixCommand::getTargetRemovalList(pkgName);
  QStringList infoTuples = targets.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QStringList *res = new QStringList();
  QString pkg;
  int space;

  foreach(QString infoTuple, infoTuples)
  {
    int pos = infoTuple.indexOf("remove");
    if (pos == -1)
    {
      return res;
    }

    pos = infoTuple.indexOf("remove");
    if (pos != -1) //We are dealing with packages HERE!
    {
      space = infoTuple.indexOf(" ");
      if (space != -1)
      {
        pkg = infoTuple.left(space);
        pos = pkg.lastIndexOf("-");
        pkg = pkg.left(pos);
        res->append(pkg);
      }
    }
  }

  res->sort();
  return res;
}

/*
 *Retrieves the list of foreign packages (those installed from unknown repositories like AUR)
 */
/*QList<PackageListData> *Package::getForeignPackageList()
{
  QString foreignPkgList = UnixCommand::getForeignPackageList();
  QStringList packageTuples = foreignPkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QList<PackageListData> * res = new QList<PackageListData>();

  foreach(QString packageTuple, packageTuples)
  {
    QStringList parts = packageTuple.split(' ');
    if (parts.size() == 2)
    {
      res->append(PackageListData(parts[0], "", parts[1], ectn_FOREIGN));
    }
  }

  return res;
}*/

/*
 * Retrieves the list of all available packages in the database (installed + non-installed)
 */
QList<PackageListData> * Package::getPackageList(const QString &packageName)
{
  QString pkgAux, pkgName, pkgOrigin, pkgVersion, pkgComment, pkgDescription;
  double pkgInstalledSize, pkgDownloadedSize;
  PackageStatus pkgStatus;
  QString pkgList = UnixCommand::getPackageList(packageName);
  QStringList packageTuples = pkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QList<PackageListData> * res = new QList<PackageListData>();

  if(!pkgList.isEmpty())
  {
    pkgDescription = "";
    foreach(QString packageTuple, packageTuples)
    {
      pkgComment = "";
      QStringList parts = packageTuple.split(' ');      
      pkgAux = parts[1];
      int dash = pkgAux.lastIndexOf("-");

      if (dash != -1)
      {
        pkgName = pkgAux.left(dash);
        pkgVersion = pkgAux.right(pkgAux.length()-(dash+1));
      }

      if (parts[0] == "[*]" || parts[0] == "i" || parts[0] == "ii")
        pkgStatus = ectn_INSTALLED;
      else
        pkgStatus = ectn_NON_INSTALLED;

      pkgDownloadedSize = 0;
      pkgInstalledSize = 0; //strToKBytes(parts[3]);

      for(int c=2; c<parts.count(); c++)
      {
        pkgComment += " " + parts[c];
      }

      if (!pkgComment.isEmpty()) pkgComment = pkgName + " " + pkgComment;
      pkgComment = pkgComment.trimmed();
      pkgDescription = pkgComment;

      PackageListData pld =
          PackageListData(pkgName, pkgOrigin, pkgVersion, pkgComment, pkgStatus, pkgInstalledSize, pkgDownloadedSize);

      res->append(pld);
    }
  }

  return res;
}

/*
 * Parses the list of packages obtained by the "pkg" tool
 */
QList<PackageListData> * Package::parsePackageTuple(const QStringList &packageTuples, QStringList &packageCache)
{
  Q_UNUSED(packageCache)

  QString pkgAux, pkgName, pkgVersion, pkgComment, strStatus, pkgDescription, pkgOrigin;
  PackageStatus pkgStatus;
  double pkgInstalledSize, pkgDownloadedSize;

  QList<PackageListData> * res = new QList<PackageListData>();

  foreach(QString packageTuple, packageTuples)
  {
    pkgComment = "";
    QStringList parts = packageTuple.split(' ');
    strStatus = parts[0];
    pkgAux = parts[1];
    int dash = pkgAux.lastIndexOf("-");

    if (dash != -1)
    {
      pkgName = pkgAux.left(dash);
      pkgVersion = pkgAux.right(pkgAux.length()-(dash+1));
    }

    if (strStatus.contains("*"))
      pkgStatus = ectn_INSTALLED;
    else
      pkgStatus = ectn_NON_INSTALLED;

    pkgDownloadedSize = 0;
    pkgInstalledSize = 0; //strToKBytes(parts[3]);

    for(int c=2; c<parts.count(); c++)
    {
      pkgComment += " " + parts[c];
    }

    if (!pkgComment.isEmpty()) pkgComment = pkgName + " " + pkgComment;
    pkgComment = pkgComment.trimmed();
    pkgDescription = pkgComment;

    PackageListData pld =
        PackageListData(pkgName, pkgOrigin, pkgVersion, pkgComment, pkgStatus, pkgInstalledSize, pkgDownloadedSize);

    res->append(pld);
  }

  return res;
}

/*
 * Retrieves the list of all AUR packages in the database (installed + non-installed)
 * given the search parameter
 */
QList<PackageListData> * Package::getRemotePackageList(const QString& searchString)
{
  QList<PackageListData> * res = new QList<PackageListData>();
  QList<PackageListData> * resComment = new QList<PackageListData>();
  QStringList packageCache;

  if (searchString.isEmpty())
    return res;

  /*QString pkgList = UnixCommand::getRemotePackageList(searchString, false);
  QStringList packageTuples = pkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  res = parsePackageTuple(packageTuples, packageCache);*/

  QString pkgListComment = UnixCommand::getRemotePackageList(searchString, true);
  QStringList packageTuplesComment = pkgListComment.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  resComment = parsePackageTuple(packageTuplesComment, packageCache);

  foreach(PackageListData pld, *resComment)
  {
    res->append(pld);
  }

  return res;
}

/*
 * Given a QString containing the output of pacman -Si/Qi (pkgInfo),
 * this method returns the contents of the given field (ex: description)
 */
QString Package::extractFieldFromInfo(const QString &field, const QString &pkgInfo)
{
  int fieldPos = pkgInfo.indexOf(field);
  int fieldEnd, fieldEnd2;
  QString aux;

  if (field == "architecture")
  {
    fieldPos = pkgInfo.indexOf(":", fieldPos+1);
    fieldPos+=2;
    aux = pkgInfo.mid(fieldPos);
    fieldEnd = aux.indexOf('\n');
    aux = aux.left(fieldEnd).trimmed();
  }
  else if (fieldPos > 0)
  {
    if(field == "Options")
    {
      fieldPos = pkgInfo.indexOf(":", fieldPos+1);
      fieldPos+=2;
      aux = pkgInfo.mid(fieldPos);

      fieldEnd = aux.indexOf("Shared Libs required");
      fieldEnd2 = aux.indexOf("Shared Libs provided");
      if (fieldEnd2 == -1)
        fieldEnd2 = aux.indexOf("Annotations");

      if((fieldEnd > fieldEnd2 && fieldEnd2 != -1) ||
         (fieldEnd == -1 && fieldEnd2 != -1)) fieldEnd = fieldEnd2;

      aux = aux.left(fieldEnd).trimmed();
      aux = aux.replace("\n", "<br>");
    }
    else
    {
      fieldPos = pkgInfo.indexOf(":", fieldPos+1);
      fieldPos+=2;
      aux = pkgInfo.mid(fieldPos);
      fieldEnd = aux.indexOf('\n');
      aux = aux.left(fieldEnd).trimmed();
    }
  }

  return aux;
}

/*
 * Retrieves "Name" field of the given package information string represented by pkgInfo
 */
QString Package::getName(const QString& pkgInfo)
{
  return extractFieldFromInfo("Name", pkgInfo);
}

/*
 * Retrieves "Version" field of the given package information string represented by pkgInfo
 */
QString Package::getVersion(const QString &pkgInfo)
{
  return extractFieldFromInfo("Version", pkgInfo);
}

/*
 * Retrieves "Version" field of the given package name
 */
QString Package::getVersionByName(const QString &pkgName)
{
  QString auxName = UnixCommand::getFieldFromLocalPackage("pkgver", pkgName);
  int dash = auxName.lastIndexOf("-");
  return auxName.right(auxName.length() - (dash+1));
}

/*
 * Retrieves "Repository" field of the given package information string represented by pkgInfo
 */
QString Package::getRepository(const QString &pkgInfo)
{
  return extractFieldFromInfo("Repository", pkgInfo);
}

/*
 * Retrieves "URL" field of the given package information string represented by pkgInfo
 */
QString Package::getURL(const QString &pkgInfo)
{
  QString URL = extractFieldFromInfo("homepage", pkgInfo);
  if (!URL.isEmpty())
    return makeURLClickable(URL);
  else
    return URL;
}

/*
 * Retrieves "Licenses" field of the given package information string represented by pkgInfo
 */
QString Package::getLicense(const QString &pkgInfo)
{
  return extractFieldFromInfo("license", pkgInfo);
}

/*
 * Retrieves "Groups" field of the given package information string represented by pkgInfo
 */
QString Package::getGroup(const QString &pkgInfo)
{
  return extractFieldFromInfo("Categories", pkgInfo);
}

/*
 * Retrieves "Provides" field of the given package information string represented by pkgInfo
 */
QString Package::getProvides(const QString &pkgInfo)
{
  return extractFieldFromInfo("Provides", pkgInfo);
}

/*
 * Retrieves "Depends On" field of the given package information string represented by pkgInfo
 */
QString Package::getDependsOn(const QString &pkgInfo)
{
  return extractFieldFromInfo("Depends On", pkgInfo);
}

/*
 * Retrieves "Optional Deps" field of the given package information string represented by pkgInfo
 */
QString Package::getOptDepends(const QString &pkgInfo)
{
  return extractFieldFromInfo("Optional Deps", pkgInfo);
}

/*
 * Retrieves "Conflicts With" field of the given package information string represented by pkgInfo
 */
QString Package::getConflictsWith(const QString &pkgInfo)
{
  return extractFieldFromInfo("Conflicts With", pkgInfo);
}

/*
 * Retrieves "Replaces" field of the given package information string represented by pkgInfo
 */
QString Package::getReplaces(const QString &pkgInfo)
{
  return extractFieldFromInfo("Replaces", pkgInfo);
}

/*
 * Retrieves "RequiredBy" field of the given package information string represented by pkgInfo
 */
QString Package::getRequiredBy(const QString &pkgInfo)
{
  return extractFieldFromInfo("Required By", pkgInfo);
}

/*
 * Retrieves "OptionalFor" field of the given package information string represented by pkgInfo
 */
QString Package::getOptionalFor(const QString &pkgInfo)
{
  return extractFieldFromInfo("Optional For", pkgInfo);
}

/*
 * Retrieves "Packager" field of the given package information string represented by pkgInfo
 */
QString Package::getPackager(const QString &pkgInfo)
{
  return extractFieldFromInfo("Packager", pkgInfo);
}

/*
 * Retrieves "Maintainer" field of the given package information string represented by pkgInfo
 */
QString Package::getMaintainer(const QString &pkgInfo)
{
  return extractFieldFromInfo("maintainer", pkgInfo);
}

/*
 * Retrieves "Architecture" field of the given package information string represented by pkgInfo
 */
QString Package::getArch(const QString &pkgInfo)
{
  return extractFieldFromInfo("architecture", pkgInfo);
}

/*
 * Retrieves "Build Date" field of the given package information string represented by pkgInfo
 */
QString Package::getBuildDate(const QString &pkgInfo)
{
  return extractFieldFromInfo("build-date", pkgInfo);
}

/*
 * Retrieves "Install Date" field of the given package information string represented by pkgInfo
 */
QString Package::getInstallDate(const QString &pkgInfo)
{
  return extractFieldFromInfo("install-date", pkgInfo);
}

/*
 * Retrieves "Download Size" field of the given package information string represented by pkgInfo
 */
double Package::getDownloadSize(const QString &pkgInfo)
{
  Q_UNUSED(pkgInfo)
  return 0;
}

/*
 * Retrieves "Download Size" field of the given package information string represented by pkgInfo
 */
QString Package::getDownloadSizeAsString(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo("filename-size", pkgInfo);
  return aux;
}

/*
 * Retrieves "Installed Size" field of the given package information string represented by pkgInfo
 */
double Package::getInstalledSize(const QString &pkgInfo)
{
  Q_UNUSED(pkgInfo)
  return 0;
}

/*
 * Retrieves "Installed Size" field of the given package information string represented by pkgInfo
 */
QString Package::getInstalledSizeAsString(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo("installed_size", pkgInfo);
  return aux;
}

/*
 * Retrieves "Options" field of the given package information string represented by pkgInfo
 */
QString Package::getOptions(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo("Options", pkgInfo);
  return aux;
}

/**
 * This function was copied from ArchLinux Pacman project
 *
 * Compare alpha and numeric segments of two versions.
 * return 1: a is newer than b
 *        0: a and b are the same version
 *       -1: b is newer than a
 */
int Package::rpmvercmp(const char *a, const char *b){
  char oldch1, oldch2;
  char *str1, *str2;
  char *ptr1, *ptr2;
  char *one, *two;
  int rc;
  int isnum;
  int ret = 0;

  /* easy comparison to see if versions are identical */
  if(strcmp(a, b) == 0) return 0;

  str1 = strdup(a);
  str2 = strdup(b);

  one = ptr1 = str1;
  two = ptr2 = str2;

  /* loop through each version segment of str1 and str2 and compare them */
  while (*one && *two) {
    while (*one && !isalnum((int)*one)) one++;
    while (*two && !isalnum((int)*two)) two++;

    /* If we ran to the end of either, we are finished with the loop */
    if (!(*one && *two)) break;

    /* If the separator lengths were different, we are also finished */
    if ((one - ptr1) != (two - ptr2)) {
      return (one - ptr1) < (two - ptr2) ? -1 : 1;
    }

    ptr1 = one;
    ptr2 = two;

    /* grab first completely alpha or completely numeric segment */
    /* leave one and two pointing to the start of the alpha or numeric */
    /* segment and walk ptr1 and ptr2 to end of segment */
    if (isdigit((int)*ptr1)) {
      while (*ptr1 && isdigit((int)*ptr1)) ptr1++;
      while (*ptr2 && isdigit((int)*ptr2)) ptr2++;
      isnum = 1;
    } else {
      while (*ptr1 && isalpha((int)*ptr1)) ptr1++;
      while (*ptr2 && isalpha((int)*ptr2)) ptr2++;
      isnum = 0;
    }

    /* save character at the end of the alpha or numeric segment */
    /* so that they can be restored after the comparison */
    oldch1 = *ptr1;
    *ptr1 = '\0';
    oldch2 = *ptr2;
    *ptr2 = '\0';

    /* this cannot happen, as we previously tested to make sure that */
    /* the first string has a non-null segment */
    if (one == ptr1) {
      ret = -1;       /* arbitrary */
      goto cleanup;
    }

    /* take care of the case where the two version segments are */
    /* different types: one numeric, the other alpha (i.e. empty) */
    /* numeric segments are always newer than alpha segments */
    /* XXX See patch #60884 (and details) from bugzilla #50977. */
    if (two == ptr2) {
      ret = isnum ? 1 : -1;
      goto cleanup;
    }

    if (isnum) {
      /* this used to be done by converting the digit segments */
      /* to ints using atoi() - it's changed because long  */
      /* digit segments can overflow an int - this should fix that. */

      /* throw away any leading zeros - it's a number, right? */
      while (*one == '0') one++;
      while (*two == '0') two++;

      /* whichever number has more digits wins */
      if (strlen(one) > strlen(two)) {
        ret = 1;
        goto cleanup;
      }
      if (strlen(two) > strlen(one)) {
        ret = -1;
        goto cleanup;
      }
    }

    /* strcmp will return which one is greater - even if the two */
    /* segments are alpha or if they are numeric.  don't return  */
    /* if they are equal because there might be more segments to */
    /* compare */
    rc = strcmp(one, two);
    if (rc) {
      ret = rc < 1 ? -1 : 1;
      goto cleanup;
    }

    /* restore character that was replaced by null above */
    *ptr1 = oldch1;
    one = ptr1;
    *ptr2 = oldch2;
    two = ptr2;
  }

  /* this catches the case where all numeric and alpha segments have */
  /* compared identically but the segment separating characters were */
  /* different */
  if ((!*one) && (!*two)) {
    ret = 0;
    goto cleanup;
  }

  /* the final showdown. we never want a remaining alpha string to
         * beat an empty string. the logic is a bit weird, but:
         * - if one is empty and two is not an alpha, two is newer.
         * - if one is an alpha, two is newer.
         * - otherwise one is newer.
         * */
  if ( (!*one && !isalpha((int)*two))
       || isalpha((int)*one) ) {
    ret = -1;
  } else {
    ret = 1;
  }

cleanup:
  free(str1);
  free(str2);
  return ret;
}

/*
 * Retrieves "Description" field of the given package information string represented by pkgInfo
 */
QString Package::getDescription(const QString &pkgInfo)
{
  return extractFieldFromInfo("Description", pkgInfo);
}

/*
 * Retrieves "Comment" field of the given package information string represented by pkgInfo
 */
QString Package::getComment(const QString &pkgInfo)
{
  return extractFieldFromInfo("Comment", pkgInfo);
}

/*
 * Retrieves all information for a given package name
 */
PackageInfoData Package::getInformation(const QString &pkgName, bool foreignPackage)
{
  PackageInfoData res;
  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, foreignPackage);

  res.name = pkgName;
  res.version = getVersion(pkgInfo);
  res.url = getURL(pkgInfo);
  res.license = getLicense(pkgInfo);
  res.group = getGroup(pkgInfo);
  res.maintainer = getMaintainer(pkgInfo);
  res.arch = getArch(pkgInfo);
  res.buildDate = getBuildDate(pkgInfo);
  res.installDate = getInstallDate(pkgInfo);
  res.description = getDescription(pkgInfo);
  res.comment = getComment(pkgInfo);
  res.downloadSize = 0; //getDownloadSize(pkgInfo);
  res.installedSize = 0; //getInstalledSize(pkgInfo);
  res.downloadSizeAsString = getDownloadSizeAsString(pkgInfo);
  res.installedSizeAsString = getInstalledSizeAsString(pkgInfo);
  res.options = getOptions(pkgInfo);

  return res;
}

/*
 * Helper to get only the Download Size field of package information
 */
double Package::getDownloadSizeDescription(const QString &pkgName)
{
  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, false);
  return getDownloadSize(pkgInfo);
}

/*
 * Helper to get only the Description field of package information, for use in tooltips
 */
QString Package::getInformationDescription(const QString &pkgName, bool foreignPackage)
{
  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, foreignPackage);
  return getDescription(pkgInfo);
}

/*
 * Helper to get only the Installed Size field of package information, for use in tooltips
 */
QString Package::getInformationInstalledSize(const QString &pkgName, bool foreignPackage)
{
  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, foreignPackage);
  return kbytesToSize(getInstalledSize(pkgInfo));
}


/*
 * Format dependencies list
 */
QString Package::formatDependencies(const QString &dependenciesList, PackageAnchor pkgAnchorState)
{
  QStringList pkgList;
  QString res;

  pkgList = dependenciesList.split("\n", QString::SkipEmptyParts);
  pkgList.sort();

  foreach(QString dependency, pkgList)
  {
    if (pkgAnchorState == ectn_WITH_PACKAGE_ANCHOR)
      res += "<a href=\"goto:" + dependency + "\">" + dependency + "</a> ";
    else
      res += dependency + " ";
  }

  return res.trimmed();
}


/*
 * Retrieves the dependencies list of packages for the given pkgName
 */
QString Package::getDependencies(const QString &pkgName, PackageAnchor pkgAnchorState)
{
  QString aux = UnixCommand::getDependenciesList(pkgName);

  return formatDependencies(aux, pkgAnchorState);
}

/*
 * Navigate thru directories to build a hierarquic directory list
 */
void Package::navigateThroughDirs(QStringList parts, QStringList& auxList, int ind)
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
 * Retrieves the file list content of the given package
 */
QStringList Package::getContents(const QString& pkgName, bool isInstalled)
{
  QByteArray result;

  result = UnixCommand::getPackageContentsUsingXBPS(pkgName, isInstalled);

  QString aux(result);
  QStringList fileList = aux.split("\n", QString::SkipEmptyParts);

  //Let's change that listing a bit...
  QStringList auxList;
  foreach(QString file, fileList)
  {
    QStringList parts = file.split("/", QString::SkipEmptyParts);
    navigateThroughDirs(parts, auxList, 0);
  }

  fileList = fileList + auxList;
  fileList.sort();

  return fileList;
}

/*
 * Retrieves the list of optional dependencies of the given package
 */
QStringList Package::getOptionalDeps(const QString &pkgName)
{
  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, false);
  QString aux = Package::getOptDepends(pkgInfo);
  QStringList result = aux.split("<br>", QString::SkipEmptyParts);
  result.removeAll("None");

  return result;
}

/*
 * Returns a modified RegExp-based string given the string entered by the user
 */
QString Package::parseSearchString(QString searchStr, bool exactMatch)
{
  if (searchStr.indexOf("*.") == 0){
    searchStr = searchStr.remove(0, 2);
    searchStr.insert(0, "\\S+\\.");
  }

  if (searchStr.indexOf("*") == 0){
    searchStr = searchStr.remove(0, 1);
    searchStr.insert(0, "\\S+");
  }

  if (searchStr.endsWith("*")){
    searchStr.remove(searchStr.length()-1, 1);
    searchStr.append("\\S*");
  }

  if (searchStr.indexOf("^") == -1 && searchStr.indexOf("\\S") != 0){
    if (!exactMatch) searchStr.insert(0, "\\S*");
    else searchStr.insert(0, "^");
  }

  if (searchStr.indexOf("$") == -1){
    if (!exactMatch && !searchStr.endsWith("\\S*")) searchStr.append("\\S*");
    else searchStr.append("$");
  }

  searchStr.replace("?", ".");
  return searchStr;
}

/*
 * Extracts the real pkg name of the given anchor
 */
QString Package::extractPkgNameFromAnchor(const QString &pkgName)
{
  int sign = pkgName.indexOf(">");

  if (sign == -1)
  {
    sign = pkgName.indexOf("<");
  }
  if (sign == -1)
  {
    sign = pkgName.indexOf("=");
  }

  QString res = pkgName.left(sign);

  res.remove("%3E");
  res.remove(QRegularExpression("-[0-9.]+$")); //CHANDED - WARNING!!!
  res.remove(QRegularExpression("-[0-9.]+_[0-9.]+$")); //CHANDED - WARNING!!!

  return res;
}

/*
 * Checks if this system has the core.db
 */
bool Package::hasXBPSDatabase()
{
  static bool done = false;
  static bool answer = false;

  if (!done)
  {
    if (UnixCommand::getLinuxDistro() == ectn_TRIDENT || UnixCommand::getLinuxDistro() == ectn_VOID)
    {
      QDir info;
      info.setPath("/var/db/xbps/");
      QStringList filters;
      filters << ctn_XBPS_CORE_DB_FILE;

      QStringList entries = info.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);

      if (entries.count() >= 1)
        answer = true;
    }

    done = true;
  }

  return answer;
}

/*
 * Returns true if the package is included in the forbidden removal list
 */
bool Package::isForbidden(const QString pkgName)
{
  QStringList forbiddenPkgs = { "base-files", "base-system", "libxbps", "xbps", "xbps-triggers" };
  return forbiddenPkgs.contains(pkgName);
}
