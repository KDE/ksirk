/***************************************************************************
                          kdialogsetupjoueur.h  -  description
                             -------------------
    begin                : Thu Jul 19 2001
    copyright            : (C) 2001 by Gaël de Chalendar
    email                : Gael.de.Chalendar@free.fr
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


#ifndef KWAITEDPLAYERSETUPDIALOG_H
#define KWAITEDPLAYERSETUPDIALOG_H

#include "ui_qwaitedplayersetupdialog.h"

#include <qwidget.h>

#include <map>

#include "GameLogic/player.h"

namespace Ksirk
{


/**
  * This is the implementation of the player configuration dialog made with
  * QT Designer
  * @author Gaël de Chalendar
  */
class KWaitedPlayerSetupDialog : public QDialog, public Ui::QWaitedPlayerSetupDialog
{
  Q_OBJECT
public:
    KWaitedPlayerSetupDialog(GameLogic::GameAutomaton* automaton,
                             QString& password,
                              int& result,
                             QWidget *parent=0);
  
  ~KWaitedPlayerSetupDialog() override;

  inline int result() {return m_result;}

private:
  GameLogic::GameAutomaton* m_automaton;
  QString& m_password;
  int& m_result;
/** Fills the waited players combo with players names, nations names and flag images */
  void fillWaitedPlayersCombo();
    
public slots:
    virtual void slotOK();
};

}

#endif // KWAITEDPLAYERSETUPDIALOG_H
