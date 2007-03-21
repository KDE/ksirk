/* This file is part of KsirK.
   Copyright (C) 2006 Gaël de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef _KSIRKCONFIGDIALOG_H_
#define _KSIRKCONFIGDIALOG_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kconfigdialog.h>

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

protected slots:
  virtual void updateSettings();
  virtual void updateWidgets();
  void settingChanged(int);

protected:
  virtual bool hasChanged();

  bool m_changed;
  Ui::KsirkPreferencesWidget*  m_widget;
};

#endif
