/* This file is part of KsirK.
   Copyright (C) 2006 Gael de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.

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
#include "ksirk_debug.h"

using namespace Ksirk;

KsirkConfigurationDialog::KsirkConfigurationDialog (
              QWidget *parent, const char *name, KConfigSkeleton *config) :
      KConfigDialog (parent, name, config),
      m_widget(new Ui::KsirkPreferencesWidget())

{
  setModal(true);
  QWidget* w = new QWidget();
  m_widget->setupUi(w);

  addPage( w, i18n("Preferences"), "games-config-options"/*, i18n("Preferences"), false*/);
  connect(m_widget->kcfg_showArmiesNumbers, &QCheckBox::toggled, this, &KsirkConfigurationDialog::armiesNumberShowingChanged);
  connect(this, &KConfigDialog::settingsChanged, this, &KsirkConfigurationDialog::updateSettings);
  setHelp(QStringLiteral("configuration"), QStringLiteral("ksirk"));
}

KsirkConfigurationDialog::~KsirkConfigurationDialog ()
{
  delete m_widget;
}

void KsirkConfigurationDialog::updateSettings()
{
  qCDebug(KSIRK_LOG);
  KsirkSettings::setSpritesSpeed(m_widget->kcfg_spritesSpeed->value());
  KsirkSettings::setSoundEnabled(m_widget->kcfg_soundEnabled->isChecked());
  KsirkSettings::setHelpEnabled(m_widget->kcfg_helpEnabled->isChecked());
  KsirkSettings::setShowArmiesNumbers(m_widget->kcfg_showArmiesNumbers->isChecked());
  KsirkSettings::self()->save();
}

void KsirkConfigurationDialog::updateWidgets()
{
  qCDebug(KSIRK_LOG);
  m_widget->kcfg_spritesSpeed->setValue(KsirkSettings::spritesSpeed());
  m_widget->kcfg_soundEnabled->setChecked(KsirkSettings::soundEnabled());
  m_widget->kcfg_helpEnabled->setChecked(KsirkSettings::helpEnabled());
  m_widget->kcfg_showArmiesNumbers->setChecked(KsirkSettings::showArmiesNumbers());
}

#include "moc_ksirkConfigDialog.cpp"
