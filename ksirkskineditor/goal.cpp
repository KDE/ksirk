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
#include "onu.h"

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

namespace KsirkSkinEditor
{

Goal::Goal() :
  m_type(NoGoal),
  m_description(""),
  m_nbCountries(0),
  m_nbArmiesByCountry(0),
  m_continents(),
  m_players()
{
}

Goal::Goal(const Goal& goal)
{
  kDebug() << "Goal copy constructor :" << endl;
  m_type = goal.m_type;
  kDebug() << "  type="<< m_type << endl;
  m_description = goal.m_description;
  kDebug() << "  description="<< m_description << endl;
  m_nbCountries = goal.m_nbCountries;
  kDebug() << "  nbCountries="<< m_nbCountries << endl;
  m_nbArmiesByCountry = goal.m_nbArmiesByCountry;
  kDebug() << "  nbArmiesByCountry="<< m_nbArmiesByCountry << endl;
  m_continents = goal.m_continents;
  kDebug() << "  continents: "<< m_continents.size() << endl;
  m_players = goal.m_players;
  kDebug() << "  players: "<< m_players.size() << endl;
//   m_player = goal.m_player;
//   kDebug() << "  player: "<< m_player << endl;
}

Goal::~Goal()
{
}

QString Goal::message(int displayType) const
{
  kDebug();
  KLocalizedString res;

//   std::set<unsigned int>::const_iterator it, it_end, it_next;
  
  if(type()==NoGoal)
  {
    QString mes = (QString)"You must conquer the World !";
    return mes;
  }
  else
  {
    if (displayType & GoalDesc)
    {
      res = ki18n(m_description.toUtf8().data());
/*      if (m_player == 0)
      {
        res = res.subs("");
      }
      else
      {
        res = res.subs(m_player->name());
      }*/
      kDebug() << "Goal type='" << m_type << "' mes = '" << res.toString() << "'" << endl;
    
      QList<unsigned int>::const_iterator it, it_end;
      switch (m_type)
      {
      case Goal::GoalPlayer :
        if (!m_players.empty())
        {
          kDebug() << "  player name='" << *m_players.begin() << endl;
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
//         it = m_continents.begin(); it_end = m_continents.end();
//         for (; it != it_end; it++)
//         {
//           if (*it != 0)
//           {
/*            kDebug() << "  arg = '" << m_automaton->game()->theWorld()->continentWithId(*it)->name() << "'" << endl;
            res=res.subs(i18n(m_automaton->game()->theWorld()->continentWithId(*it)->name().toUtf8().data()));*/
//           }
//         }
        break;
      default:;
      }
    }
  }

  QString mes = res.toString();
  if (displayType & GoalAdvance)
  {
//     int diff = 0;
    switch (m_type)
    {
    case Goal::GoalPlayer :
/*      if (m_automaton->playerNamed(*m_players.begin()) != 0)
      {
        mes += i18n("<br>%1 is still alive...",*m_players.begin());
      }
      else
      {
        mes += i18n("<br>You now have to conquer %1",m_nbCountries);
      }*/
    break;
    case Goal::Countries:
//       diff  = m_nbCountries-m_player->countries().size();
//       if (diff > 0)
//       {
//         mes += i18n("<br>%1, you have still %2 countries to conquer...",m_player->name(),diff);
//       }
//       else
//       {
//         int nbOk = 0;
//         for (unsigned int i = 0; i < m_player->countries().size();i++)
//         {
//           if (m_player->countries().at(i)->nbArmies() >= m_nbArmiesByCountry)
//           {
//             nbOk++;
//           }
//         }
//         mes += i18n("<br>%1, you have enough countries but you still have to put more than %2 armies on %3 of them...",m_player->name(),m_nbArmiesByCountry,m_nbCountries-nbOk);
//         
//       }
      break;
    case Goal::Continents:
/*      mes += i18n("<br>%1, you still have to conquer ",m_player->name());
      it = m_continents.begin(); it_end = m_continents.end();
      if (*it != 0)
      {
        Continent* continent = const_cast<Continent*>(m_automaton->game()->theWorld()->continentWithId(*it));
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
          joint = i18n(" and ");
        }
        else
        {
          joint = i18n(", ");
        }
        if (*it != 0)
        {
          Continent* continent = const_cast<Continent*>(m_automaton->game()->theWorld()->continentWithId(*it));
          int nb = continent->getMembers().size() - continent->countriesOwnedBy(m_player).size();
          mes += joint + i18nc("@info An element of the enumeration of the number of countries in the given continent", "%1 in %2",nb,continent->name());
        }
        it++;
      }
      mes += '.';*/
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
  kDebug() << message(displayType);
//   m_automaton->game()->showMessage(message(displayType),5, KGameWindow::ForceShowing);
  KMessageBox::information(
                            0,
                            message(displayType), 
                            i18n("KsirK - Goal Display"));
}

QDataStream& operator<<(QDataStream& stream, const Goal& goal)
{
  kDebug() << "Goal operator<< : type" << goal.type()<< endl;
  stream << quint32(goal.type());
/*  if (goal.player() != 0)
  {
    kDebug() << "Goal operator<< : player " << goal.player()->id() << endl;
    stream << quint32(goal.player()->id());
  }
  else
  {
    kDebug() << "Goal operator<< : player " << 0 << endl;
    stream << quint32(0);
  }*/
  kDebug() << "Goal operator<< : description " << goal.description() << endl;
  stream << goal.description();
  QList<QString>::ConstIterator it, it_end;
  QList<unsigned int>::const_iterator itc, itc_end;
  switch (goal.type())
  {
  case Goal::GoalPlayer :
    kDebug() << "Goal operator<< : players " << goal.players().size() << endl;
    stream << quint32(goal.players().size());
    it = goal.players().constBegin(); it_end = goal.players().constEnd();
    for (; it != it_end; it++)
    {
      kDebug() << "Goal operator<< : player " << (*it) << endl;
      stream << *it;
    }
    kDebug() << "Goal operator<< : nbCountries " << goal.nbCountries() << endl;
    stream << quint32(goal.nbCountries());
    break;
  case Goal::Countries:
    kDebug() << "Goal operator<< : nbCountries " << goal.nbCountries() << endl;
    stream << quint32(goal.nbCountries());
    kDebug() << "Goal operator<< : nbArmiesByCountry " << goal.nbArmiesByCountry() << endl;
    stream << quint32(goal.nbArmiesByCountry());
    break;
  case Goal::Continents:
    kDebug() << "Goal operator<< : continents " << goal.continents().size() << endl;
    stream << quint32(goal.continents().size());
    it = goal.continents().begin(); it_end = goal.continents().end();
    for (; it != it_end; it++)
    {
      kDebug() << "Goal operator<< : continent " << (*it) << endl;
      stream << (*it);
    }
    break;
  default: break;
  }
  return stream;
}

QDataStream& operator>>(QDataStream& stream, Goal& goal)
{
  kDebug() << "Goal operator>>" << endl;
  quint32 type;
  QString description;
  QString playerName;
  quint32 nb, nbp;
  QString id;
  quint32 ownerId;
  stream >> type;
  kDebug() << "Goal operator>> type: " << type << endl;
  stream >> ownerId;
  kDebug() << "Goal operator>> ownerId: " << ownerId << endl;
  goal.setType(Goal::GoalType(type));
  stream >> description;
  kDebug() << "Goal operator>> description: " << description << endl;
  goal.setDescription(description);
  switch (type)
  {
  case Goal::GoalPlayer :
    goal.players().clear();
    stream >> nbp;
    kDebug() << "Goal operator>> nbp: " << nbp << endl;
    for (quint32 i = 0; i < nbp; i++)
    {
      stream >> playerName;
      kDebug() << "Goal operator>> player name: " << playerName << endl;
      goal.players().push_back(playerName);
    }
    stream >> nb;
    kDebug() << "Goal operator>> nbCountries: " << nb << endl;
    goal.setNbCountries(nb);
    break;
  case Goal::Countries:
    stream >> nb;
    kDebug() << "Goal operator>> nbCountries: " << nb << endl;
    goal.setNbCountries(nb);
    stream >> nb;
    goal.setNbArmiesByCountry(nb);
    kDebug() << "Goal operator>> nbArmiesByCountry: " << nb << endl;
    break;
  case Goal::Continents:
    stream >> nb;
    kDebug() << "Goal operator>> nbContinents: " << nb << endl;
    goal.continents().clear();
    for (quint32 i = 0; i < nb; i++)
    {
      stream >> id;
      kDebug() << "Goal operator>> continent: " << id << endl;
      goal.continents().push_back(id);
    }
    break;
  default:;
  }
  return stream;
}

void Goal::saveXml(std::ostream& xmlStream) const
{
  
  xmlStream << "<goal player=\"";
/*  if (m_player != 0)
  {
    xmlStream << m_player->name().toUtf8().data();
  }*/
  xmlStream << "\" type=\"" << m_type << "\" description=\"" << m_description.toUtf8().data();
  xmlStream << "\" nbCountries=\"" << m_nbCountries << "\" nbArmiesByCountry=\"" << m_nbArmiesByCountry << "\">\n";
  xmlStream << "<continents>\n";
  QList<QString>::const_iterator itc, itc_end;
  itc = continents().begin(); itc_end = continents().end();
  for (; itc != itc_end; itc++)
  {
//     QString name = (*itc==0)?"":m_automaton->game()->theWorld()->continentWithId(*itc)->name();
//     xmlStream << "<continent name=\"" << name.toUtf8().data() << "\"/>\n";
  }
  xmlStream << "</continents>\n";
  xmlStream << "<players>\n";
  QList<QString>::ConstIterator itp, itp_end;
  itp = m_players.constBegin(); itp_end = m_players.constEnd();
  for (; itp != itp_end; itp++)
  {
    xmlStream << "<player name=\"" << (*itp).toUtf8().data() << "\"/>\n";
  }
  xmlStream << "</players>\n";
  xmlStream << "</goal>\n";
  
    
}

}
