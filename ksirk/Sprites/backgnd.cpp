/***************************************************************************
                          backgnd.cpp  -  description
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001-2006 by Gael de Chalendar (aka Kleag)
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/
 
#define KDE_NO_COMPAT
#include "backgnd.h"
#include "GameLogic/onu.h"

#include <QPixmap>

#include <kdebug.h>

namespace Ksirk
{

using namespace GameLogic;

BackGnd::BackGnd(QGraphicsScene *scene, const GameLogic::ONU* theWorld, bool arena) :
    QGraphicsPixmapItem(0, scene), m_theWorld(theWorld)
{
  kDebug() << "BackGnd constructor" << endl;

  QPixmap pix;
  if (arena == false) {
    pix = theWorld->map();
  } else {
    // search the background image for the arena
    KConfig config(m_theWorld->getConfigFileName());
    KConfigGroup onugroup = config.group("onu");
    QString skin = onugroup.readEntry("skinpath");
    QString imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/arena.svg");
    // create the background image
    pix.load(imageFileName);
    pix = pix.scaled(m_theWorld->width(),m_theWorld->height(),Qt::KeepAspectRatio);
  }
  // and finaly put the image
  setPixmap(pix);

  setZValue(1);
  show();
}

BackGnd::~BackGnd()
{
//   m_frame = 0;
}



}
