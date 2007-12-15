/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gael de Chalendar <kleag@free.fr>

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

/*  begin                : Wed Jul 18 2001  */

#ifndef ANIMSPRITESLIST_H
#define ANIMSPRITESLIST_H

#include "animsprite.h"
#include "armysprite.h"

#include <iostream>

#include <QPoint>
#include <QList>
#include <QObject>

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
  void saveXmlAll(std::ostream& xmlStream);
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
{    // cache tous les sprites du tableau, puis les enleve et les
    // detruit si le tableau n'etait pas en auto delete
//    qDebug("AnimSpritesList< SpriteType >::hideAndRemoveAll");

  while (!QList< SpriteType* >::empty())
  {
    SpriteType* sprite = QList< SpriteType* >::front();
    sprite-> hide();
    QList< SpriteType* >::pop_front();
    delete sprite;
  }
}

template < typename SpriteType >
void AnimSpritesList< SpriteType >::hideAndRemoveFirst()
{   
    SpriteType* sprite = QList< SpriteType* >::front();
    sprite-> hide();
    QList< SpriteType* >::pop_front();
    delete sprite;
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
      typename AnimSpritesList< SpriteType >::iterator itToRemove = it;
      sp-> getDestination()-> incrNbArmies((*it)-> nbArmies());
      sp-> hide();
      it++;
      QList< SpriteType* >::remove(itToRemove);
      sp-> getDestination()-> createArmiesSprites(sp-> getBackGnd());
    }
    else it++;
  }
}

template < typename SpriteType >
void AnimSpritesList< SpriteType >::moveAllToDestinationNow(bool clear)
{
  typename AnimSpritesList< SpriteType >::iterator it, it_end;
  it = QList< SpriteType* >::begin();
  it_end = QList< SpriteType* >::end();
  while (it != it_end)
  {
    SpriteType* sp = (*it);

    const QPointF& destinationPoint = sp-> getDestinationPoint();
    sp->setPos(destinationPoint);

    if (clear)
    {
      typename AnimSpritesList< SpriteType >::iterator itToRemove = it;
      sp-> getDestination()-> incrNbArmies(((ArmySprite*)(*it))-> nbArmies());
      sp-> hide();
      it++;
      QList< SpriteType* >::erase(itToRemove);
      sp-> getDestination()-> createArmiesSprites(sp-> getBackGnd());
    }
    else
    {
      it++;
    }
  }
}

template < typename SpriteType >
void AnimSpritesList< SpriteType >::saveXmlAll(std::ostream& xmlStream)
{
  typename AnimSpritesList< SpriteType >::iterator it, it_end;
  it = QList< SpriteType* >::begin();
  it_end = QList< SpriteType* >::end();
  while (it != it_end)
  {
    SpriteType* sp = (*it);
    sp-> saveXml(xmlStream);
    it++;
  }
}

} // closing namespace Ksirk

#endif // ANIMSPRITESLIST_H

