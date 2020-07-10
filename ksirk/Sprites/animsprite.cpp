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

#include "animsprite.h"
#include "skinSpritesData.h"
#include "backgnd.h"
#include "kgamewin.h"
#include "ksirksettings.h"
#include "GameLogic/nationality.h"
#include "GameLogic/country.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/onu.h"

#include <QPoint>
#include <QPixmap>
#include <QPixmapCache>
#include <QSvgRenderer>

#include <KMessageBox>
#include <KLocalizedString>
#include "ksirk_debug.h"

namespace Ksirk
{

using namespace GameLogic;

AnimSprite::AnimSprite(const QString &svgid,
                        unsigned int width,
                        unsigned int height,
                        unsigned int nbFrames, unsigned int nbDirs,
                        double zoom,
                        BackGnd* aBackGnd,
                        unsigned int visibility) :
    QGraphicsPixmapItem(0),
    m_animated(false), m_zoom(zoom), m_svgid(svgid),
    look(right), nbVersions(nbDirs),
    backGnd(aBackGnd), destination(0), destinationPoint(), frames(nbFrames), actFrame(0),
    myState(NONE),
    m_height(zoom*height),
    m_width(zoom*width),
    approachDestByLeft(false), approachDestByRight(false),
    approachDestByTop(false), approachDestByBottom(false),
    m_frames(),
    m_renderer(const_cast<ONU*>(aBackGnd->onu())->renderer()),
    m_numberOfShots(0),
    m_timer(this),
    m_skin(backGnd->onu()->skin())
{
   aBackGnd-> scene()->addItem(this);
//   qCDebug(KSIRK_LOG) << svgid << nbFrames;
  setNone();

  sequenceConstruction();
  setZValue(visibility);
  show();

  /// @note uncomment here if you want to try sprites animation by connection to a timer
//   connect(GameAutomaton::changeable().game()->frame()->timer(),SIGNAL(timeout()),this,SLOT(animate()));
  connect(&m_timer,&QTimer::timeout,this,&AnimSprite::animate);
//   m_timer.setSingleShot(true);
  if (frames > 1)
  {
    m_timer.start(200);
  }
}

AnimSprite::~AnimSprite()
{
//   qCDebug(KSIRK_LOG) << (void*)this ;
  m_timer.stop();
  disconnect(&m_timer,&QTimer::timeout,this,&AnimSprite::animate);
  setStatic();
}

void AnimSprite::repaint()
{
  update();
}

void AnimSprite::setLook(TDir newLook)
{
//   qCDebug(KSIRK_LOG);
  if (newLook != look)
  {
//        qCDebug(KSIRK_LOG)<<"setLook : " << newLook << ")";
    look=newLook;
    setFrame(0);
    update();
  }
}

/**
 * updates the sequence of images used by the underlying QGraphicsPixmapItem 
 * with the ones taken from the image found at imgPath. It is function of the
 * direction of the look and the geometry of the sprite
 */
void AnimSprite::sequenceConstruction()
{
  QList<QPixmap> list;

  QPixmap allpm;
  QString allpmCacheId = m_skin+m_svgid+QString::number(m_width*frames)+"x"+QString::number(m_height*nbVersions);
  if (!QPixmapCache::find(allpmCacheId, &allpm))
  {
    // Pixmap isn't in the cache, create it and insert to cache
    QSize size((int)(m_width*frames), (int)(m_height*nbVersions));
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter p(&image);
    m_renderer->render(&p, m_svgid);
    allpm = QPixmap::fromImage(image);

    QPixmapCache::insert(allpmCacheId, allpm);
  }
  
  for (unsigned int l = 0; l<nbVersions;l++)
  {
    for (unsigned int i = 0; i<frames;i++)
    {
//       qCDebug(KSIRK_LOG)<< "constr s : "<<m_width<<" "<<m_height<<" "<<look-1;
      QPixmap pm;
      QString pmCacheId = m_skin+m_svgid+QString::number(m_width*frames)+"x"+QString::number(m_height*nbVersions)+"-"+QString::number(i)+":"+QString::number(l);
      if (!QPixmapCache::find(pmCacheId, &pm))
      {
        // Pixmap isn't in the cache, create it and insert to cache
        pm = allpm.copy((int)(m_width*i), (int)(m_height*l),
                   (int)(m_width), (int)(m_height));
                   
        QPixmapCache::insert(pmCacheId, pm);
      }

      list.push_back(pm);
    }
  }
  m_frames = list;

  setFrame(0);
}

void AnimSprite::changeSequence(const QString &id)
{
  qCDebug(KSIRK_LOG) << (void*)this << id;
  changeSequence(
          id,
          Sprites::SkinSpritesData::single().intData(id+"-width"),
          Sprites::SkinSpritesData::single().intData(id+"-height"),
          Sprites::SkinSpritesData::single().intData(id+"-frames"),
          Sprites::SkinSpritesData::single().intData(id+"-versions"));
}
void AnimSprite::changeSequence(const QString &svgid,
                                 unsigned int width,
                                 unsigned int height,
                                 unsigned int newNbFrames,
                                 unsigned int nbDirs)
{
  qCDebug(KSIRK_LOG) << (void*)this << svgid;
  m_svgid = svgid;
  m_width = width*m_zoom;
  m_height = height*m_zoom;
  frames = newNbFrames;
  actFrame = 0;
  nbVersions = nbDirs;

  sequenceConstruction();
  update();
  //    qCDebug(KSIRK_LOG)<<"OUT";
}

void AnimSprite::nextFrame()
{
  if (frames <= 1)
  {
    return;
  }
  actFrame++;    // next image

  if (actFrame > (frames-1))
  {
    actFrame=0; // come back to start
    if (m_numberOfShots == 1)
    {
      setStatic();
      m_numberOfShots = std::numeric_limits<unsigned int>::max();
      qCDebug(KSIRK_LOG) << "Emiting animationFinished";
      emit animationFinished(this);
    }
    else if (m_numberOfShots != std::numeric_limits<unsigned int>::max())
    {
      m_numberOfShots--;
      qCDebug(KSIRK_LOG) << "numberOfShots is now " << m_numberOfShots ;
    }
  }
  setFrame(actFrame);
}

void AnimSprite::setFrame(unsigned int numFrame)
{
//   qCDebug(KSIRK_LOG) << " " << numFrame << " look=" << look <<" ; frames="
//           <<frames<<" ; m_frames size="<<m_frames.size();
  if (numFrame < (unsigned int)m_frames.size())
  {
    setPixmap(m_frames[(look-1)*frames+numFrame]);
  }
}

void AnimSprite::moveIt()
{
//   qCDebug(KSIRK_LOG) << "Position of " << (void*)this << " is: "
//     << pos() << " (destination point is: " << destinationPoint << ")";
  qreal delta = 5*m_zoom;
  switch (KsirkSettings::spritesSpeed())
  {
    case 0:
      delta = 2*m_zoom;
      break;
    case 1:
      delta = 5*m_zoom;
      break;
    case 2:
      delta = 10*m_zoom;
      break;
    case 3:
      setPos(destinationPoint);
      destinationPoint = QPointF();
      emit atDestination(this);
      return;
      break;
    default:
      delta = 5*m_zoom;
  }
  delta=delta<1.0?1.0:delta;
  
//   qCDebug(KSIRK_LOG) << "delta="<<delta;
  if (getApproachDestByLeft())
  {
    setLook(right);
    if (x() < destinationPoint.x())
    {
      if (destinationPoint.x() - x() > delta) setPos(pos() + QPointF(delta,0)) ;
      else if (destinationPoint.x() - x() <= 1) setPos(destinationPoint.x(),y());
      else /*if (destinationPoint.x() - x() <= delta)*/ setPos(pos() + QPointF(1,0));
    }
    if (x() > destinationPoint.x())
    {
      if (getMaxX() - x() > delta) setPos(pos()+QPointF(delta,0));
      if (getMaxX() - x() <= delta) setPos(QPointF(destinationPoint.x()<0?destinationPoint.x():0,y()));
    }
  }
  else if (getApproachDestByRight())
  {
    setLook(left);
    if (x() < destinationPoint.x())
    {
      if (x() > delta) setPos(pos()+QPointF(-delta,0));
      if (x() <= delta) setPos(QPointF(getMaxX(),y()));
    }
    if (x() > destinationPoint.x())
    {
      if ( x() - destinationPoint.x() > delta) setPos(pos()+QPointF(-delta,0));
      else if ( x() - destinationPoint.x() <= 1) setPos(destinationPoint.x(),y());
      else /*if ( x() - destinationPoint.x() <= delta)*/ setPos(pos()+QPointF(-1,0));
    }
  }
  else
  {
    if (x() < destinationPoint.x())
    {
      setLook(right);
      if (destinationPoint.x() - x() > delta) setPos(pos()+QPointF(delta,0));
      if (destinationPoint.x() - x() <= 1) setPos(destinationPoint.x(),y());
      else /*if (destinationPoint.x() - x() <= delta)*/ setPos(pos()+QPointF(1,0));
    }
    if (x() > destinationPoint.x())
    {
      setLook(left);
      if (x() - destinationPoint.x() > delta) setPos(pos()+QPointF(-delta,0));
      else if (x() - destinationPoint.x() <= 1) setPos(destinationPoint.x(),y());
      else /*if (x() - destinationPoint.x() <= delta)*/ setPos(pos()+QPointF(-1,0));
    }
  }
//   qCDebug(KSIRK_LOG) << "After x, position is: " << pos();
  if (getApproachDestByTop())
  {
    if (y() < destinationPoint.y())
    {
      if (destinationPoint.y() - y() > delta) setPos(pos()+QPointF(0,delta));
      else if (destinationPoint.y() - y() <= delta) setPos(x(),destinationPoint.y());
      else /*if (destinationPoint.y() - y() <= delta)*/ setPos(pos()+QPointF(0,1));
    }
    if (y() > destinationPoint.y())
    {
      if (getMaxY() - y() > delta) setPos(pos()+QPointF(0,delta));
      if (getMaxY() - y() <= delta) setPos(x(),destinationPoint.y()<0?destinationPoint.y():0);
    }
  }
  else if (getApproachDestByBottom())
  {
    if (y() < destinationPoint.y())
    {
      if (destinationPoint.y() - y() > delta) setPos(pos()+QPointF(0,-delta));
      if (destinationPoint.y() - y() <= delta) setPos(x(), getMaxY() );
    }
    if (y() > destinationPoint.y())
    {
      if ( y() - destinationPoint.y() > delta) setPos(pos()+QPointF(0,-delta));
      else if ( y() - destinationPoint.y() <= 1) setPos(x(),destinationPoint.y());
      else /*if ( y() - destinationPoint.y() <= delta)*/ setPos(pos()+QPointF(0,-1));
    }
  }
  else
  {
    if (y() < destinationPoint.y())
    {
      if (destinationPoint.y() - y() > delta) setPos(pos()+QPointF(0,delta));
      else if (destinationPoint.y() - y() <= 1) setPos(x(),destinationPoint.y());
      else /*if (destinationPoint.y() - y() <= delta)*/ setPos(pos()+QPointF(0,1));
    }
    if (y() > destinationPoint.y())
    {
      if (y() - destinationPoint.y() > delta) setPos(pos()+QPointF(0,-delta));
      else if (y() - destinationPoint.y() <= 1) setPos(x(), destinationPoint.y());
      else /*if (y() - destinationPoint.y() <= delta)*/ setPos(pos()+QPointF(0,-1));
    }
  }
//   qCDebug(KSIRK_LOG) << "New position of " << (void*)this << " is: " << pos();
  nextFrame();
  if (pos() == destinationPoint)
  {
    setStatic();
    destinationPoint = QPointF();
    emit atDestination(this);
  }
}

bool AnimSprite::isLastFrame() const
{
//    qCDebug(KSIRK_LOG)<<"AnimSprite::isLastFrame actFrame = "<<actFrame<<" frames = "<<frames-1;
    return (actFrame == (frames - 1));
}

void AnimSprite::setDestinationPoint(const QPointF &point)
{
    
    destinationPoint = point;
}

const QPointF& AnimSprite::getDestinationPoint() const
{
    return  destinationPoint;
}

int AnimSprite::operator==(const AnimSprite& Arg) const 
{
    return (memcmp(this,&Arg,sizeof(AnimSprite)));
}

void AnimSprite::setDestination(Country* country)
{
    destination = country;
}

Country* AnimSprite::getDestination() 
{
    return destination;
}

void AnimSprite::turnTowardDestination()
{
    if (x() <= destinationPoint.x())
        setLook(right);
    else setLook(left);
}

bool AnimSprite::isAttacker() const
{
    return (isMyState( ATTACKER ));
}

void AnimSprite::setAttacker()
{
    setState ( ATTACKER );
}

bool AnimSprite::isDefendant() const
{
    return (isMyState(DEFENDANT));
}

void AnimSprite::setDefendant()
{
    setState (DEFENDANT);
}

bool AnimSprite::isNone() const
{
    return (isMyState(NONE));
}

void AnimSprite::setNone()
{
    setState (NONE );
}

/** turn the sprite towards left */
void AnimSprite::setLookLeft()
{
  qCDebug(KSIRK_LOG);
  setLook(left);
}

/** tourne le sprite vers la droite */
void AnimSprite::setLookRight()
{
  qCDebug(KSIRK_LOG);
  setLook(right);
}
/** No descriptions */
bool AnimSprite::looksToLeft() const
{
    return (look == left);
}
/** No descriptions */
bool AnimSprite::looksToRight() const
{
    return (look == right);
}

/** Read property of bool approachDestByRight. */
bool AnimSprite::getApproachDestByRight() const
{
    return approachDestByRight;
}

/** Write property of bool approachDestByRight. */
void AnimSprite::setApproachDestByRight( const bool& _newVal)
{
  //    qCDebug(KSIRK_LOG);
    approachDestByRight = _newVal;
    if (_newVal) setApproachDestByLeft(false);
}

/** Read property of bool approachDestByLeft . */
bool AnimSprite::getApproachDestByLeft () const
{
    return approachDestByLeft ;
}

/** Write property of bool approachDestByLeft . */
void AnimSprite::setApproachDestByLeft ( const bool& _newVal)
{
  //    qCDebug(KSIRK_LOG);
    approachDestByLeft  = _newVal;
    if (_newVal) setApproachDestByRight(false);
}

/** Read property of bool approachDestByTop. */
bool AnimSprite::getApproachDestByTop() const
{
    return approachDestByTop;
}

/** Write property of bool approachDestByTop. */
void AnimSprite::setApproachDestByTop( const bool& _newVal)
{
    approachDestByTop = _newVal;
    if (_newVal) setApproachDestByBottom(false);
}

/** Read property of bool approachDestByBottom . */
bool AnimSprite::getApproachDestByBottom () const
{
    return approachDestByBottom ;
}

/** Write property of bool approachDestByBottom . */
void AnimSprite::setApproachDestByBottom ( const bool& _newVal)
{
    approachDestByBottom  = _newVal;
    if (_newVal) setApproachDestByTop(false);
}

/** Return the maximum value for x for this sprite by looking to its including
  * background. Necessary for directed approaches.
  * Quit with error if there is no background
  */
qreal AnimSprite::getMaxX() const
{
    if (backGnd)
        return backGnd-> pixmap().width();
    else
    {
        KMessageBox::error(0, i18n("Cannot find Max X for sprite: no background!"), i18n("Error!"));
        exit(2);
    }
}

/** Return the maximum value for y for this sprite by looking to its including
  * background. Necessary for directed approaches.
  * Quit with error if there is no background
  */
qreal AnimSprite::getMaxY() const
{
    if (backGnd)
        return backGnd-> pixmap().height();
    else
    {
        KMessageBox::error(0, i18n("Cannot find Max Y for sprite: no background!"), i18n("Error!"));
        exit(2);
    }
}

void AnimSprite::setupTravel(Country* src, Country* dest, const QPointF* dpi)
{
  qCDebug(KSIRK_LOG) << src->name() << dest->name() << *dpi << (dpi==0?QPointF():*dpi);
  if (dpi ==0) AnimSprite::setupTravel(src, dest, src->centralPoint(), dest-> centralPoint());
  else AnimSprite::setupTravel(src, dest, src->centralPoint(), *dpi);
}

/**
  * This function chooses the approach mode of a sprite towards its destination:
  * if the distance between the origin and the destination is higher than half
  * the size of the map and if the origin and destination countries comunicate,
  * then the sprite should choose an approach by left or right, through the
  * edge of the map.
  * This protected method will be called by three public functions specialized
  * using as source point, respectivly, the infantryman point, the cavalryman
  * point and the cannon point.
  */
void AnimSprite::setupTravel(
        Country* src, 
        Country* dest, 
        const QPointF& srcPoint, 
        const QPointF& destPoint)
{
   qCDebug(KSIRK_LOG) << src->name() << srcPoint << ", " << dest->name() << destPoint ;

  setDestination(dest);
  setDestinationPoint(destPoint);
  setPos(srcPoint);

  if (!src-> communicateWith(dest))
  {
      qCCritical(KSIRK_LOG) << "Error in AnimSprite::setupTravel: " << src-> name() << "  and " 
              << dest-> name() << " do not communicate!\n";
      exit(2);
  }
  
  if ( (qAbs(srcPoint.x() - destPoint.x())) > ((backGnd-> boundingRect().width())/2) && !backGnd->bgIsArena()) 
  {
      // src is at the right of dest, approch dest by left
      if (srcPoint.x() > destPoint.x()) setApproachDestByLeft(true);
      // src is at the left of dest, approch dest by right
      if (srcPoint.x() < destPoint.x()) setApproachDestByRight(true);
  }
  else
  {
      // src is at the right of dest, approch dest by left
      if (srcPoint.x() > destPoint.x()) setApproachDestByRight(true);
      // src is at the left of dest, approch dest by right
      if (srcPoint.x() < destPoint.x()) setApproachDestByLeft(true);
  }

  if ( ((qAbs(srcPoint.y() - destPoint.y())) > ((backGnd-> boundingRect().height())/2)) && !backGnd->bgIsArena())
  {
      // src is under the dest, approch dest by top
      if (srcPoint.y() > destPoint.y()) setApproachDestByTop(true);
      // src is up to the dest, approch dest by botto
      if (srcPoint. y() < destPoint.y()) setApproachDestByBottom(true);
  }
  else
  {
      // src is under the dest, approch dest by bottom
      if (srcPoint.y() > destPoint.y()) setApproachDestByBottom(true);
      // src is up to the dest, approch dest by top
      if (srcPoint. y() < destPoint.y()) setApproachDestByTop(true);
  }
  if (src->pointFlag().x() < dest-> pointFlag().x()) {
    setLookRight();
  } else {
    setLookLeft();
  }
  setAnimated();
  qCDebug(KSIRK_LOG) << "Done";
}

void AnimSprite::arrival()
{
  qCDebug(KSIRK_LOG)<< "at " << destinationPoint ;
  if (!backGnd->bgIsArena())
  {
    qCDebug(KSIRK_LOG)<< "x=" << x() << "pf=" << getDestination()->pointFlag().x();
    if (x() < getDestination()-> pointFlag().x())
      setLookRight();
    else 
      setLookLeft();
  }
  repaint();
}

/** Return true if the state of the sprite is the argument; false otherwise */
bool AnimSprite::isMyState(State state) const
{
    return myState == state;
}

/**
  * returns the current state of the sprite
  */
AnimSprite::State AnimSprite::getState() const
{
//    qCDebug(KSIRK_LOG) << "I'm a sprite; my state is : " << myState ;
    return myState;
}

/** sets the new state of the game */
void AnimSprite::setState(AnimSprite::State newState)
{
//    qCDebug(KSIRK_LOG) << "Setting sprite's state to : " << newState << " (was : " << myState << ")";
    myState = newState;
}

void AnimSprite::saveXml(QTextStream& /*xmlStream*/)
{
}

QPixmap AnimSprite::image(unsigned int numFrame) const
{
//   qCDebug(KSIRK_LOG) << "image(" << numFrame << ") / " << m_frames.size();
  if (numFrame >= (unsigned int)m_frames.size())
  {
    return QPixmap();
  }
  else
  {
    return m_frames[(look-1)*frames+numFrame];
  }
}

void AnimSprite::animate()
{
  //   qCDebug(KSIRK_LOG) << (void*)this ;
  if (!destinationPoint.isNull() && pos() != destinationPoint)
  {
    moveIt();
  }
  else if (m_animated && frames > 1)
  {
    nextFrame();
  }
  //   qCDebug(KSIRK_LOG) <<"finished for " << (void*)this ;
}

void AnimSprite::setAnimated(unsigned int numberOfShots)
{
  m_numberOfShots = numberOfShots;
  m_animated = true;
  if (!m_timer.isActive())
  {
    m_timer.start(200);
  }
//   AnimSpritePool::changeable().addSprite(this);
}

void AnimSprite::setStatic()
{
  m_animated = false;
  m_timer.stop();
//   AnimSpritePool::changeable().removeSprite(this);
}

void AnimSprite::applyZoomFactor(qreal zoomFactor)
{
  qCDebug(KSIRK_LOG) << "old zoom=" << m_zoom ;
  m_zoom *= zoomFactor;
  m_width *= m_zoom;
  m_height *= m_zoom;
  
  sequenceConstruction();
  update();
}

void AnimSprite::addDecoration(const QString& svgid, const QRectF& geometry)
{
  QSize size(geometry.size().toSize());
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  image.fill(0);
  QPainter p(&image);
  m_renderer->render(&p, svgid);
  QPixmap pm = QPixmap::fromImage(image);
  QGraphicsPixmapItem* item = new QGraphicsPixmapItem(pm,this);
  item->setPos(geometry.topLeft());
  item->show();
}

} // closing namespace Ksirk


