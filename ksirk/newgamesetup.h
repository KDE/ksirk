/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef NEWGAMESETUP_H
#define NEWGAMESETUP_H

#include "GameLogic/gameautomaton.h"


#include <qlist.h>
#include <qmap.h>

namespace Ksirk
{
  class NewPlayerData;
  namespace GameLogic
  {
    class ONU;
  }
}
  
class NewGameSetup
{
public:
  explicit NewGameSetup(Ksirk::GameLogic::GameAutomaton* automaton) :
  m_automaton(automaton), m_skin(""), m_worlds(), m_players(),
  m_nbPlayers(0),m_nbNetworkPlayers(0),
  m_useGoals(true), m_networkGameType(Ksirk::GameLogic::GameAutomaton::None),
  m_tcpPort(20000) {}

  virtual ~NewGameSetup() {}
  
  inline Ksirk::GameLogic::GameAutomaton* automaton() {return m_automaton;}
  inline const QString& skin() const {return m_skin;}
  inline void setSkin(const QString& skin) {m_skin= skin;}

  inline QMap<QString, Ksirk::GameLogic::ONU*>& worlds() {return m_worlds;}
  
  inline QList<Ksirk::NewPlayerData*>& players() {return m_players;}
  inline const QList<Ksirk::NewPlayerData*>& players() const {return m_players;}

  inline int nbPlayers() const {return m_nbPlayers;}
  inline void setNbPlayers(int nbPlayers) {m_nbPlayers = nbPlayers;}

  int nbLocalPlayers() const;

  inline int nbNetworkPlayers() const {return m_nbNetworkPlayers;}
  inline void setNbNetworkPlayers(int nbNetworkPlayers) {m_nbNetworkPlayers = nbNetworkPlayers;}

  inline bool useGoals() const {return m_useGoals;}
  inline void setUseGoals(bool useGoals) {m_useGoals = useGoals;}

  inline Ksirk::GameLogic::GameAutomaton::NetworkGameType networkGameType() const
  {
    return m_networkGameType;
  }
  inline void setNetworkGameType(Ksirk::GameLogic::GameAutomaton::NetworkGameType networkGameType)
  {
    m_networkGameType = networkGameType;
  }

  inline int tcpPort() const {return m_tcpPort;}
  inline void setTcpPort(int tcpPort) {m_tcpPort = tcpPort;}
  inline const QString& host() const {return m_host;}
  inline void setHost(const QString& host) {m_host = host;}

  bool addPlayer(Ksirk::NewPlayerData* player);

  void clear();
  
private:
  Ksirk::GameLogic::GameAutomaton* m_automaton;
  QString m_skin;
  QMap<QString, Ksirk::GameLogic::ONU*> m_worlds;
  
  QList<Ksirk::NewPlayerData*> m_players;
  int m_nbPlayers;
  int m_nbNetworkPlayers;
  bool m_useGoals;
  Ksirk::GameLogic::GameAutomaton::NetworkGameType m_networkGameType;
  int m_tcpPort;
  QString m_host;
};

QDataStream& operator<<(QDataStream& stream, const NewGameSetup& ngs);
QDataStream& operator>>(QDataStream& stream, NewGameSetup& ngs);

#endif // NEWGAMESETUP_H
