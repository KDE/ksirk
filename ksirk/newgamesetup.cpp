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
