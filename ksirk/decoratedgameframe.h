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

/* begin                : Thu Jul 19 2001 */

#ifndef DECORATEDGAMEFRAME_H
#define DECORATEDGAMEFRAME_H

#include "KsirkGlobalDefinitions.h"

#include <stdlib.h>
#include <qtimer.h>
#include <QMouseEvent>
#include <QGraphicsView>

class KGameIO;

namespace Ksirk
{


/**
  * The DecoratedGameFrame class is the central widget of the application where
  * all the sprites are displayed. It is linked to its parent widget (the main 
  * window)
 */
class DecoratedGameFrame: public QGraphicsView
{
    Q_OBJECT
public:
  /**
    * Creates the frame, its timer and set some parameters
    * @param parent The parent widget, the main window
    * @param mapW The width of the map. Will be the width given in size hint.
    * @param mapH The height of the map. Will be the height given in size hint.
    */
  DecoratedGameFrame(QWidget* parent, unsigned int mapW, unsigned int mapH);

  /**
    * Destroy the frame : stops and deletes the timer
    */
  virtual ~DecoratedGameFrame();

  /**
    * Returns the size given to the constructor.
    * @return The size given to the constructor.
    */
  virtual QSize sizeHint() const;

public slots:
  /** 
    * Slot connected in the game/document object to catch and
    * process mouse events. This is a KGame function which
    * ends up generating the move for the game.
    * @param input is the KGameIO input object
    * @param stream is the output command stream
    * @param mouse is a mouse event
    * @param eatevent is to be set to true if you processed the event
    *
    */
  void slotMouseInput(KGameIO *input,QDataStream &stream,
                      QMouseEvent *mouse, bool *eatevent);
  
protected:

  void mouseMoveEvent ( QMouseEvent * event );

private:
  unsigned int m_mapW;
  unsigned int m_mapH;

};

}

#endif // DECORATEDGAMEFRAME_H

