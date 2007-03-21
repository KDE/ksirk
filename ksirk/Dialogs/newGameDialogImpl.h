/***************************************************************************
                          newGameDialogImpl.h  -  description
                             -------------------
    begin                : Wed Feb 23 2005
    copyright            : (C) 2005 by Gaël de Chalendar
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
  NewGameDialogImpl(
      bool& ok,
      unsigned int& nbPlayers, 
      unsigned int maxPlayers, 
      QString& skin,
      bool& networkGame,
      bool& useGoals,
      QWidget *parent=0);

  virtual ~NewGameDialogImpl();

private:
  bool& m_ok;
  unsigned int& m_nbPlayers;
  QString& m_skin;
  bool& m_networkGame;
  bool& m_useGoals;
  QMap<QString, GameLogic::ONU*> m_worlds;
  
/** 
    * Fills the skins combo with skins dir names found in the Ksirk app data dir
    * @todo Use skins names instead of dir names
    */
  void fillSkinsCombo();
    
public slots:
    virtual void slotOK();
    virtual void slotCancel();
    virtual void slotHelp();
    void slotSkinChanged(int skinNum);
};

} // closing namespace Ksirk

#endif // KSIRK_NEWGAMEDIALOGIMPL_H

