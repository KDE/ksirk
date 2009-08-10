/***************************************************************************
                          newGameSummaryWidget.cpp  -  description
                             -------------------
    begin                : Fri Jul 31 2009
    copyright            : (C) 2009 by Gaël de Chalendar
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

#include "newGameSummaryWidget.h"

#include <KLocale>
#include <KDebug>

namespace Ksirk
{

NewGameSummaryWidget::NewGameSummaryWidget(QWidget *parent) :
  QWidget(parent),
  Ui::NewGameSummary()
{
  kDebug();
  setupUi(this);
}
  
NewGameSummaryWidget::~NewGameSummaryWidget()
{
}

void NewGameSummaryWidget::slotStart()
{
}

void NewGameSummaryWidget::slotPrevious()
{
}

void NewGameSummaryWidget::slotCancel()
{
}

}

