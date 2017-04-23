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

#include "ksirk_debug.h"
#include <KLocalizedString>
#include <kmessagebox.h>
#include <KStringHandler>

namespace Ksirk
{

namespace GameLogic
{

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
  m_waitedAck(""),
  m_flag(0)
{
  qCDebug(KSIRK_LOG) << "Player constructor" << endl;
  setAsyncInput(true);
  dataHandler()->setPolicy(KGamePropertyBase::PolicyClean,false);
  m_nbAttack.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_nbAttack"));
  m_nbCountries.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_nbCountries"));
//   m_nbAvailArmies.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_nbAvailArmies"));
  m_nbDefense.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_nbDefense"));
  m_password.registerData(dataHandler(),KGamePropertyBase::PolicyClean,QString("m_password"));
  
  m_nbAttack = 0;
  m_nbDefense = 0;
  m_nbCountries = 0;
  m_distributionData.init(nbArmies, m_automaton->game()->theWorld()->getNbCountries());
//   m_nbAvailArmies = nbArmies;
  m_password = "";
  
  setName(playerName);
  setFlag();
  qCDebug(KSIRK_LOG) << "Done creating player" << endl;
}

bool Player::operator==(const Player& player) const
{
  return (name() == player.name());
}

void Player::reset()
{
  m_distributionData.init(0, m_automaton->game()->theWorld()->getNbCountries());
}


/**  */
unsigned int Player::getNbAvailArmies()
{
  return (m_distributionData.nbToPlace());
//   return (m_nbAvailArmies);
}

/**  */
void Player::setNbAvailArmies(unsigned int nb, bool transmit)
{
  qCDebug(KSIRK_LOG) << name() << " setNbAvailArmies: " << nb << " transmit=" << transmit << endl;
  m_distributionData.setNbToPlace(nb);
  if (transmit)
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << name() << m_distributionData.nbToPlace();
    m_automaton->sendMessage(buffer, PlayerAvailArmies);
  }
}

/**  */
unsigned int Player::getNbAttack()
{
  qCDebug(KSIRK_LOG) << m_nbAttack;
  return (m_nbAttack);
}

/**  */
void Player::setNbAttack(unsigned int nb)
{
  qCDebug(KSIRK_LOG) << name() << nb;
  m_nbAttack = nb;
}

/**  */
unsigned int Player::getNbDefense()
{
  qCDebug(KSIRK_LOG) << m_nbDefense;
  return (m_nbDefense);
}

/**  */
void Player::setNbDefense(unsigned int nb)
{
  qCDebug(KSIRK_LOG) << name() << nb;
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
  m_distributionData.setNbToPlace(m_distributionData.nbToPlace() + nb);
}

/** remove nb armies to the number of available armies */
void Player::decrNbAvailArmies(unsigned int nb)
{
//   qCDebug(KSIRK_LOG) << "Player::decrNbAvailArmies " << name() << " " << nb << endl;
  if (nb > (unsigned)m_distributionData.nbToPlace()/*m_nbAvailArmies*/)
  {
    qCCritical(KSIRK_LOG) << "Removing " << nb << " armies while owning " << m_distributionData.nbToPlace()/*m_nbAvailArmies*/ << endl;
    Q_ASSERT(false);
  }
  m_distributionData.setNbToPlace(m_distributionData.nbToPlace() - nb);
}

void Player::putArmiesInto(int nb, int country)
{
  qCDebug(KSIRK_LOG) << nb << country << m_distributionData[country];
  if (nb > m_distributionData.nbToPlace()/*m_nbAvailArmies*/)
  {
    qCCritical(KSIRK_LOG) << "Removing " << nb << " armies while owning " << m_distributionData.nbToPlace()/*m_nbAvailArmies*/ << endl;
    exit(1);
  }
  m_distributionData.setNbToPlace(m_distributionData.nbToPlace() - nb);
  m_distributionData[country] = m_distributionData[country] + nb;
}

void Player::removeArmiesFrom(int nb, int country)
{
  qCDebug(KSIRK_LOG) << nb << m_distributionData[country];
  if (nb > m_distributionData[country])
  {
    qCCritical(KSIRK_LOG) << "Trying to remove " << nb << " armies while x were added: x=" << m_distributionData[country] << endl;
    exit(1);
  }
  m_distributionData.setNbToPlace(m_distributionData.nbToPlace() + nb);
  m_distributionData[country] = m_distributionData[country] - nb;
}

bool Player::canRemoveArmiesFrom(int nb, int country)
{
  qCDebug(KSIRK_LOG) << nb << m_distributionData[country];
  return (nb <= m_distributionData[country]);
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
      qCCritical(KSIRK_LOG) << "Removing " << nb << " countries to " << name() << " while owning " << m_nbCountries << endl;
      exit(1);
    }
  setNbCountries(m_nbCountries - nb);
}

/**
  * This function returns the flag associated to the nationality of the player
  */
const AnimSprite* Player::getFlag() const
{
//   qCDebug(KSIRK_LOG) << "Player::getFlag" << endl;
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

void Player::saveXml(QTextStream& xmlStream)
{
  xmlStream << "<player ai=\"false\" ";
  innerSaveXml(xmlStream);
  xmlStream << " />" << endl;
}

void Player::innerSaveXml(QTextStream& xmlStream)
{
  QString theName = name();
  theName = theName.replace("&","&amp;");
  theName = theName.replace("<","&lt;");
  theName = theName.replace(">","&gt;");
  xmlStream << " name=\"" << theName << "\"";
  xmlStream << " nbCountries=\"" << m_nbCountries << "\"";
  xmlStream << " nbAvailArmies=\"" << m_distributionData.nbToPlace() << "\"";
  xmlStream << " nbAttack=\"" << m_nbAttack << "\"";
  xmlStream << " nbDefense=\"" << m_nbDefense << "\"";
  QString nationName = m_nation->name();
  nationName = nationName.replace("&","&amp;");
  nationName = nationName.replace("<","&lt;");
  nationName = nationName.replace(">","&gt;");
  xmlStream << " nation=\"" << nationName << "\"";
  xmlStream << " password=\"" << KStringHandler::obscure(m_password.value()) << "\"";
  xmlStream << " local=\"" << (isVirtual()?"false":"true") << "\"";
}

bool Player::load (QDataStream &stream)
{
//   qCDebug(KSIRK_LOG) << "Player::load" << endl;
  if (!KPlayer::load(stream)) return false;
  QString nationName;
  stream >> nationName;
//   qCDebug(KSIRK_LOG) << "Player::load nationName=" << nationName << endl;
  setNation(nationName);
  stream >> m_goal;
  int nbToPlace;
  stream >> nbToPlace;
  int nbCountries;
  stream >> nbCountries;
  m_distributionData.init(nbToPlace, nbCountries);
  for (int i = 0; i < nbCountries; i++)
  {
    int nb;
    stream >> nb;
    m_distributionData[i] = nb;
  }
  return true;
}

bool Player::save (QDataStream &stream)
{
//   qCDebug(KSIRK_LOG) << "Player::save" << endl;
  if (!KPlayer::save(stream)) return false;
  stream << m_nation->name();
  stream << m_goal;
  stream << m_distributionData.nbToPlace();
  stream << m_distributionData.size();
  for (int i = 0; i < m_distributionData.size(); i++)
  {
    stream << m_distributionData[i];
  }
  return true;
}

Nationality* Player::getNation() 
{
  qCDebug(KSIRK_LOG) << "Player::getNation for " << name() << endl;
  if (m_nation == 0 && !m_delayedInitNationName.isEmpty())
  {
    qCCritical(KSIRK_LOG) << "  retrieving delayed nation " << m_delayedInitNationName << endl;
    setNation(m_delayedInitNationName);
  }
  return m_nation;
}

void Player::setNation(const QString& nationName)
{
  m_nation = m_automaton->game()->theWorld()-> nationNamed(nationName);
  if (m_nation == 0)
  {
//     qCDebug(KSIRK_LOG) << "Delaying nation initialization ("<<nationName<<") for " << name() << endl;
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
                            Sprites::SkinSpritesData::single().intData("flag-width"),
                            Sprites::SkinSpritesData::single().intData("flag-height"),
                            Sprites::SkinSpritesData::single().intData("flag-frames"), 
                            Sprites::SkinSpritesData::single().intData("flag-versions"),
                            m_automaton->game()->backGnd()->onu()->zoom(),
                            m_automaton->game()->backGnd());
    m_flag->hide();
  }
}

void Player::goal(const Goal& goal)
{
  qCDebug(KSIRK_LOG) << "Player::goal (setter) " << name() << endl;
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
QList<Country*> Player::countries() const
{
//   qCDebug(KSIRK_LOG) << name() << ": Player::countries()" << endl;
  QList<Country*> list;
  foreach (Country* c, m_automaton->game()->theWorld()->getCountries())
  {
    if (c-> owner() == static_cast< const Player * >(this))
    {
//            qCDebug(KSIRK_LOG) << "\t" << c-> name() << endl;
      
      list.push_back(c);
    }
  }
//    qCDebug(KSIRK_LOG) << name() << ": OUT AIPlayer::countries()" << endl;
  return list;
}

bool Player::acknowledge(const QString& ack)
{
  QMutexLocker locker(&m_waitedAckMutex);
  
  if (ack == m_waitedAck)
  {
    m_waitedAck = "";
    qCDebug(KSIRK_LOG) << ack << true;
    return true;
  }
  else
  {
    qCDebug(KSIRK_LOG) << ack << false;
    return false;
  }
}

QDataStream& operator<<(QDataStream& stream, PlayerMatrix& p)
{
  stream << p.name << quint32(p.nbAttack) << quint32(p.nbCountries) << quint32(p.nbAvailArmies)
    << quint32(p.nbDefense) << p.nation << quint32(p.isAI) << quint32(p.countries.size());
  foreach (const QString& s, p.countries)
  {
    stream << s;
  }
  stream << p.goal;
/*  stream << m_distributionData.nbToPlace();
  stream << m_distributionData.size();
  for (int i = 0; i < m_distributionData.size(); i++)
  {
    stream << m_distributionData[i];
  }*/
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
    p.countries.push_back(country);
  }
  stream >> p.goal;
/*  int nbToPlace;
  stream >> nbToPlace;
  int nbCountries;
  stream >> nbCountries;
  m_distributionData.init(nbToPlace, nbCountries);
  for (int i = 0; i < nbCountries; i++)
  {
    int nb;
    stream >> nb;
    m_distributionData[i] = nb;
  }*/
  return stream;
}

} // closing namespace GameLogic

} // closing namespace Ksirk


