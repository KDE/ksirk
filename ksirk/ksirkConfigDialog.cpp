/* This file is part of KsirK.
   Copyright (C) 2006 Gael de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/


#include "ksirkConfigDialog.h"
#include "ksirksettings.h"
#include "ui_preferences.h"

#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <ktabwidget.h>
#include <kedittoolbar.h>
#include <kdebug.h>

#include <kstandardaction.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfigdialog.h>

#include <kapplication.h>

#include <qslider.h>
#include <qcheckbox.h>
#include <iostream>

using namespace Ksirk;

KsirkConfigurationDialog::KsirkConfigurationDialog (
              QWidget *parent, const char *name, KConfigSkeleton *config, 
              FaceType dialogType, ButtonCodes dialogButtons, 
              ButtonCode defaultButton, bool modal) : 
  KConfigDialog (parent, name, config, dialogType, dialogButtons, 
      defaultButton, modal) , m_changed(false), 
      m_widget(new Ui::KsirkPreferencesWidget())

{
     QWidget* w = new QWidget();
     m_widget->setupUi(w);

 
  addPage( w, i18n("Preferences"), "preferences", i18n("Preferences"), false); 
  // below, connection to activate the apply button
//   connect(m_widget->reloadOnChangeMode, SIGNAL(clicked(int)), this, SLOT(settingChanged(int)));
}

KsirkConfigurationDialog::~KsirkConfigurationDialog () 
{
}

void KsirkConfigurationDialog::settingChanged(int)
{
//   std::cerr << "KsirkConfigurationDialog::settingChanged" << std::endl;
  m_changed = true;
  settingsChangedSlot();
  updateButtons ();
}

bool KsirkConfigurationDialog::hasChanged()
{
//   std::cerr << "KsirkConfigurationDialog::hasChanged" << std::endl;
  return m_changed;
}

void KsirkConfigurationDialog::updateSettings()
{
//   std::cerr << "KsirkConfigurationDialog::updateSettings" << std::endl;
  m_changed = false;
  KsirkSettings::setSpritesSpeed(m_widget->spritesSpeed->value());
  KsirkSettings::setSoundEnabled(m_widget->soundEnabled->isChecked());
  KsirkSettings::writeConfig();
}

void KsirkConfigurationDialog::updateWidgets()
{
//   std::cerr << "KsirkConfigurationDialog::updateWidgets" << std::endl;

  m_changed = false;
  m_widget->spritesSpeed->setValue(KsirkSettings::spritesSpeed());
  m_widget->soundEnabled->setChecked(KsirkSettings::soundEnabled());
}

#include "ksirkConfigDialog.moc"
