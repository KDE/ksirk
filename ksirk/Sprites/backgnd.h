/***************************************************************************
                          backgnd.h  -  description
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001-2006 by Gael de Chalendar (aka Kleag)
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/
#define KDE_NO_COMPAT

#ifndef BACKGND_H
#define BACKGND_H

#include "decoratedgameframe.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <KConfig>
#include <KConfigGroup>

namespace Ksirk
{
namespace GameLogic
{
  class ONU;
}

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
  BackGnd(QGraphicsScene *scene, const GameLogic::ONU* theWorld, bool arena = false);

  /**
    * Default destructor
    */
  ~BackGnd() override;

  inline const GameLogic::ONU* onu() const {return m_theWorld;}

  inline bool bgIsArena() const {return m_bgIsArena;}
  
protected:
  /**
    * Reimplemented of the inherited one to let the event be transmitted to the 
    * parent widget.
    * @param unused
    * @return false: the event is not handled here.
    */
  bool sceneEvent ( QEvent * ) override {return false;}

private:
  const GameLogic::ONU* m_theWorld;
  bool m_bgIsArena;
};
}
#endif //  BACKGND_H

