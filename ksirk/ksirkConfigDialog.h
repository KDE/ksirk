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


#ifndef _KSIRKCONFIGDIALOG_H_
#define _KSIRKCONFIGDIALOG_H_

#include <kconfigdialog.h>
#include <KConfigSkeleton>
namespace Ui
{
  class KsirkPreferencesWidget;
}

/**
 * This is the KsirK configuration dialog. Based on KConfigDialog.
 *
 * @short Configuration dialog
 * @author Gaël de Chalendar (aka Kleag) <kleag@free.fr>
 */
class KsirkConfigurationDialog : public KConfigDialog
{
  Q_OBJECT
public:
  /**
    * Constructor
    */
  KsirkConfigurationDialog (
                QWidget *parent, const char *name, KConfigSkeleton *config, 
                FaceType faceType=List, 
                ButtonCodes dialogButtons=Default|Ok|Apply|Cancel|Help, 
                ButtonCode defaultButton=Ok, bool modal=false);

  /** Destructor */
  virtual ~KsirkConfigurationDialog ();

Q_SIGNALS:
  void armiesNumberShowingChanged(int);

protected Q_SLOTS:
  virtual void updateSettings();
  virtual void updateWidgets();
  void settingChanged(int);
//   void slotArmiesNumberChanged(int);
  
protected:
  virtual bool hasChanged();

  bool m_changed;
  Ui::KsirkPreferencesWidget*  m_widget;
};

#endif
