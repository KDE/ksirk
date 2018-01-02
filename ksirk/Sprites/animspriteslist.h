/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gael de Chalendar <kleag@free.fr>

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

/*  begin                : Wed Jul 18 2001  */

#ifndef ANIMSPRITESLIST_H
#define ANIMSPRITESLIST_H

#include "animsprite.h"
#include "armysprite.h"

#include <QTextStream>
#include <QPoint>
#include <QList>
#include <QObject>

#include "ksirk_debug.h"

namespace Ksirk
{


/**
 * the AnimSpritesList is a list of AnimSprite-s with useful methods to
 * do some actions on each the elements. It is templatized to allow its
 * instanciation with the various kind of sprites
 */
template < typename SpriteType >
class AnimSpritesList : public QList< SpriteType* >
{
public:

  /** Default constructor */
  AnimSpritesList();

  /** Default destructor */
  virtual ~AnimSpritesList();

  
  /**
    * hide the sprites of the list, remove them from the list  and call their
    * destructors iff the liste is in auto-delete mode
    */
  void hideAndRemoveAll();

 /**
    * hide the sprites of the list, remove them from the list  and call their
    * destructors iff the liste is in auto-delete mode
    */
  void hideAndRemoveFirst();

  /**
    * return the first AnimSprite of the list that currently displays its last
    * frame
    */
  const SpriteType* firstThatIsLastFrame();

  /**
    * Makes all sprites of this list to move one step toward their destination
    */
  void moveAll();

  /**
    * Makes all sprites of this list to move up to their destination
    * @param clear if true removes the sprites from the list and add them to
    * their destination country
    */
  void moveAllToDestinationNow(bool clear = false);

  /**
    * Saves all elements of this list with a XML format
    * @param xmlStream the stream on which to write the XML
    */
  void saveXmlAll(QTextStream& xmlStream);
} ;

template < typename SpriteType >
AnimSpritesList< SpriteType >::AnimSpritesList() : QList< SpriteType* >()
{
}

template < typename SpriteType > 
  AnimSpritesList< SpriteType >::~AnimSpritesList()
{
}

template < typename SpriteType >
void AnimSpritesList< SpriteType >::hideAndRemoveAll()
{
//   qCDebug(KSIRK_LOG);

  while (!QList< SpriteType* >::empty())
  {
    hideAndRemoveFirst();
  }
}

template < typename SpriteType >
void AnimSpritesList< SpriteType >::hideAndRemoveFirst()
{   
    SpriteType* sprite = QList< SpriteType* >::front();
    QList< SpriteType* >::pop_front();
    sprite-> hide();
    sprite->deleteLater();
}

template < typename SpriteType >
const SpriteType* AnimSpritesList< SpriteType >::firstThatIsLastFrame()
{
//    qDebug("AnimSpritesList< SpriteType >::firstThatIsLastFrame");
  typename AnimSpritesList< SpriteType >::iterator it;
  typename AnimSpritesList< SpriteType >::iterator it_end = QList< SpriteType* >::end();
  for ( it = QList< SpriteType* >::begin(); it != it_end; it++ )
  {
    if ((*it)-> isLastFrame()) return (*it);
  }
  return 0;
}

template < typename SpriteType >
void AnimSpritesList< SpriteType >::moveAll()
{
  typename AnimSpritesList< SpriteType >::iterator it, it_end;
  it = QList< SpriteType* >::begin();
  it_end = QList< SpriteType* >::end();
  while (it != it_end)
  {
    SpriteType* sp = (*it);

    const QPointF& destinationPoint = sp-> getDestination()-> pointFor(sp);

    if (((sp->x()) == (destinationPoint.x())) && ((sp-> y()) == (destinationPoint.y())))
    {
      sp-> hide();
      it = QList< SpriteType* >::erase(it);
      sp-> getDestination()-> incrNbArmies((*it)-> nbArmies());
      sp-> getDestination()-> createArmiesSprites();
      sp->deleteLater();
    }
    else it++;
  }
}

template < typename SpriteType >
void AnimSpritesList< SpriteType >::moveAllToDestinationNow(bool clear)
{
  qCDebug(KSIRK_LOG) << clear;
  typename AnimSpritesList< SpriteType >::iterator it, it_end;
  it = QList< SpriteType* >::begin();
  it_end = QList< SpriteType* >::end();
  while (it != it_end)
  {
    SpriteType* sp = (*it);

    const QPointF& destinationPoint = sp-> getDestinationPoint();
    sp->setPos(destinationPoint);
    sp->moveIt();

    if (clear)
    {
      sp-> hide();
      it = QList< SpriteType* >::erase(it);
      sp-> getDestination()-> incrNbArmies(((ArmySprite*)(*it))-> nbArmies());
      sp-> getDestination()-> createArmiesSprites();
    }
    else
    {
      it++;
    }
  }
}

template < typename SpriteType >
void AnimSpritesList< SpriteType >::saveXmlAll(QTextStream& xmlStream)
{
  foreach (SpriteType* sp, *this)
  {
    sp->saveXml(xmlStream);
  }
}

} // closing namespace Ksirk

#endif // ANIMSPRITESLIST_H

