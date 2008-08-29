/***************************************************************************
                          newGameDialogImpl.h  -  description
                             -------------------
    begin                : Wed Feb 23 2005
    copyright            : (C) 2005 by Gael de Chalendar
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
#define KDE_NO_COMPAT


#include "ui_newGameDialog.h"

#include <qmap.h>

#ifndef KSIRK_NEWGAMEDIALOGIMPL_H
#define KSIRK_NEWGAMEDIALOGIMPL_H

namespace Ksirk 
{
namespace GameLogic
{
    class ONU;
    class GameAutomaton;
}

/**
  * This is the implementation of the new game configuration dialog made with
  * QT Designer
  * @author Gaël de Chalendar
  */
class NewGameDialogImpl : public QDialog, public Ui::NewGameDialog
{
  Q_OBJECT
public:
  NewGameDialogImpl(QWidget *parent=0);

  void init(GameLogic::GameAutomaton* automaton,
             unsigned int maxPlayers,
             const QString& skin);

  virtual ~NewGameDialogImpl();

public Q_SLOTS:
    virtual void slotOK();
    virtual void slotCancel();
    virtual void slotHelp();
    void slotSkinChanged(int skinNum);
    void slotGHNS();

Q_SIGNALS:
  void newGameOK(unsigned int nbPlayers, const QString& skin, bool networkGame, bool useGoals);
  void newGameKO();

private:
  /**
  * Fills the skins combo with skins dir names found in the Ksirk app data dir
  * @todo Use skins names instead of dir names
  */
  void fillSkinsCombo();
  
  GameLogic::GameAutomaton* m_automaton;
  unsigned int m_nbPlayers;
  QString m_skin;
  bool m_networkGame;
  bool m_useGoals;
  QMap<QString, GameLogic::ONU*> m_worlds;

};

} // closing namespace Ksirk

#endif // KSIRK_NEWGAMEDIALOGIMPL_H

