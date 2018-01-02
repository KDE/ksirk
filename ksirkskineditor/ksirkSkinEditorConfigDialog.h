/* This file is part of KsirK.
   Copyright (C) 2008 Gael de Chalendar <kleag@free.fr>

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


#ifndef _KSIRKSKINEDITORCONFIGDIALOG_H_
#define _KSIRKSKINEDITORCONFIGDIALOG_H_

#include <kconfigdialog.h>
#include <KConfigSkeleton>
#include <QDialogButtonBox>
namespace Ui
{
  class KsirkSkinEditorPreferencesWidget;
}

/**
 * This is the KsirK Skin Editor configuration dialog. Based on KConfigDialog.
 *
 * @short Configuration dialog
 * @author Gaël de Chalendar (aka Kleag) <kleag@free.fr>
 */
class KsirkSkinEditorConfigurationDialog : public KConfigDialog
{
  Q_OBJECT
public:
  /**
    * Constructor
    */
  KsirkSkinEditorConfigurationDialog (
                QWidget *parent, const char *name, KConfigSkeleton *config, 
                FaceType faceType=List, 
                QDialogButtonBox::StandardButtons dialogButtons=QDialogButtonBox::RestoreDefaults|QDialogButtonBox::Ok|QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::Help, 
                QDialogButtonBox::StandardButton defaultButton=QDialogButtonBox::Ok, bool modal=false);

  /** Destructor */
  virtual ~KsirkSkinEditorConfigurationDialog ();

Q_SIGNALS:

protected Q_SLOTS:
  virtual void updateSettings();
  virtual void updateWidgets();
  void settingChanged(int);
  
protected:
  virtual bool hasChanged();

  bool m_changed;
  Ui::KsirkSkinEditorPreferencesWidget*  m_widget;
};

#endif
