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

#include "ksirkgamexmlhandler.h"
#include "KsirkGlobalDefinitions.h"
#include "GameLogic/country.h"
#include "GameLogic/onu.h"
#include "GameLogic/goal.h"
#include "GameLogic/KMessageParts.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <sstream>
#include <map>

namespace Ksirk
{
using namespace GameLogic;

namespace SaveLoad
{

bool GameXmlHandler::startDocument()
{ 
  kDebug() << "startDocument" << endl;
  return true;
}

bool GameXmlHandler::startElement( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts ) 
{
  Q_UNUSED(namespaceURI);
  Q_UNUSED(qName);
  kDebug() << "startElement " << localName << " / " << qName << endl;
  if (localName == "ksirkSavedGame")
  {
    QString fv =atts.value("formatVersion");
    QString wfv =SAVE_GAME_FILE_FORMAT_VERSION;
    if (fv!=wfv)
    {
      KMessageBox::sorry(0, 
          i18n("Wrong save game format. Waited %1 and got %2 !",QString(SAVE_GAME_FILE_FORMAT_VERSION),atts.value("formatVersion")),
          i18n("KsirK - Cannot load !"));

      return false;
    }
  }
  else if (localName == "game")
  {
    kDebug() << "GameXmlHandler stored game state is: " << atts.value("state") << endl;
    
    std::istringstream issSkin(atts.value("skin").toUtf8().data());
    std::string skin;
    issSkin >> skin;
    m_game.automaton()->skin(skin.c_str());
    
    std::istringstream iss(atts.value("state").toUtf8().data());
    int istate;
    iss >> istate;
    m_savedState = GameLogic::GameAutomaton::GameState(istate);
    m_game.automaton()->savedState(m_savedState);
  }
  else if (localName == "players" && !m_inGoal)
  {
    std::istringstream iss(atts.value("nb").toUtf8().data());
    int nb;
    iss >> nb;
    kDebug() << "Setting min-max players to " << nb << endl;
    m_game.automaton()->setMinPlayers(nb);
    m_game.automaton()->setMaxPlayers(nb);
  }
  else if (localName == "player" && !m_inGoal)
  {
    kDebug() << "Reading a player" << endl;
    m_playersNumber++;
    unsigned int nbAvailArmies;
    std::istringstream issNbAvailArmies(atts.value("nbAvailArmies").toUtf8().data());
    issNbAvailArmies >> nbAvailArmies;
    
    unsigned int nbCountries;
    std::istringstream issNbCountries(atts.value("nbCountries").toUtf8().data());
    issNbCountries >> nbCountries;
    
    QString name = atts.value("name");
    
    QString nationName = atts.value("nation");
    
    unsigned int nbAttack;
    std::istringstream issNbAttack(atts.value("nbAttack").toUtf8().data());
    issNbAvailArmies >> nbAttack;
    
    unsigned int nbDefense;
    std::istringstream issNbDefense(atts.value("nbDefense").toUtf8().data());
    issNbAvailArmies >> nbDefense;
    
    
    bool isAi = false;
    if (atts.value("ai") == "true") isAi = true;
    
    QString password = atts.value("password");
    
    bool isLocal = true; // local player by default
    if (atts.value("local") == "false") isLocal = false;

    if (isLocal)
    {
      kDebug() << "Adding the read player " << name << endl;
      m_game.addPlayer(name, nbAvailArmies, nbCountries, nationName,
                        isAi, password, nbAttack, nbDefense);
    }
    else
    {
      kDebug() << "Player" << name << "stored in matrix";
      PlayerMatrix pm(m_game.automaton());
      pm.name = name;
      pm.nbAttack = nbAttack;
      pm.nbCountries = nbCountries;
      pm.nbAvailArmies = nbAvailArmies;
      pm.nbDefense = nbDefense;
      pm.nation = nationName;
      pm.password = password;
      pm.isAI = isAi;
      foreach (const QString& k, m_ownersMap.keys())
      {
        if ( m_ownersMap[k] == name )
        {
          pm.countries.push_back(k);
        }
      }
      m_waitedPlayers.push_back(pm);
    }
  }
  else if (localName == "currentPlayer")
  {
    Player* currentPlayer = m_game.automaton()->playerNamed(atts.value("name"));
    if (currentPlayer)
    {
//       kDebug() << "Setting current player to " << atts.value("name") << " / " << currentPlayer << endl;
      m_game.automaton()->currentPlayer(currentPlayer);
      KMessageParts messageParts;
      messageParts << I18N_NOOP("Current player is: %1") << currentPlayer->name();
      m_game.broadcastChangeItem(messageParts, ID_STATUS_MSG2);
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << currentPlayer->name();
      m_game.automaton()->sendMessage(buffer,SetBarFlagButton);
    }
    m_game.automaton()->savedPlayer(atts.value("name"));
  }
  else if (localName == "ONU")
  {
    kDebug() << "GameXmlHandler starts new game with ONU file: " << atts.value("file") << endl;
    if (!(m_game.automaton()->playerList()->isEmpty()))
    {
      m_game.automaton()->playerList()->clear();
      m_game.automaton()->currentPlayer(0);
      kDebug() << "  playerList size = " << m_game.automaton()->playerList()->count() << endl;
    }
    m_game.automaton()->game()->newSkin(atts.value("file"));
  }
  else if (localName == "country")
  {
//   kDebug() << "GameXmlHandler loads country: " << atts.value("name") << endl;
    Country* country = m_game.theWorld()->countryNamed(atts.value("name"));
    unsigned int gotNbArmies;
    std::istringstream issNbArmies(atts.value("nbArmies").toUtf8().data());
    issNbArmies >> gotNbArmies;
    country->nbArmies(gotNbArmies);
    
    unsigned int gotNbAddedArmies;
    std::istringstream issNbAddedArmies(atts.value("nbArmiesAdded").toUtf8().data());
    issNbArmies >> gotNbAddedArmies;
    country->nbAddedArmies(gotNbAddedArmies);

    kDebug() << "Storing" << atts.value("owner") << "as owner of" << atts.value("name");
    m_ownersMap.insert(atts.value("name"), atts.value("owner"));
  }
  else if (localName == "goal")
  {
    kDebug() << "loads goal for: " << atts.value("player") << endl;
    m_goal = new GameLogic::Goal(m_game.automaton());
    m_goalPlayerName = atts.value("player");
    Player* player = m_game.automaton()->playerNamed(atts.value("player").toUtf8().data());
//     kDebug() << "Got player pointer " << player << endl;
    m_goal->player(player);
    unsigned int type;
    std::istringstream issType(atts.value("type").toUtf8().data());
    issType >> type;
    m_goal->type(GameLogic::Goal::GoalType(type));
    m_goal->description(atts.value("description"));
    unsigned int nbCountries;
    std::istringstream issNbCountries(atts.value("nbCountries").toUtf8().data());
    issNbCountries >> nbCountries;
    m_goal->nbCountries(nbCountries);
    unsigned int nbArmiesByCountry;
    std::istringstream issNbArmiesByCountry(atts.value("nbArmiesByCountry").toUtf8().data());
    issNbArmiesByCountry>> nbArmiesByCountry;
    m_goal->nbArmiesByCountry(nbArmiesByCountry);
    
    m_inGoal = true;
  }
  else if (localName == "player" && m_inGoal)
  {
    m_goal->players().push_back(atts.value("name"));
  }
  else if (localName == "continent" && m_inGoal)
  {
//     kDebug() << "Getting id of continent named " << atts.value("name") << endl;
    QString id;
    if (!atts.value("name").isEmpty())
        id = atts.value("name");
    m_goal->continents().push_back(id);
  }
  return true;
}

bool GameXmlHandler::endElement(const QString& namespaceURI, const QString& localName, const QString& qName)
{
  Q_UNUSED(namespaceURI);
  Q_UNUSED(qName);
//   kDebug() << "endElement " << localName << " / " << qName << endl;
  if (localName == "game")
  {
    foreach (const QString& k, m_ownersMap.keys())
    {
//       kDebug() << "Setting owner of " << k << " to " << m_ownersMap[k] << endl;
      Country* country = m_game.theWorld()->countryNamed(k);
      Player* owner = m_game.automaton()->playerNamed(m_ownersMap[k]);
      if (owner)
      {
//         kDebug() << "Setting owner of " << country->name() << " to " << owner->name() << endl;
        country-> owner(owner);
      }
      else
      {
//         kDebug() << "Player" << m_ownersMap[k] << "not found";
        QList<GameLogic::PlayerMatrix>::iterator itw,itw_end;
        itw = m_waitedPlayers.begin(); itw_end = m_waitedPlayers.end();
        for (; itw != itw_end; itw++)
        {
          if ( (*itw).name == m_ownersMap[k] )
          {
            (*itw).countries.push_back(k);
            break;
          }
        }
      }
    }
    if (!m_waitedPlayers.empty())
    {
//       kDebug() << "There is waited players: does not change state nor run game..." << endl;
      m_waitedPlayers[0].state = m_savedState;
    }
    else
    {
//       kDebug() << "GameXmlHandler set game state to: " << m_savedState << endl;
      m_game.automaton()->state(m_savedState);
    }
  }
  else if (localName == "goal")
  {
    m_inGoal = false;
    if (m_goal)
    {
      if (m_goal->player())
      {
        m_goal->player()->goal(*m_goal);
      }
      else
      {
        QList<GameLogic::PlayerMatrix>::iterator itw,itw_end;
        itw = m_waitedPlayers.begin(); itw_end = m_waitedPlayers.end();
        for (; itw != itw_end; itw++)
        {
          if ( (*itw).name == m_goalPlayerName )
          {
            (*itw).goal = *m_goal;
            break;
          }
        }
      }
      delete m_goal;
    }
    m_goal = 0;
  }
  return true;
}


} // closing namespace SaveLoad
} // closing namespace Ksirk


