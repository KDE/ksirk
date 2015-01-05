/* This file is part of KsirK.
 *   Copyright (C) 2007 Gael de Chalendar <kleag@free.fr>
 * 
 *   KsirK is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation, version 2.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 */

#include "restartOrExitDialogImpl.h"

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qradiobutton.h>
#include <qlabel.h>

namespace Ksirk
{

RestartOrExitDialogImpl::RestartOrExitDialogImpl(
      const QString& label,
      QWidget *parent) :
  QDialog(parent),
  Ui::RestartOrExitDialog()
{
  setupUi(this);
  messageLabel->setText(label);
  messageLabel->adjustSize();
  QObject::connect((const QObject *)doNothingButton, SIGNAL(clicked()), this, SLOT(slotDoNothing()) );
  QObject::connect((const QObject *)exitButton, SIGNAL(clicked()), this, SLOT(slotExit()) );
  QObject::connect((const QObject *)newGameButton, SIGNAL(clicked()), this, SLOT(slotNewGame()) );
  adjustSize();
}

RestartOrExitDialogImpl::~RestartOrExitDialogImpl()
{
}

void RestartOrExitDialogImpl::slotNewGame()
{
  kDebug();
  close();
}

void RestartOrExitDialogImpl::slotExit()
{
  kDebug();
  close();
}

/** @todo implements a help */
void RestartOrExitDialogImpl::slotDoNothing()
{
  kDebug();
  close();
}

}


