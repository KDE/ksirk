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


#ifndef KPLAYERSETUPWIDGET_H
#define KPLAYERSETUPWIDGET_H

#include "ui_qplayersetupdialog.h"

#include <QWidget>

#include <QMap>

class KPlayer;
class NewGameSetup;

namespace Ksirk
{
  class NewPlayerData;
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
class KPlayerSetupWidget : public QWidget, public Ui::QPlayerSetupWidget
{
  Q_OBJECT
public:
  explicit KPlayerSetupWidget(QWidget *parent=nullptr);
  
  ~KPlayerSetupWidget() override;

  void init(GameLogic::GameAutomaton* automaton,
                      GameLogic::ONU* onu,
                      unsigned int playerNumber,
                      QString &playerName,
                      QString& password,
                      bool computerPlayer,
                      QMap< QString, QString >& nations,
                      QString & nationName,
                      NewGameSetup* newGameSetup);

  void init(NewPlayerData* player = nullptr);
  
Q_SIGNALS:
  void next();
  void previous();
  void cancel();
  
public Q_SLOTS:
  void slotNext();
  void slotPrevious();
  void slotCancel();
  void slotPlayerJoinedGame(KPlayer* player);
  void slotNationChanged();
  void slotNameEdited (const QString& text);
  
private:
  /** Fills the nations combo with nations names and flag images */
  void fillNationsCombo();
  
  bool isAvailable(QString nationName);
  void setLabelText();

  GameLogic::GameAutomaton* m_automaton;
  QString m_name;
  bool m_computer;
  QString m_nationName;
  /** list of nation name , flag file name pairs */
  QMap< QString, QString > m_nations;
  GameLogic::ONU* m_onu;
  
  /** list of internationalized nations names. Used to retrive the internal name after choice in UI */
  QMap< QString, QString > m_nationsNames;
  unsigned int m_number;
  
  QString m_password;
  NewGameSetup* m_newGameSetup;
};

}

#endif // KPLAYERSETUPWIDGET_H
