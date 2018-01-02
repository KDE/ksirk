/* This file is part of KsirK.
   Copyright (C) 2005-2007 Gaël de Chalendar <kleag@free.fr>

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

/* begin                : Mon Feb 07 2005 */

#ifndef KSIRK_SAVELOAD_KSIRKGAMEXMLLOADER_H
#define KSIRK_SAVELOAD_KSIRKGAMEXMLLOADER_H

#include "KsirkGlobalDefinitions.h"
#include "kgamewin.h"
#include "GameLogic/player.h"

#include <qxml.h>
#include <QList>

namespace Ksirk
{
namespace SaveLoad
{

/** 
  * Sets up the KsirK skin data file SAX parser and runs it
  * @author Gael de Chalendar (aka Kleag) 
  */
class GameXmlLoader
{
public:
  /**
    * Constructor
    * @param fileName The name of the file to read.
    * @param game The game to initialize with the file's data
    * @param waitedPlayers The list of players definitions whose connection 
    * from the network will be waited for.
    */
  GameXmlLoader(const QString& fileName, KGameWindow& game, 
        QList<GameLogic::PlayerMatrix>& waitedPlayers);
  
private:
  QString m_onuFile;
};


} // closing namespace SaveLoad
} // closing namespace Ksirk


#endif // KSIRK_SAVELOAD_KSIRKGAMEXMLLOADER_H

