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
#include "GameLogic/onu.h"

#include <KLocalizedString>
#include "ksirk_debug.h"
#include <QStandardPaths>
#include <KMessageBox>

#include <qdir.h>

using namespace Ksirk;

NewGameSetup::NewGameSetup(Ksirk::GameLogic::GameAutomaton* automaton) :
m_automaton(automaton), m_skin(""), m_worlds(), m_players(),
                               m_nbPlayers(0),m_nbNetworkPlayers(0),
                               m_useGoals(true), m_networkGameType(Ksirk::GameLogic::GameAutomaton::None),
                               m_tcpPort(20000)
{
  QStringList skinsDirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, "skins", QStandardPaths::LocateDirectory);
  qCDebug(KSIRK_LOG) << skinsDirs;
  foreach (const QString &skinsDirName, skinsDirs)
  {
    if (skinsDirName.isEmpty())
    {
      KMessageBox::error(0,
                         i18n("Skins directory not found - Verify your installation\nProgram cannot continue"),
                         i18n("Fatal Error!"));
                         exit(2);
    }
    qCDebug(KSIRK_LOG) << "Got skins dir name: " << skinsDirName;
    QDir skinsDir(skinsDirName);
    QStringList skinsDirsNames = skinsDir.entryList(QStringList("[a-zA-Z]*"), QDir::Dirs);
    
    foreach (const QString& name, skinsDirsNames)
    {
      qCDebug(KSIRK_LOG) << "Got skin dir name: " << name;
      QDir skinDir(skinsDirName + '/' + name);
      if (skinDir.exists())
      {
        qCDebug(KSIRK_LOG) << "Got skin dir: " << skinDir.dirName();
        GameLogic::ONU* world = new GameLogic::ONU(automaton,skinsDirName + '/' + skinDir.dirName() + "/Data/world.desktop");
        if (!world->skin().isEmpty())
        {
          m_worlds[i18n(world->name().toUtf8().data())] = world;
        }
        else
        {
          delete world;
        }
      }
    }
  }
}
                               
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
  qCDebug(KSIRK_LOG) << player->name();
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
  qCDebug(KSIRK_LOG);
  m_players.clear();
}

QDataStream& operator<<(QDataStream& stream, const NewGameSetup& ngs)
{
  qCDebug(KSIRK_LOG);
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
  qCDebug(KSIRK_LOG);
  QString skin;
  stream >> skin;
  ngs.setSkin(skin);
  quint32 players;
  stream >> players;
  qCDebug(KSIRK_LOG) << "nb players" << players;
  for (quint32 i = 0; i < players; i++)
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
    qCDebug(KSIRK_LOG) << "player" << name << nation << password << computer << !network;
    Ksirk::NewPlayerData* newPlayer = new NewPlayerData(name,nation,password,computer,!network);
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
