/* This file is part of KsirK.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

/*    begin                : Mon Feb 07 2005 */
 
#include "KsirkGlobalDefinitions.h"
#include "ksirkgamexmlloader.h"
#include "ksirkgamexmlhandler.h"

#include <qxml.h>
#include <KMessageBox>
#include <KLocale>

namespace Ksirk
{
namespace SaveLoad
{

GameXmlLoader::GameXmlLoader(const QString& fileName, KGameWindow& game, 
    std::vector<GameLogic::PlayerMatrix>& waitedPlayers)
{
  GameXmlHandler handler(game, waitedPlayers);
  
  QFile xmlFile( fileName );
  QXmlInputSource source( &xmlFile );
  
  QXmlSimpleReader reader;
  reader.setContentHandler( &handler );
  
  if (!reader.parse( source ))
  {
    KMessageBox::error(0, i18n("Skin file parsing error"), 
      i18n("KsirK - Error"));
    exit(0);
  }
}

} // closing namespace SaveLoad
} // closing namespace Ksirk

