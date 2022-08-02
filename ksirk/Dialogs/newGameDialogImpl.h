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


#include "ui_newGameDialog.h"
#include "newgamesetup.h"

#include <QMap>

#ifndef KSIRK_NEWGAMEDIALOGIMPL_H
#define KSIRK_NEWGAMEDIALOGIMPL_H

namespace Ksirk 
{
namespace GameLogic
{
    class GameAutomaton;
}

/**
  * This is the implementation of the new game configuration dialog made with
  * QT Designer
  * @author Gaël de Chalendar
  */
class NewGameWidget : public QWidget, public Ui::NewGameDialog
{
  Q_OBJECT
public:
  explicit NewGameWidget(NewGameSetup* newGameSetup, QWidget *parent=nullptr);

  ~NewGameWidget() override;

  void init(const QString& skin, GameLogic::GameAutomaton::NetworkGameType netGameType);
  
  public Q_SLOTS:
    virtual void slotOK();
    virtual void slotCancel();
    void slotSkinChanged(int skinNum);
    void slotGHNS();
    void slotTcpPortEdited(int);
    void slotNbNetworkPlayersEdited(int);
    
Q_SIGNALS:
  void newGameOK();
  void newGameKO();

private:
  /**
  * Fills the skins combo with skins dir names found in the Ksirk app data dir
  * @todo Use skins names instead of dir names
  */
  void fillSkinsCombo();
  
  NewGameSetup* m_newGameSetup;
};

} // closing namespace Ksirk

#endif // KSIRK_NEWGAMEDIALOGIMPL_H

