/* This file is part of KsirK.
 *   Copyright (C) 2005-2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>
 *
 *   KsirK is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.
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
#include "gameautomaton.h"

#include "ksirk_debug.h"
#include <KLocalizedString>
#include <KMessageBox>

namespace Ksirk {

namespace GameLogic {

Goal::Goal() :
  m_automaton(nullptr),
  m_type(NoGoal),
  m_description(),
  m_nbCountries(0),
  m_nbArmiesByCountry(0),
  m_continents(),
  m_players(),
  m_player(nullptr)
{
}

Goal::Goal(GameAutomaton* automaton) :
m_automaton(automaton),
m_type(NoGoal),
m_description(),
m_nbCountries(0),
m_nbArmiesByCountry(0),
m_continents(),
m_players(),
m_player(nullptr)
{
}

Goal::~Goal()
{
}

bool Goal::checkFor(const GameLogic::Player* player) const 
{
  qCDebug(KSIRK_LOG) << message(GoalAdvance);
  int diff = 0;
  switch (type())
  {
  case Goal::GoalPlayer :
    if (m_automaton->playerNamed(*m_players.begin()) != nullptr)
    {
      return false;
    }
    else
    {
      diff  = m_nbCountries-m_player->countries().size();
      if (diff > 0)
      {
        return false;
      }
      else
      {
        return true;
      }
    }
    return false;
    break;
  case Goal::Countries:
    return checkCountriesFor(player);
    break;
  case Goal::Continents:
    return checkContinentsFor(player);
    break;
  default:
    return ((int) player->getNbCountries() >= m_automaton->game()->theWorld()->getCountries().size());
  }
}

bool Goal::checkCountriesFor(const GameLogic::Player* player) const
{
  qCDebug(KSIRK_LOG) << player->name();
  if (player->getNbCountries() >= m_nbCountries)
  {
    uint nbCountriesOk = 0;
    for (Country* country: player->countries())
    {
      if (country->nbArmies() >= m_nbArmiesByCountry)
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
  qCDebug(KSIRK_LOG) << "Goal::checkContinentsFor " << player->name();
  for (const QString& continent: m_continents)
  {
    qCDebug(KSIRK_LOG) << "Should be owned continent id: " << continent;
    if ( m_automaton->game()->theWorld()->continentNamed(continent) == nullptr
       || m_automaton->game()->theWorld()->continentNamed(continent)->owner() != player)
    {
      return false;
    }
  }
  if (!m_continents.contains(QString()))
  {
    return true;
  }
  bool otherFound = false;
  QList<Continent*>& continents = m_automaton->game()->theWorld()->getContinents();
  for (Continent* continent: continents)
  {
    if ( ( !m_continents.contains(continent->name()) )
          && ( continent->owner() == player) )
    {
      otherFound = true;
      break;
    }
  }
  return otherFound;
}

QString Goal::message(int displayType) const
{
  qCDebug(KSIRK_LOG);
  KLocalizedString res;

  QList<QString>::const_iterator it, it_end, it_next;
  
  if(type()==NoGoal) {
    QString mes = i18n("You must conquer the World!");
    return mes;
  }
  else
  {
    if (displayType & GoalDesc)
    {
      res = ki18n(m_description.toUtf8().data());
      if (m_player == nullptr)
      {
        res = res.subs("");
      }
      else
      {
        res = res.subs(m_player->name());
      }
      qCDebug(KSIRK_LOG) << "Goal type='" << m_type << "' mes = '" << res.toString() << "'";
    
      QList<unsigned int>::const_iterator it, it_end;
      switch (m_type)
      {
      case Goal::GoalPlayer :
        if (!m_players.empty())
        {
          qCDebug(KSIRK_LOG) << "  player name='" << *m_players.begin();
          qCDebug(KSIRK_LOG) << "  this is player='" << m_automaton->playerNamed(*m_players.begin());
          if (m_automaton->playerNamed(*m_players.begin())==nullptr)
          {
            res = res.subs(i18n("%1 (already dead)",*m_players.begin()));
          }
          else
          {
            qCDebug(KSIRK_LOG) << "  its name is='" << *m_players.begin();
            res = res.subs(*m_players.begin());
          }
          res = res.subs(m_nbCountries);
        }
        else
        {
          res = res.subs(i18n("Error: no player to destroy"));
        }
        break;
      case Goal::Countries:
        qCDebug(KSIRK_LOG) << "  arg1 = '" << m_nbCountries << "'";
        res=res.subs(m_nbCountries);
        if (m_nbArmiesByCountry > 0)
        {
          qCDebug(KSIRK_LOG) << "  arg2 = '" << m_nbArmiesByCountry << "'";
          res=res.subs(m_nbArmiesByCountry);
        }
        break;
      case Goal::Continents:
        for (const QString& continent: m_continents)
        {
          if (!continent.isEmpty())
          {
            qCDebug(KSIRK_LOG) << "  arg = '" << continent << "'";
            res=res.subs(i18n(continent.toUtf8().data()));
          }
        }
        break;
      default:;
      }
    }
  }

  QString mes = res.toString();
  if (displayType & GoalAdvance)
  {
    int diff = 0;
    switch (m_type)
    {
    case Goal::GoalPlayer :
      if (m_automaton->playerNamed(*m_players.begin()) != nullptr)
      {
        mes += i18n("<br>%1 is still alive…", *m_players.begin());
      }
      else
      {
        diff  = m_nbCountries-m_player->countries().size();
        if (diff > 0)
        {
          mes += i18np("<br>%2, you still have 1 country to conquer…", "<br>%2, you still have %1 countries to conquer…", diff, m_player->name());
        }
        else
        {
          mes += i18np("<br>Your goal is reached: %2 is dead and you possess %1 country.",
                       "<br>Your goal is reached: %2 is dead and you possess %1 countries.",
                       m_nbCountries, *m_players.begin());
        }
      }
    break;
    case Goal::Countries:
      diff  = m_nbCountries-m_player->countries().size();
      if (diff > 0)
      {
        mes += i18np("<br>%2, you still have 1 country to conquer…", "<br>%2, you still have %1 countries to conquer…", diff, m_player->name());
      }
      else
      {
        int nbOk = 0;
        for (int i = 0; i < m_player->countries().size();++i)
        {
          if (m_player->countries().at(i)->nbArmies() >= m_nbArmiesByCountry)
          {
            nbOk++;
          }
        }
        mes += i18np("<br>%2, you have enough countries but you still have to put more than 1 army on %3 of them…", "<br>%2, you have enough countries but you still have to put more than %1 armies on %3 of them…", m_nbArmiesByCountry, m_player->name(), m_nbCountries-nbOk);
        
      }
      break;
    case Goal::Continents:
      mes += i18n("<br>%1, you still have to conquer ",m_player->name());
      it = m_continents.begin(); it_end = m_continents.end();
      if (*it != nullptr)
      {
        Continent* continent = const_cast<Continent*>(m_automaton->game()->theWorld()->continentNamed(*it));
        int nb = continent->getMembers().size() - continent->countriesOwnedBy(m_player).size();
        mes += i18np("1 country in %2","%1 countries in %2",nb,i18n(continent->name().toUtf8().data()));
      }
      it++;
      while (it != it_end)
      {
        it_next = it;
        it_next++;
        QString joint;
        if (it_next==it_end)
        {
          joint = i18nc("@item:intext country list separator of last item", " and ");
        }
        else
        {
          joint = i18nc("@item:intext country list separator of non-last item", ", ");
        }
        if (*it != nullptr)
        {
          Continent* continent = const_cast<Continent*>(m_automaton->game()->theWorld()->continentNamed(*it));
          int nb = continent->getMembers().size() - continent->countriesOwnedBy(m_player).size();
          mes += joint + i18nc("@info An element of the enumeration of the number of countries in the given continent", "%1 in %2",nb,i18n(continent->name().toUtf8().data()));
        }
        it++;
      }
      mes += '.';
      break;
    default:
      break;
;
    }
  }

  return mes;
}
  
void Goal::show(int displayType)
{
  qCDebug(KSIRK_LOG) << message(displayType);
//   m_automaton->game()->showMessage(message(displayType),5, KGameWindow::ForceShowing);
  KMessageBox::information(
                            m_automaton->game(),
                            message(displayType), 
                            i18nc("@title:window", "Goal Display"));
}

QDataStream& operator<<(QDataStream& stream, const Goal& goal)
{
  qCDebug(KSIRK_LOG) << "Goal operator<< : type" << goal.type();
  stream << quint32(goal.type());
  if (goal.player() != nullptr)
  {
    qCDebug(KSIRK_LOG) << "Goal operator<< : player " << goal.player()->id();
    stream << quint32(goal.player()->id());
  }
  else
  {
    qCDebug(KSIRK_LOG) << "Goal operator<< : player " << 0 ;
    stream << quint32(0);
  }
  qCDebug(KSIRK_LOG) << "Goal operator<< : description " << goal.description();
  stream << goal.description();
  QList<QString>::ConstIterator it, it_end;
  QList<QString>::ConstIterator itc, itc_end;
  switch (goal.type())
  {
  case Goal::GoalPlayer :
    qCDebug(KSIRK_LOG) << "Goal operator<< : players " << goal.players().size();
    stream << quint32(goal.players().size());
    it = goal.players().constBegin(); it_end = goal.players().constEnd();
    for (; it != it_end; it++)
    {
      qCDebug(KSIRK_LOG) << "Goal operator<< : player " << (*it);
      stream << *it;
    }
    qCDebug(KSIRK_LOG) << "Goal operator<< : nbCountries " << goal.nbCountries();
    stream << quint32(goal.nbCountries());
    break;
  case Goal::Countries:
    qCDebug(KSIRK_LOG) << "Goal operator<< : nbCountries " << goal.nbCountries();
    stream << quint32(goal.nbCountries());
    qCDebug(KSIRK_LOG) << "Goal operator<< : nbArmiesByCountry " << goal.nbArmiesByCountry();
    stream << quint32(goal.nbArmiesByCountry());
    break;
  case Goal::Continents:
    qCDebug(KSIRK_LOG) << "Goal operator<< : continents " << goal.continents().size();
    stream << quint32(goal.continents().size());
    itc = goal.continents().constBegin(); itc_end = goal.continents().constEnd();
    for (; itc != itc_end; itc++)
    {
      qCDebug(KSIRK_LOG) << "Goal operator<< : continent " << (*itc);
      stream << (*itc);
    }
    break;
  default: break;
  }
  return stream;
}

QDataStream& operator>>(QDataStream& stream, Goal& goal)
{
  qCDebug(KSIRK_LOG) << "Goal operator>>";
  quint32 type;
  QString description;
  QString playerName;
  quint32 nb, nbp;
  QString id;
  quint32 ownerId;
  stream >> type;
  qCDebug(KSIRK_LOG) << "Goal operator>> type: " << type ;
  stream >> ownerId;
  qCDebug(KSIRK_LOG) << "Goal operator>> ownerId: " << ownerId ;
  goal.player(static_cast<Player*>(goal.m_automaton->findPlayer(ownerId)));
  goal.type(Goal::GoalType(type));
  stream >> description;
  qCDebug(KSIRK_LOG) << "Goal operator>> description: " << description ;
  goal.description(description);
  switch (type)
  {
  case Goal::GoalPlayer :
    goal.players().clear();
    stream >> nbp;
    qCDebug(KSIRK_LOG) << "Goal operator>> nbp: " << nbp ;
    for (quint32 i = 0; i < nbp; i++)
    {
      stream >> playerName;
      qCDebug(KSIRK_LOG) << "Goal operator>> player name: " << playerName ;
      goal.players().push_back(playerName);
    }
    stream >> nb;
    qCDebug(KSIRK_LOG) << "Goal operator>> nbCountries: " << nb ;
    goal.nbCountries(nb);
    break;
  case Goal::Countries:
    stream >> nb;
    qCDebug(KSIRK_LOG) << "Goal operator>> nbCountries: " << nb ;
    goal.nbCountries(nb);
    stream >> nb;
    goal.nbArmiesByCountry(nb);
    qCDebug(KSIRK_LOG) << "Goal operator>> nbArmiesByCountry: " << nb ;
    break;
  case Goal::Continents:
    stream >> nb;
    qCDebug(KSIRK_LOG) << "Goal operator>> nbContinents: " << nb ;
    goal.continents().clear();
    for (quint32 i = 0; i < nb; i++)
    {
      stream >> id;
      qCDebug(KSIRK_LOG) << "Goal operator>> continent: " << id ;
      goal.continents().push_back(id);
    }
    break;
  default:;
  }
  return stream;
}

void Goal::saveXml(QTextStream& xmlStream) const
{
  
  xmlStream << "<goal player=\"";
  if (m_player != nullptr)
  {
    xmlStream << m_player->name();
  }
  xmlStream << "\" type=\"" << m_type << "\" description=\"" << m_description;
  xmlStream << "\" nbCountries=\"" << m_nbCountries << "\" nbArmiesByCountry=\"" << m_nbArmiesByCountry << "\">";
  xmlStream << "<continents>";
  QList<QString>::ConstIterator itc, itc_end;
  itc = continents().constBegin(); itc_end = continents().constEnd();
  for (; itc != itc_end; itc++)
  {
    QString name = ((*itc).isEmpty())?"":(*itc);
    xmlStream << "<continent name=\"" << name << "\"/>";
  }
  xmlStream << "</continents>";
  xmlStream << "<players>";
  QList<QString>::ConstIterator itp, itp_end;
  itp = m_players.constBegin(); itp_end = m_players.constEnd();
  for (; itp != itp_end; itp++)
  {
    xmlStream << "<player name=\"" << (*itp) << "\"/>";
  }
  xmlStream << "</players>";
  xmlStream << "</goal>";
}

}

}
