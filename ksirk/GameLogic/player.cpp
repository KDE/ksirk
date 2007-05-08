/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gaël de Chalendar <kleag@free.fr>

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

/*  begin                : Wed Jul 18 2001 */

#define KDE_NO_COMPAT
#include "player.h"
#include "Sprites/backgnd.h"
#include "Sprites/skinSpritesData.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/onu.h"
#include "kgamewin.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

namespace Ksirk
{

namespace GameLogic
{

unsigned int Player::m_uid = 0;

Player::Player(
    GameAutomaton* automaton,
    const QString& playerName, 
    unsigned int nbArmies, 
    Nationality* nation) :
  KPlayer(),
  m_automaton(automaton),
  m_nation(nation),
  m_goal(automaton),
  m_delayedInitNationName(""),
  m_waitedAck(0),
  m_flag(0)
{
  kDebug() << "Player constructor" << endl;
  setAsyncInput(true);
  dataHandler()->setPolicy(KGamePropertyBase::PolicyClean,false);
  m_nbAttack.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_nbAttack"));
  m_nbCountries.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_nbCountries"));
  m_nbAvailArmies.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_nbAvailArmies"));
  m_nbDefense.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_nbDefense"));
  m_password.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_password"));
  
  m_nbAttack = 0;
  m_nbDefense = 0;
  m_nbCountries = 0;
  m_nbAvailArmies = nbArmies;
  m_password = "";
  
  setName(playerName);
  setFlag();
  kDebug() << "Done creating player" << endl;
}

bool Player::operator==(const Player& player) const
{
  return (name() == player.name());
}


/**  */
unsigned int Player::getNbAvailArmies(){
    return (m_nbAvailArmies);
}

/**  */
void Player::setNbAvailArmies(unsigned int nb, bool /*transmit*/)
{
//   kDebug() << name() << " setNbAvailArmies: " << nb << " transmit=" << transmit << endl;
  m_nbAvailArmies = nb;
/*  if (transmit)
  {
    QByteArray buffer;
    QDataStream stream(buffer, QIODevice::WriteOnly);
    stream << name() << m_nbAvailArmies;
    GameAutomaton::changeable().sendMessage(buffer, PlayerAvailArmies);
  }*/
}

/**  */
unsigned int Player::getNbAttack()
{
    return (m_nbAttack);
}

/**  */
void Player::setNbAttack(unsigned int nb)
{
    m_nbAttack = nb;
}

/**  */
unsigned int Player::getNbDefense()
{
    return (m_nbDefense);
}

/**  */
void Player::setNbDefense(unsigned int nb)
{
    m_nbDefense = nb;
}

/** */
unsigned int Player::getNbCountries() const
{
    return (m_nbCountries);
}

/** add nb armies to the number of available armies */
void Player::incrNbAvailArmies(unsigned int nb)
{
  setNbAvailArmies(m_nbAvailArmies + nb);
}

/** remove nb armies to the number of available armies */
void Player::decrNbAvailArmies(unsigned int nb)
{
//   kDebug() << "Player::decrNbAvailArmies " << name() << " " << nb << endl;
  if (nb > m_nbAvailArmies)
  {
    kError() << "Removing " << nb << " armies while owning " << m_nbAvailArmies << endl;
    exit(1);
  }
  setNbAvailArmies(m_nbAvailArmies - nb);
}

/**  */
void Player::setNbCountries(unsigned int nb)
{
  m_nbCountries = nb;
}

/** add nb countries to the player */
void Player::incrNbCountries(unsigned int nb)
{
  setNbCountries(m_nbCountries + nb);
}

/** Enleve nb pays au player */
void Player::decrNbCountries(unsigned int nb)
{
    if (nb > m_nbCountries)
    {
      kError() << "Removing " << nb << " countries to " << name() << " while owning " << m_nbCountries << endl;
      exit(1);
    }
  setNbCountries(m_nbCountries - nb);
}

/**
  * This function returns the flag associated to the nationality of the player
  */
const AnimSprite* Player::getFlag() const
{
//   kDebug() << "Player::getFlag" << endl;
  return m_flag;
}

/**
  * This function returns the filename of the flag associated to the nationality of the player
  */
const QString& Player::flagFileName() const
{
    return m_nation-> flagFileName();
}

/**
  * Returns false (a Player is not an AI)
  */
bool Player::isAI() const
{
    return false;
}

void Player::saveXml(std::ostream& xmlStream)
{
  xmlStream << "<player ai=\"false\" ";
  innerSaveXml(xmlStream);
  xmlStream << " />" << std::endl;
}

void Player::innerSaveXml(std::ostream& xmlStream)
{
  QString theName = name().toUtf8();
  theName = theName.replace("&","&amp;");
  theName = theName.replace("<","&lt;");
  theName = theName.replace(">","&gt;");
  xmlStream << " name=\"" << theName.toUtf8().data() << "\"";
  xmlStream << " nbCountries=\"" << m_nbCountries << "\"";
  xmlStream << " nbAvailArmies=\"" << m_nbAvailArmies << "\"";
  xmlStream << " nbAttack=\"" << m_nbAttack << "\"";
  xmlStream << " nbDefense=\"" << m_nbDefense << "\"";
  QString nationName = m_nation->name();
  nationName = nationName.replace("&","&amp;");
  nationName = nationName.replace("<","&lt;");
  nationName = nationName.replace(">","&gt;");
  xmlStream << " nation=\"" << nationName.toUtf8().data() << "\"";
  xmlStream << " password=\"" << m_password.value().toUtf8().data() << "\"";
}

bool   Player::load (QDataStream &stream)
{
//   kDebug() << "Player::load" << endl;
  if (!KPlayer::load(stream)) return false;
  QString nationName;
  stream >> nationName;
//   kDebug() << "Player::load nationName=" << nationName << endl;
  setNation(nationName);
  stream >> m_goal;
  return true;
}

bool Player::save (QDataStream &stream)
{
//   kDebug() << "Player::save" << endl;
  if (!KPlayer::save(stream)) return false;
  stream << m_nation->name();
  stream << m_goal;
  return true;
}

Nationality* Player::getNation() 
{
  kDebug() << "Player::getNation for " << name() << endl;
  if (m_nation == 0 && !m_delayedInitNationName.isEmpty())
  {
    kError() << "  retrieving delayed nation " << m_delayedInitNationName << endl;
    setNation(m_delayedInitNationName);
  }
  return m_nation;
}

void Player::setNation(const QString& nationName)
{
  m_nation = m_automaton->game()->theWorld()-> nationNamed(nationName);
  if (m_nation == 0)
  {
//     kDebug() << "Delaying nation initialization ("<<nationName<<") for " << name() << endl;
    m_delayedInitNationName = nationName;
  }
  setFlag();
}

void Player::setFlag()
{
/*  if (m_flag != 0)
    delete m_flag;*/
  m_flag = 0;
  if (m_nation != 0)
  {
    m_flag = new AnimSprite(
                            m_nation->flagFileName(), 
                            m_automaton->game()->backGnd(), 
                            Sprites::SkinSpritesData::single().intData("flag-frames"), 
                            Sprites::SkinSpritesData::single().intData("flag-versions"),
                            m_automaton->game()->backGnd()->onu()->zoom());
    m_flag->hide();
  }
}

void Player::goal(const Goal& goal)
{
  kDebug() << "Player::goal (setter) " << name() << endl;
/*  if (m_goal)
  {
    delete m_goal;
  }*/
  m_goal = Goal(goal);
  m_goal.player(this);
/*  if (!isVirtual() && !isAI())
  {
    KMessageBox::information(
      GameAutomaton::changeable().game(),
      i18n("%1, your goal will be displayed. Please make sure that no other player can see it !",name()),
      i18n("KsirK - Displaying Goal"));
    m_goal->show();
  }*/
}

bool Player::checkGoal()
{
  return m_goal.checkFor(this);
}

/**
  * Returns the list of the countries owned by this player
  */
std::vector<Country*> Player::countries() const
{
//   kDebug() << name() << ": Player::countries()" << endl;
  std::vector<Country*> list;
  for ( unsigned int i = 0; i < m_automaton->game()->theWorld()->getCountries().size(); i++ )
  {
    Country* c = m_automaton->game()->theWorld()->getCountries().at(i);
    if (c-> owner() == static_cast< const Player * >(this))
    {
//            kDebug() << "\t" << c-> name() << endl;
      
      list.push_back(c);
    }
  }
//    kDebug() << name() << ": OUT AIPlayer::countries()" << endl;
  return list;
}

bool Player::acknowledge(unsigned int ack)
{
  if (ack == m_waitedAck)
  {
    m_waitedAck = 0;
    return true;
  }
  else
  {
    return false;
  }
}

QDataStream& operator<<(QDataStream& stream, PlayerMatrix& p)
{
  stream << p.name << quint32(p.nbAttack) << quint32(p.nbCountries) << quint32(p.nbAvailArmies)
    << quint32(p.nbDefense) << p.nation << quint32(p.isAI) << quint32(p.countries.size());
  std::set<QString>::iterator it, it_end;
  it = p.countries.begin(); it_end = p.countries.end();
  for (; it != it_end; it++)
  {
    stream << (*it);
  }
  stream << p.goal;
  return stream;
}

QDataStream& operator>>(QDataStream& stream, PlayerMatrix& p)
{
  quint32 nbCountries;
  stream >> p.name >> p.nbAttack >> p.nbCountries >> p.nbAvailArmies >> p.nbDefense >> p.nation;
  quint32 isAI;
  stream >> isAI;
  p.isAI = (bool)isAI;
  stream >> nbCountries;
  for (quint32 i = 0 ; i < nbCountries; i++)
  {
    QString country;
    stream >> country;
    p.countries.insert(country);
  }
  stream >> p.goal;
  return stream;
}

} // closing namespace GameLogic

} // closing namespace Ksirk

#include "player.moc"
