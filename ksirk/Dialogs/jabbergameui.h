/* This file is part of KsirK.
   Copyright (C) 2008 Gael de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

#ifndef JABBERGAMEWIDGET_H
#define JABBERGAMEWIDGET_H

#include "ui_jabbergameui.h"
#include "Jabber/jabberclient.h"

#include <QWidget>

#include <qca.h>

namespace Ksirk
{
  namespace GameLogic
  {
    class GameAutomaton;
  }
}
class KsirkJabberGameWidget : public QWidget, public Ui::KsirkJabberGameWidget
{
  Q_OBJECT
public:
  KsirkJabberGameWidget(QWidget* parent);

  ~KsirkJabberGameWidget() {}
                         
  inline void setPreviousGuiIndex(int previousIndex) {m_previousGuiIndex = previousIndex;}

  void init(Ksirk::GameLogic::GameAutomaton* automaton);

Q_SIGNALS:
  void cancelled(int);
  
public Q_SLOTS:
  void slotNewJabberGame(const QString& nick,
                          int nbPlayers,
                          const QString& skin);

private Q_SLOTS:
  void slotJabberConnectButtonClicked();
  void slotJabberDisconnected();
  void slotJabberError(int);
  void slotHandleTLSWarning(QCA::TLS::IdentityResult, QCA::Validity);
  void slotJabberConnected();
  void slotJabberClientError(JabberClient::ErrorCode);

  void slotRosterRequestFinished ( bool );
  
  void slotJoinRoom();
  void slotGroupChatJoined (const XMPP::Jid & jid);
  void slotGroupChatLeft (const XMPP::Jid & jid);
  void slotGroupChatPresence (const XMPP::Jid & jid, const XMPP::Status & status);
  void slotGroupChatError (const XMPP::Jid & jid, int error, const QString & reason);

  void slotJoinJabberGame();
  
  void slotCellClicked(int row, int column);

  void slotCancel();
  
private:
  Ksirk::GameLogic::GameAutomaton* m_automaton;
  QString m_nick;
  int m_nbPlayers;
  int m_previousGuiIndex;
};

#endif
