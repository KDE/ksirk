/* This file is part of KsirK.
   Copyright (C) 2008 Gael de Chalendar <kleag@free.fr>

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


#include "ksirkSkinEditorConfigDialog.h"
#include "ksirkskineditorsettings.h"
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

using namespace KsirkSkinEditor;

KsirkSkinEditorConfigurationDialog::KsirkSkinEditorConfigurationDialog (
              QWidget *parent, const char *name, KConfigSkeleton *config, 
              FaceType dialogType, ButtonCodes dialogButtons, 
              ButtonCode defaultButton, bool modal) : 
      KConfigDialog (parent, name, config) , m_changed(false), 
      m_widget(new Ui::KsirkSkinEditorPreferencesWidget())

{
  setFaceType(dialogType);
  setButtons(dialogButtons);
  setDefaultButton(defaultButton);
  setModal(modal);
  QWidget* w = new QWidget();
  m_widget->setupUi(w);

 
  addPage( w, i18n("Preferences"), "games-config-options"/*, i18n("Preferences"), false*/);
  // below, connection to activate the apply button
//   connect(m_widget->reloadOnChangeMode, SIGNAL(clicked(int)), this, SLOT(settingChanged(int)));
}

KsirkSkinEditorConfigurationDialog::~KsirkSkinEditorConfigurationDialog ()
{
}

void KsirkSkinEditorConfigurationDialog::settingChanged(int)
{
  kDebug();
  m_changed = true;
  //settingsChangedSlot();
  //updateButtons ();
}

bool KsirkSkinEditorConfigurationDialog::hasChanged()
{
  kDebug();
  return m_changed;
}

void KsirkSkinEditorConfigurationDialog::updateSettings()
{
  kDebug();
  m_changed = false;
  KsirkSkinEditorSettings::self()->writeConfig();
}

void KsirkSkinEditorConfigurationDialog::updateWidgets()
{
  kDebug();

  m_changed = false;
}

#include "ksirkSkinEditorConfigDialog.moc"
