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


#ifndef KPLAYERSETUPDIALOG_H
#define KPLAYERSETUPDIALOG_H

#include "ui_qplayersetupdialog.h"

#include <qwidget.h>

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
class KPlayerSetupDialog : public QDialog, public Ui::QPlayerSetupDialog
{
  Q_OBJECT
public:
  KPlayerSetupDialog(GameLogic::GameAutomaton* automaton,
                     GameLogic::ONU* onu, unsigned int playerNumber,
                     QString &playerName,
                     bool network, QString& password,
                     bool &computerPlayer,
                     QMap< QString, QString >& nations,
                     QString & nationName,
                     QWidget *parent=0);
  
  virtual ~KPlayerSetupDialog();

protected:
  void reject();

private:
  GameLogic::GameAutomaton* m_automaton;
  QString &name;
  bool &computer;
  QString& m_nationName;
  /** list of nation name , flag file name pairs */
  QMap< QString, QString >& m_nations;
  GameLogic::ONU* m_onu;
  
  /** list of internationalized nations names. Used to retrive the internal name after choice in UI */
  QMap< QString, QString > m_nationsNames;
  unsigned int number ;
  
  QString& m_password;
  /** Fills the nations combo with nations names and flag images */
  void fillNationsCombo();

  bool testEmptyUserName(const QString& name) const;
  bool testUniqueUserName(const QString& name) const;
    
public slots:
    virtual void slotOK();
  void slotPlayerJoinedGame(KPlayer* player);
  void slotNationChanged();
  
};

}

#endif // KPLAYERSETUPDIALOG_H
