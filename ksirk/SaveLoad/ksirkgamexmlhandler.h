/* This file is part of KsirK.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

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

#ifndef KSIRK_SAVELOAD_KSIRKGAMEXMLHANDLER_H
#define KSIRK_SAVELOAD_KSIRKGAMEXMLHANDLER_H

#include "KsirkGlobalDefinitions.h"
#include "kgamewin.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/player.h"
#include "GameLogic/goal.h"

#include <QString>
#include <qxml.h>

namespace Ksirk
{

namespace SaveLoad
{

/**
  * Sax handler for the KsirK skin XML definition file.
  * @author Gael de Chalendar (aka Kleag)
  */
class GameXmlHandler : public QXmlDefaultHandler
{
public:
  /** 
    * Constructor
    * @param game The game that the file loaded will set up
    * @param waitedPlayers This list will be filled by the description of the 
    * network players whose connection will be waited for
    */
  GameXmlHandler(KGameWindow& game, QList<GameLogic::PlayerMatrix>& waitedPlayers) :
    m_game(game),
    m_waitedPlayers(waitedPlayers),
    m_playersNumber(0),
    m_inGoal(false),
    m_goal(0)
  {
    m_waitedPlayers.clear();
  }

  /** Default destructor */
  ~GameXmlHandler() override {}

  /** Called when initializing reading of a document */
  bool startDocument() override;

  /** 
    * Called whenever a XML open tag is read
    * @param namespaceURI The read element namespace URI
    * @param localName The simple name of the tag
    * @param qName The qualified name of the tag (see http://www.w3.org/XML/ 
    * for details)
    * @param atts The attributes of this tag.
    */
  bool startElement( const QString & namespaceURI, const QString & localName, 
                     const QString & qName, const QXmlAttributes & atts ) override;

  /**
    * Called whenever a XML close tag is read.
    * @param namespaceURI The closing element namespace URI
    * @param localName The simple name of the tag
    * @param qName The qualified name of the tag (see http://www.w3.org/XML/ 
    * for details)
    */
  bool endElement(const QString& namespaceURI, const QString& localName, 
                  const QString& qName ) override;

private:
  KGameWindow& m_game;
  GameLogic::GameAutomaton::GameState m_savedState;
  QMap<QString,QString> m_ownersMap;
  QList<GameLogic::PlayerMatrix>& m_waitedPlayers;
  unsigned int m_playersNumber;
  bool m_inGoal;
  GameLogic::Goal* m_goal;
  QString m_goalPlayerName;
};


} // closing namespace SaveLoad
} // closing namespace Ksirk


#endif // KSIRK_SAVELOAD_KSIRKGAMEXMLHANDLER_H

