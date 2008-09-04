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
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/
#define KDE_NO_COMPAT


#ifndef JOINGAMEDIALOG_H
#define JOINGAMEDIALOG_H

#include "ui_joingame.h"

#include <QDialog>

#include <QMap>

class KPlayer;

namespace Ksirk
{

namespace GameLogic
{
  class ONU;
  class GameAutomaton;
}

/**
  * This is the implementation of the player configuration dialog made with
  * QT Designer
  * @author Gael de Chalendar
  */
class JoinGameDialog : public QDialog, public Ui::JoinGameDialog
{
  Q_OBJECT
public:
  JoinGameDialog(GameLogic::GameAutomaton* automaton,
                  QString& host,
                  int& port,
                  QWidget *parent=0);
  
  virtual ~JoinGameDialog();

protected:
  void accept();
  void reject();
  
public Q_SLOTS:
  void slotNewJabberGame(const QString& nick,
                          const QString& host,
                          int port,
                          const QString& skin
                          );

private Q_SLOTS:
  void slotCellClicked(int row, int column);

private:
  GameLogic::GameAutomaton* m_automaton;
  QString &m_host;
  int& m_port;

};

}

#endif // KPLAYERSETUPDIALOG_H
