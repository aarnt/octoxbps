/*
* This file is part of OctoXBPS, an open-source GUI for XBPS.
* Copyright (C) 2013 Alexandre Albuquerque Arnt
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

#include "../src/settingsmanager.h"
#include "../src/strconstants.h"
#include "optionsdialog.h"
#include "ui_optionsdialog.h"

OptionsDialog::OptionsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::OptionsDialog)
{
  ui->setupUi(this);
  init();
}

OptionsDialog::~OptionsDialog()
{
  delete ui;
}

/*
 * Here we read the values from ~/.config/octopi.conf...
 */
void OptionsDialog::init()
{
  connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  setWindowTitle(StrConstants::getNotifierOptionsDialogTitle());
  ui->groupBox->setTitle(StrConstants::getNotiferSetupDialogGroupBoxTitle());
  ui->rbOnceADay->setText(StrConstants::getOnceADay());
  ui->rbOnceADayAt->setText(StrConstants::getOnceADayAt());
  ui->lblOnceADayAt->setText(StrConstants::getOnceADayAtDesc());
  ui->rbOnceEvery->setText(StrConstants::getOnceEvery());
  ui->lblOnceEvery->setText(StrConstants::getOnceEveryDesc().arg(5).arg(44640));
  ui->rbNever->setText(StrConstants::getNever());

  connect(ui->rbOnceADay, SIGNAL(clicked()), this, SLOT(selectOnceADay()));
  connect(ui->rbOnceADayAt, SIGNAL(clicked()), this, SLOT(selectOnceADayAt()));
  connect(ui->rbOnceEvery, SIGNAL(clicked()), this, SLOT(selectOnceEvery()));

  //First, which radio button do we select?
  int syncDbInterval = SettingsManager::getSyncDbInterval();
  int syncDbHour = SettingsManager::getSyncDbHour();
  bool useSyncDbInterval = false;
  bool useSyncDbHour = false;

  //User does NOT want to check for updates!
  if (syncDbInterval == -2)
  {
    selectNever();
    return;
  }
  else if (syncDbInterval == -1)
  {
    ui->spinOnceEvery->setValue(5);
  }
  else if (syncDbInterval != -1)
  {
    ui->spinOnceEvery->setValue(syncDbInterval);
    useSyncDbInterval = true;    
  }
  if (syncDbHour == -1)
  {
    ui->spinOnceADayAt->setValue(0);
  }
  else if (syncDbHour != -1)
  {
    ui->spinOnceADayAt->setValue(syncDbHour);
    useSyncDbHour = true;
  }

  if (useSyncDbInterval)
  {
    ui->rbOnceEvery->setChecked(true);
    selectOnceEvery();
  }
  else if (useSyncDbHour)
  {
    ui->rbOnceADayAt->setChecked(true);
    selectOnceADayAt();
  }
  else //We are using just "Once a day"!!!
  {
    ui->rbOnceADay->setChecked(true);
    selectOnceADay();
  }
}

/*
 * Every time user clicks on OK button, we save his choice.
 */
void OptionsDialog::accept()
{
  saveChanges();
  QDialog::accept();
}

/*
 * Here we save the changes back into ~/.config/octopi.conf...
 */
void OptionsDialog::saveChanges()
{
  if (ui->rbOnceADay->isChecked())
  {
    SettingsManager::setSyncDbHour(-1);
    SettingsManager::setSyncDbInterval(-1);
  }
  else if (ui->rbOnceADayAt->isChecked())
  {
    SettingsManager::setSyncDbHour(ui->spinOnceADayAt->value());
    SettingsManager::setSyncDbInterval(-1);
  }
  else if (ui->rbOnceEvery->isChecked())
  {
    SettingsManager::setSyncDbInterval(ui->spinOnceEvery->value());
  }
  else if (ui->rbNever->isChecked())
  {
    SettingsManager::setSyncDbInterval(-2);
  }
}

/*
 * Whenever user selects the first radio button, we have to disable some widgets
 */
void OptionsDialog::selectOnceADay()
{
  ui->rbOnceADay->setChecked(true);
  ui->spinOnceADayAt->setEnabled(false);
  ui->spinOnceEvery->setEnabled(false);
  ui->rbOnceADayAt->setChecked(false);
  ui->rbOnceEvery->setChecked(false);
  ui->rbNever->setChecked(false);
}

/*
 * Whenever user selects the second radio button, we have to disable some widgets
 */
void OptionsDialog::selectOnceADayAt()
{
  ui->rbOnceADayAt->setChecked(true);
  ui->spinOnceADayAt->setEnabled(true);
  ui->spinOnceEvery->setEnabled(false);
  ui->rbOnceADay->setChecked(false);
  ui->rbOnceEvery->setChecked(false);
  ui->rbNever->setChecked(false);
}

/*
 * Whenever user selects the third radio button, we have to disable some widgets
 */
void OptionsDialog::selectOnceEvery()
{
  ui->rbOnceEvery->setChecked(true);
  ui->spinOnceADayAt->setEnabled(false);
  ui->spinOnceEvery->setEnabled(true);
  ui->rbOnceADay->setChecked(false);
  ui->rbOnceADayAt->setChecked(false);
  ui->rbNever->setChecked(false);
}

/*
 * Whenever user selects the forth radio button, we have to disable some widgets
 */
void OptionsDialog::selectNever()
{
  ui->rbOnceEvery->setChecked(false);
  ui->spinOnceADayAt->setEnabled(false);
  ui->spinOnceEvery->setEnabled(false);
  ui->rbOnceADay->setChecked(false);
  ui->rbOnceADayAt->setChecked(false);
  ui->rbNever->setChecked(true);
}
