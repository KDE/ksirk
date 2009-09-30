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

#include "newgamesetup.h"
#include "GameLogic/newplayerdata.h"

using namespace Ksirk;

int NewGameSetup::nbLocalPlayers() const
{
  int n = 0;
  foreach (Ksirk::NewPlayerData* player, m_players)
  {
    if (!player->network())
    {
      n++;
    }
  }
  return n;
}

bool NewGameSetup::addPlayer(NewPlayerData* player)
{
  kDebug() << player->name();
  bool found = false;
  foreach (Ksirk::NewPlayerData* p, m_players)
  {
    if (p->name() == player->name())
    {
      found = true;
      break;
    }
  }
  if (!found)
  {
    m_players.push_back(player);
  }
  return !found;
}

void NewGameSetup::clear()
{
  kDebug();
  m_players.clear();
}

QDataStream& operator<<(QDataStream& stream, const NewGameSetup& ngs)
{
  kDebug();
  stream << ngs.skin();

  stream << (quint32)ngs.players().size();
  foreach (Ksirk::NewPlayerData* newPlayer, ngs.players())
  {
    stream << newPlayer->name();
    stream << newPlayer->nation();
    stream << newPlayer->password();
    stream << (quint32)newPlayer->computer();
    stream << (quint32)newPlayer->network();
    
  }
  stream << (quint32)ngs.nbPlayers();
  stream << (quint32)ngs.nbNetworkPlayers();
  stream << (quint32)ngs.useGoals();
  stream << (quint32)ngs.networkGameType();
  stream << (quint32)ngs.tcpPort();
  stream << ngs.host();
  
  return stream;
}

QDataStream& operator>>(QDataStream& stream, NewGameSetup& ngs)
{
  kDebug();
  QString skin;
  stream >> skin;
  ngs.setSkin(skin);
  quint32 players;
  stream >> players;
  kDebug() << "nb players" << players;
  for (int i = 0; i < players; i++)
  {
    QString name;
    stream >> name;
    QString nation;
    stream >> nation;
    QString password;
    stream >> password;
    quint32 computer;
    stream >> computer;
    quint32 network;
    stream >> network;
    kDebug() << "player" << name << nation << password << computer << network;
    Ksirk::NewPlayerData* newPlayer = new NewPlayerData(name,nation,password,computer,network);
    ngs.players().push_back(newPlayer);
  }
  quint32 nbPlayers;
  stream >> nbPlayers;
  ngs.setNbPlayers(nbPlayers);
  quint32 nbNetworkPlayers;
  stream >> nbNetworkPlayers;
  ngs.setNbNetworkPlayers(nbNetworkPlayers);
  quint32 useGoals;
  stream >> useGoals;
  ngs.setUseGoals(useGoals);
  quint32 networkGameType;
  stream >> networkGameType;
  ngs.setNetworkGameType((GameLogic::GameAutomaton::NetworkGameType)networkGameType);
  quint32 tcpPort;
  stream >> tcpPort;
  ngs.setTcpPort(tcpPort);
  QString host;
  stream >> host;
  ngs.setHost(host);
  return stream;
}
