/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2015 LXQt team
 * Authors:
 *   Palo Kisa <palo.kisa@gmail.com>
 *
 * Copyright: 2020 OctoXBPS team
 * Authors:
 *   Alexandre Albuquerque Arnt
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

/*
 * This code is adapted from LXQt version 0.14.1
 *
 * It is not using LXQt libs and is intended to act as the *only* sudo tool supported in OctoXBPS
 */

#include "sudo.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char **argv)
{
  QApplication app(argc, argv, true);
  app.setQuitOnLastWindowClosed(false);
  app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

  QTranslator translator;
  if (translator.load(QLocale(), QStringLiteral("lxqt-sudo"), QStringLiteral("_"), QStringLiteral(":/translations")))
    app.installTranslator(&translator);

  Sudo s;
  return s.main();
}
