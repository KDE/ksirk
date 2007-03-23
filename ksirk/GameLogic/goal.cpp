/* This file is part of KsirK.
 *   Copyright (C) 2005-2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>
 *
 *   KsirK is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 */
#include "goal.h"
#include "player.h"
#include "onu.h"
#include "kgamewin.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

namespace Ksirk {

namespace GameLogic {

Goal::Goal() :
  m_type(NoGoal),
  m_description(""),
  m_nbCountries(0),
  m_nbArmiesByCountry(0),
  m_continents(),
  m_players(),
  m_player(0)
{
}

Goal::Goal(const Goal& goal)
{
  m_type = goal.m_type;
  m_description = goal.m_description;
  m_nbCountries = goal.m_nbCountries;
  m_nbArmiesByCountry = goal.m_nbArmiesByCountry;
  m_continents = goal.m_continents;
  m_players = goal.m_players;
  m_player = goal.m_player;
}

Goal::~Goal()
{
}

bool Goal::checkFor(const GameLogic::Player* player) const 
{
  switch (type())
  {
  case Goal::GoalPlayer :
    kDebug() << "Goal reached checked elsewhere for Player type" << endl;
    return false;
    break;
  case Goal::Countries:
    return checkCountriesFor(player);
    break;
  case Goal::Continents:
    return checkContinentsFor(player);
    break;
  default:
    return (player->getNbCountries() >= GameAutomaton::changeable().game()->theWorld()->getCountries().size());
  }
}

bool Goal::checkCountriesFor(const GameLogic::Player* player) const
{
  kDebug() << "Goal::checkCountriesFor " << player->name() << endl;
  if (player->getNbCountries() >= m_nbCountries)
  {
    uint nbCountriesOk = 0;
    for (unsigned int i = 0; i < player->countries().size(); i++)
    {
      if ((player->countries().at(i))->nbArmies() >= m_nbArmiesByCountry)
      {
        nbCountriesOk++;
      }
    }
    return (nbCountriesOk >= m_nbCountries);
  }
  else 
  {
    return false;
  }
}

bool Goal::checkContinentsFor(const GameLogic::Player* player) const
{
  kDebug() << "Goal::checkContinentsFor " << player->name() << endl;
  std::set<unsigned int>::const_iterator it, it_end;
  it = m_continents.begin(); it_end = m_continents.end();
  for (; it != it_end; it++)
  {
    kDebug() << "Should be owned continent id: " << *it<< endl;
    if ( GameAutomaton::changeable().game()->theWorld()->continentWithId(*it) == 0
       || GameAutomaton::changeable().game()->theWorld()->continentWithId(*it)->owner() != player)
    {
      return false;
    }
  }
  bool otherFound = ( m_continents.find(0) == m_continents.end() );
  if (!otherFound)
  {
    std::vector<Continent*> continents = GameAutomaton::changeable().game()->theWorld()->getContinents();
    for ( unsigned int j = 0; j < continents.size(); j++)
    {
      if ( ( m_continents.find((continents.at(j))->id()) == m_continents.end() )
           && ( continents.at(j)->owner() == player) )
      {
        otherFound = true;
        break;
      }
    }
  }
  return otherFound;
}

QString Goal::message(int displayType)
{
  KLocalizedString res;

  std::set<unsigned int>::const_iterator it, it_end, it_next;
  if (displayType & GoalDesc)
  {
    res = ki18n(m_description.toUtf8().data());
    if (m_player == 0)
    {
      res = res.subs("");
    }
    else
    {
      res = res.subs(m_player->name());
    }
    kDebug() << "Goal type='" << m_type << "' mes = '" << res.toString() << "'" << endl;
    
    std::set<unsigned int>::const_iterator it, it_end;
    switch (m_type)
    {
    case Goal::GoalPlayer :
      if (!m_players.empty())
      {
        kDebug() << "  player num='" << (*m_players.begin()) << endl;
        kDebug() << "  this is player='" << GameAutomaton::changeable().findPlayer(*m_players.begin()) << endl;
        if (GameAutomaton::changeable().findPlayer(*m_players.begin())==0)
        {
          res = res.subs("?");
        }
        else
        {
          kDebug() << "  its name is='" << GameAutomaton::changeable().findPlayer(*m_players.begin())->name() << endl;
          res = res.subs(GameAutomaton::changeable().findPlayer(*m_players.begin())->name());
        }
        res = res.subs(m_nbCountries);
      }
      else
      {
        res = res.subs(i18n("Error : no player to destroy"));
      }
      break;
    case Goal::Countries:
      kDebug() << "  arg1 = '" << m_nbCountries << "'" << endl;
      res=res.subs(m_nbCountries);
      if (m_nbArmiesByCountry > 0)
      {
        kDebug() << "  arg2 = '" << m_nbArmiesByCountry << "'" << endl;
        res=res.subs(m_nbArmiesByCountry);
      }
      break;
    case Goal::Continents:
      it = m_continents.begin(); it_end = m_continents.end();
      for (; it != it_end; it++)
      {
        if (*it != 0)
        {
          kDebug() << "  arg = '" << GameAutomaton::changeable().game()->theWorld()->continentWithId(*it)->name() << "'" << endl;
          res=res.subs(i18n(GameAutomaton::changeable().game()->theWorld()->continentWithId(*it)->name().toUtf8().data()));
          
        }
      }
      break;
    default:;
    }
  }
  QString mes = res.toString();
  if (displayType & GoalAdvance)
  {
    int diff = 0;
    switch (m_type)
    {
    case Goal::GoalPlayer :
      if (GameAutomaton::changeable().findPlayer(*m_players.begin()) != 0)
      {
        mes += i18n("\n%1 is still alive...",GameAutomaton::changeable().findPlayer(*m_players.begin())->name());
      }
      else
      {
        mes += i18n("\nYou now have to conquer %1",m_nbCountries);
      }
    break;
    case Goal::Countries:
      diff  = m_nbCountries-m_player->countries().size();
      if (diff > 0)
      {
        mes += i18n("\n%1, you have still %2 countries to conquer...",m_player->name(),diff);
      }
      else
      {
        int nbOk = 0;
        for (unsigned int i = 0; i < m_player->countries().size();i++)
        {
          if (m_player->countries().at(i)->nbArmies() >= m_nbArmiesByCountry)
          {
            nbOk++;
          }
        }
        mes += i18n("\n%1, you have enough countries but you still have to put more than %2 armies on %3 of them...",m_player->name(),m_nbArmiesByCountry,m_nbCountries-nbOk);
        
      }
      break;
    case Goal::Continents:
      mes += i18n("\n%1, you still have to conquer ",m_player->name());
      it = m_continents.begin(); it_end = m_continents.end();
      if (*it != 0)
      {
        Continent* continent = const_cast<Continent*>(GameAutomaton::changeable().game()->theWorld()->continentWithId(*it));
        int nb = continent->getMembers().size() - continent->countriesOwnedBy(m_player).size();
        mes += i18n("%1 countries in %2",nb,continent->name());
      }
      it++;
      while (it != it_end)
      {
        it_next = it;
        it_next++;
        QString joint;
        if (it_next==it_end)
        {
          joint = " and ";
        }
        else
        {
          joint = ", ";
        }
        if (*it != 0)
        {
          Continent* continent = const_cast<Continent*>(GameAutomaton::changeable().game()->theWorld()->continentWithId(*it));
          int nb = continent->getMembers().size() - continent->countriesOwnedBy(m_player).size();
          mes += joint + i18n("%1 in %2",nb,continent->name());
        }
        it++;
      }
      mes += '.';
      break;
    default:;
    }
  }
  return mes;
}
  
void Goal::show(int displayType)
{
  KMessageBox::information(
                            GameAutomaton::changeable().game(), 
                            message(displayType), 
                            i18n("KsirK - Goal Display"));
}

QDataStream& operator<<(QDataStream& stream, const Goal& goal)
{
  stream << quint32(goal.type());
  if (goal.player() != 0)
  {
    stream << quint32(goal.player()->id());
  }
  else
  {
    stream << quint32(0);
  }
  stream << goal.description();
  std::set< unsigned int >::iterator it, it_end;
  switch (goal.type())
  {
  case Goal::GoalPlayer :
    stream << quint32(goal.players().size());
    it = goal.players().begin(); it_end = goal.players().end();
    for (; it != it_end; it++)
    {
      stream << quint32(*it);
    }
    stream << quint32(goal.nbCountries());
    break;
  case Goal::Countries:
    stream << quint32(goal.nbCountries());
    stream << quint32(goal.nbArmiesByCountry());
    break;
  case Goal::Continents:
    stream << quint32(goal.continents().size());
    it = goal.continents().begin(); it_end = goal.continents().end();
    for (; it != it_end; it++)
    {
      stream << quint32(*it);
    }
    break;
  default:;
  }
  return stream;
}

QDataStream& operator>>(QDataStream& stream, Goal& goal)
{
  quint32 type;
  QString description;
  quint32 nb, nbp;
  quint32 id, ownerId;
  stream >> type;
  stream >> ownerId;
  goal.player(static_cast<Player*>(GameAutomaton::changeable().findPlayer(ownerId)));
  goal.type(Goal::GoalType(type));
  stream >> description;
  goal.description(description);
  switch (type)
  {
  case Goal::GoalPlayer :
    goal.players().clear();
    stream >> nbp;
    for (quint32 i = 0; i < nbp; i++)
    {
      stream >> id;
      goal.players().insert(id);
    }
    stream >> nb;
    goal.nbCountries(nb);
    break;
  case Goal::Countries:
    stream >> nb;
    goal.nbCountries(nb);
    stream >> nb;
    goal.nbArmiesByCountry(nb);
    break;
  case Goal::Continents:
    stream >> nb;
    goal.continents().clear();
    for (quint32 i = 0; i < nb; i++)
    {
      stream >> id;
      goal.continents().insert(id);
    }
    break;
  default:;
  }
  return stream;
}

void Goal::saveXml(std::ostream& xmlStream) const
{
  
  xmlStream << "<goal player=\"";
  if (m_player != 0)
  {
    xmlStream << m_player->name().toUtf8().data();
  }
  xmlStream << "\" type=\"" << m_type << "\" description=\"" << m_description.toUtf8().data();
  xmlStream << "\" nbCountries=\"" << m_nbCountries << "\" nbArmiesByCountry=\"" << m_nbArmiesByCountry << "\">\n";
  xmlStream << "<continents>\n";
  std::set< unsigned int >::iterator itc, itc_end;
  itc = continents().begin(); itc_end = continents().end();
  for (; itc != itc_end; itc++)
  {
    QString name = (*itc==0)?"":GameAutomaton::changeable().game()->theWorld()->continentWithId(*itc)->name();
    xmlStream << "<continent name=\"" << name.toUtf8().data() << "\"/>\n";
  }
  xmlStream << "</continents>\n";
  xmlStream << "<players>\n";
  std::set< unsigned int >::iterator itp, itp_end;
  itp = players().begin(); itp_end = players().end();
  for (; itp != itp_end; itp++)
  {
    xmlStream << "<player name=\"" << GameAutomaton::changeable().findPlayer(*itp)->name().toUtf8().data() << "\"/>\n";
  }
  xmlStream << "</players>\n";
  xmlStream << "</goal>\n";
  
    
}

}

}
