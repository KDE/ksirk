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
#include "gameautomaton.h"

#include "ksirk_debug.h"
#include <KLocalizedString>
#include <kmessagebox.h>

namespace Ksirk {

namespace GameLogic {

Goal::Goal() :
  m_automaton(0),
  m_type(NoGoal),
  m_description(""),
  m_nbCountries(0),
  m_nbArmiesByCountry(0),
  m_continents(),
  m_players(),
  m_player(0)
{
}

Goal::Goal(GameAutomaton* automaton) :
m_automaton(automaton),
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
  m_automaton = goal.m_automaton;
  qCDebug(KSIRK_LOG) << "Goal copy constructor :" << endl;
  m_type = goal.m_type;
  qCDebug(KSIRK_LOG) << "  type="<< m_type << endl;
  m_description = goal.m_description;
  qCDebug(KSIRK_LOG) << "  description="<< m_description << endl;
  m_nbCountries = goal.m_nbCountries;
  qCDebug(KSIRK_LOG) << "  nbCountries="<< m_nbCountries << endl;
  m_nbArmiesByCountry = goal.m_nbArmiesByCountry;
  qCDebug(KSIRK_LOG) << "  nbArmiesByCountry="<< m_nbArmiesByCountry << endl;
  m_continents = goal.m_continents;
  qCDebug(KSIRK_LOG) << "  continents: "<< m_continents.size() << endl;
  m_players = goal.m_players;
  qCDebug(KSIRK_LOG) << "  players: "<< m_players.size() << endl;
  m_player = goal.m_player;
  qCDebug(KSIRK_LOG) << "  player: "<< m_player << endl;
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
    if (m_automaton->playerNamed(*m_players.begin()) != 0)
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
    foreach (Country* country, player->countries())
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
  qCDebug(KSIRK_LOG) << "Goal::checkContinentsFor " << player->name() << endl;
  foreach (const QString& continent, m_continents)
  {
    qCDebug(KSIRK_LOG) << "Should be owned continent id: " << continent<< endl;
    if ( m_automaton->game()->theWorld()->continentNamed(continent) == 0
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
  foreach (Continent* continent, continents)
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
      if (m_player == 0)
      {
        res = res.subs("");
      }
      else
      {
        res = res.subs(m_player->name());
      }
      qCDebug(KSIRK_LOG) << "Goal type='" << m_type << "' mes = '" << res.toString() << "'" << endl;
    
      QList<unsigned int>::const_iterator it, it_end;
      switch (m_type)
      {
      case Goal::GoalPlayer :
        if (!m_players.empty())
        {
          qCDebug(KSIRK_LOG) << "  player name='" << *m_players.begin() << endl;
          qCDebug(KSIRK_LOG) << "  this is player='" << m_automaton->playerNamed(*m_players.begin()) << endl;
          if (m_automaton->playerNamed(*m_players.begin())==0)
          {
            res = res.subs(i18n("%1 (already dead)",*m_players.begin()));
          }
          else
          {
            qCDebug(KSIRK_LOG) << "  its name is='" << *m_players.begin() << endl;
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
        qCDebug(KSIRK_LOG) << "  arg1 = '" << m_nbCountries << "'" << endl;
        res=res.subs(m_nbCountries);
        if (m_nbArmiesByCountry > 0)
        {
          qCDebug(KSIRK_LOG) << "  arg2 = '" << m_nbArmiesByCountry << "'" << endl;
          res=res.subs(m_nbArmiesByCountry);
        }
        break;
      case Goal::Continents:
        foreach (const QString& continent, m_continents)
        {
          if (continent != QString())
          {
            qCDebug(KSIRK_LOG) << "  arg = '" << continent << "'" << endl;
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
      if (m_automaton->playerNamed(*m_players.begin()) != 0)
      {
        mes += i18n("<br>%1 is still alive...",*m_players.begin());
      }
      else
      {
        diff  = m_nbCountries-m_player->countries().size();
        if (diff > 0)
        {
          mes += i18np("<br>%2, you still have 1 country to conquer...","<br>%2, you still have %1 countries to conquer...",diff,m_player->name());
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
        mes += i18np("<br>%2, you still have 1 country to conquer...","<br>%2, you still have %1 countries to conquer...",diff,m_player->name());
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
        mes += i18np("<br>%2, you have enough countries but you still have to put more than 1 army on %3 of them...","<br>%2, you have enough countries but you still have to put more than %1 armies on %3 of them...",m_nbArmiesByCountry,m_player->name(),m_nbCountries-nbOk);
        
      }
      break;
    case Goal::Continents:
      mes += i18n("<br>%1, you still have to conquer ",m_player->name());
      it = m_continents.begin(); it_end = m_continents.end();
      if (*it != 0)
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
          joint = i18n(" and ");
        }
        else
        {
          joint = i18n(", ");
        }
        if (*it != 0)
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
                            i18n("KsirK - Goal Display"));
}

QDataStream& operator<<(QDataStream& stream, const Goal& goal)
{
  qCDebug(KSIRK_LOG) << "Goal operator<< : type" << goal.type()<< endl;
  stream << quint32(goal.type());
  if (goal.player() != 0)
  {
    qCDebug(KSIRK_LOG) << "Goal operator<< : player " << goal.player()->id() << endl;
    stream << quint32(goal.player()->id());
  }
  else
  {
    qCDebug(KSIRK_LOG) << "Goal operator<< : player " << 0 << endl;
    stream << quint32(0);
  }
  qCDebug(KSIRK_LOG) << "Goal operator<< : description " << goal.description() << endl;
  stream << goal.description();
  QList<QString>::ConstIterator it, it_end;
  QList<QString>::ConstIterator itc, itc_end;
  switch (goal.type())
  {
  case Goal::GoalPlayer :
    qCDebug(KSIRK_LOG) << "Goal operator<< : players " << goal.players().size() << endl;
    stream << quint32(goal.players().size());
    it = goal.players().constBegin(); it_end = goal.players().constEnd();
    for (; it != it_end; it++)
    {
      qCDebug(KSIRK_LOG) << "Goal operator<< : player " << (*it) << endl;
      stream << *it;
    }
    qCDebug(KSIRK_LOG) << "Goal operator<< : nbCountries " << goal.nbCountries() << endl;
    stream << quint32(goal.nbCountries());
    break;
  case Goal::Countries:
    qCDebug(KSIRK_LOG) << "Goal operator<< : nbCountries " << goal.nbCountries() << endl;
    stream << quint32(goal.nbCountries());
    qCDebug(KSIRK_LOG) << "Goal operator<< : nbArmiesByCountry " << goal.nbArmiesByCountry() << endl;
    stream << quint32(goal.nbArmiesByCountry());
    break;
  case Goal::Continents:
    qCDebug(KSIRK_LOG) << "Goal operator<< : continents " << goal.continents().size() << endl;
    stream << quint32(goal.continents().size());
    itc = goal.continents().constBegin(); itc_end = goal.continents().constEnd();
    for (; itc != itc_end; itc++)
    {
      qCDebug(KSIRK_LOG) << "Goal operator<< : continent " << (*itc) << endl;
      stream << (*itc);
    }
    break;
  default: break;
  }
  return stream;
}

QDataStream& operator>>(QDataStream& stream, Goal& goal)
{
  qCDebug(KSIRK_LOG) << "Goal operator>>" << endl;
  quint32 type;
  QString description;
  QString playerName;
  quint32 nb, nbp;
  QString id;
  quint32 ownerId;
  stream >> type;
  qCDebug(KSIRK_LOG) << "Goal operator>> type: " << type << endl;
  stream >> ownerId;
  qCDebug(KSIRK_LOG) << "Goal operator>> ownerId: " << ownerId << endl;
  goal.player(static_cast<Player*>(goal.m_automaton->findPlayer(ownerId)));
  goal.type(Goal::GoalType(type));
  stream >> description;
  qCDebug(KSIRK_LOG) << "Goal operator>> description: " << description << endl;
  goal.description(description);
  switch (type)
  {
  case Goal::GoalPlayer :
    goal.players().clear();
    stream >> nbp;
    qCDebug(KSIRK_LOG) << "Goal operator>> nbp: " << nbp << endl;
    for (quint32 i = 0; i < nbp; i++)
    {
      stream >> playerName;
      qCDebug(KSIRK_LOG) << "Goal operator>> player name: " << playerName << endl;
      goal.players().push_back(playerName);
    }
    stream >> nb;
    qCDebug(KSIRK_LOG) << "Goal operator>> nbCountries: " << nb << endl;
    goal.nbCountries(nb);
    break;
  case Goal::Countries:
    stream >> nb;
    qCDebug(KSIRK_LOG) << "Goal operator>> nbCountries: " << nb << endl;
    goal.nbCountries(nb);
    stream >> nb;
    goal.nbArmiesByCountry(nb);
    qCDebug(KSIRK_LOG) << "Goal operator>> nbArmiesByCountry: " << nb << endl;
    break;
  case Goal::Continents:
    stream >> nb;
    qCDebug(KSIRK_LOG) << "Goal operator>> nbContinents: " << nb << endl;
    goal.continents().clear();
    for (quint32 i = 0; i < nb; i++)
    {
      stream >> id;
      qCDebug(KSIRK_LOG) << "Goal operator>> continent: " << id << endl;
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
  if (m_player != 0)
  {
    xmlStream << m_player->name();
  }
  xmlStream << "\" type=\"" << m_type << "\" description=\"" << m_description;
  xmlStream << "\" nbCountries=\"" << m_nbCountries << "\" nbArmiesByCountry=\"" << m_nbArmiesByCountry << "\">" << endl;
  xmlStream << "<continents>" << endl;
  QList<QString>::ConstIterator itc, itc_end;
  itc = continents().constBegin(); itc_end = continents().constEnd();
  for (; itc != itc_end; itc++)
  {
    QString name = (*itc==QString())?"":(*itc);
    xmlStream << "<continent name=\"" << name << "\"/>" << endl;
  }
  xmlStream << "</continents>" << endl;
  xmlStream << "<players>" << endl;
  QList<QString>::ConstIterator itp, itp_end;
  itp = m_players.constBegin(); itp_end = m_players.constEnd();
  for (; itp != itp_end; itp++)
  {
    xmlStream << "<player name=\"" << (*itp) << "\"/>" << endl;
  }
  xmlStream << "</players>" << endl;
  xmlStream << "</goal>" << endl;
}

}

}
