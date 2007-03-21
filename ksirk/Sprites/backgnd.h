/***************************************************************************
                          backgnd.h  -  description
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001-2006 by Gaël de Chalendar (aka Kleag)
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define KDE_NO_COMPAT

#ifndef BACKGND_H
#define BACKGND_H

#include "decoratedgameframe.h"
#include <QGraphicsItem>
#include <QGraphicsScene>

namespace Ksirk
{
namespace GameLogic
{
  class ONU;
}
class DecoratedGameFrame;

/**
 * BackGnd is the sprite used to display the background map image
 */
class BackGnd : public QGraphicsPixmapItem
{
public:
  /**
    * Constructor.
    * @param scene The canvas where all the sprites are set up.
    * @param theWorld The world represented by this background 
    */
  BackGnd(QGraphicsScene *scene, const GameLogic::ONU* theWorld);

  /**
    * Default destructor
    */
  ~BackGnd();

protected:
  /**
    * Reimplemented of the inherited one to let the event be transmitted to the 
    * parent widget.
    * @param unused
    * @return false: the event is not handled here.
    */
  virtual bool sceneEvent ( QEvent * ) {return false;}

};
}
#endif //  BACKGND_H

