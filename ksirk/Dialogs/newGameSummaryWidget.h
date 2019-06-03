/***************************************************************************
                          newGameSummary.h  -  description
                             -------------------
    begin                : Fri Jul 31 2009
    copyright            : (C) 2009 by Gaël de Chalendar
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/
#define KDE_NO_COMPAT


#ifndef NEWGAMESUMMARY_H
#define NEWGAMESUMMARY_H

#include "ui_newGameSummary.h"

#include <QWidget>
#include <QMap>


namespace Ksirk
{
  class KGameWindow;
  
namespace GameLogic
{
}

/**
  * This is the implementation of the player configuration dialog made with
  * QT Designer
  * @author Gael de Chalendar
  */
class NewGameSummaryWidget : public QWidget, public Ui::NewGameSummary
{
  Q_OBJECT
public:
  NewGameSummaryWidget(QWidget *parent=0);
  
  ~NewGameSummaryWidget() override;

  void show(KGameWindow* game);
Q_SIGNALS:
  void start();
  void previous();
  void cancel();
};

}

#endif // NEWGAMESUMMARY_H
