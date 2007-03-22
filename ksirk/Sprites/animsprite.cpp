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

#include "animsprite.h"
#include "backgnd.h"
#include "kgamewin.h"
#include "ksirksettings.h"
#include "GameLogic/nationality.h"
#include "GameLogic/country.h"
#include "GameLogic/gameautomaton.h"

#include <qpoint.h>
#include <qmessagebox.h>
#include <QPixmap>
#include <QSvgRenderer>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

namespace Ksirk
{

using namespace GameLogic;

AnimSprite::AnimSprite(const QString &imgPath, BackGnd* aBackGnd,
                        unsigned int nbFrames, unsigned int nbDirs, unsigned int visibility) :
    QGraphicsPixmapItem(0, aBackGnd-> scene()),
    m_animated(false), look(right), nbVersions(nbDirs),
    backGnd(aBackGnd), destination(0), destinationPoint(), frames(nbFrames), actFrame(0),
    myState(NONE), m_zoom(1.0),
    approachDestByLeft(false), approachDestByRight(false),
    approachDestByTop(false), approachDestByBottom(false),
    m_frames(),
    m_renderer(0),
    m_numberOfShots(0),
    m_timer(this)
{
  setNone();

  KStandardDirs *m_dirs = KGlobal::dirs();

  QString imgName = m_dirs-> findResource("appdata", GameLogic::GameAutomaton::single().skin() + "/Images/sprites/" + imgPath);
//   kDebug() << "Sprite file name: " << imgName << endl;
//   kDebug() << "    frames: " << frames << " ; nbVersions: " << nbVersions << endl;
  if (imgName.isNull())
  {
      QMessageBox::critical(0, i18n("Error !"), i18n("Sprite images resource not found\nProgram cannot continue"));
      exit(2);
  }
  m_renderer.load(imgName);

  spriteWidth = ((double)m_renderer.defaultSize().width()) / frames;
  spriteHeight = ((double)m_renderer.defaultSize().height()) / nbVersions;

  sequenceConstruction();
  setZValue(visibility);
  show();

  /// @note uncomment here if you want to try sprites animation by connection to a timer
//   connect(GameAutomaton::changeable().game()->frame()->timer(),SIGNAL(timeout()),this,SLOT(animate()));
  connect(&m_timer,SIGNAL(timeout()),this,SLOT(animate()));
//   m_timer.setSingleShot(true);
  if (frames > 1)
  {
    m_timer.start(200);
  }
}

AnimSprite::~AnimSprite()
{
  kDebug() << "AnimSprite::~AnimSprite " << (void*)this << endl;
  m_timer.stop();
  disconnect(&m_timer,SIGNAL(timeout()),this,SLOT(animate()));
  setStatic();
}

void AnimSprite::repaint()
{
  update();
}

void AnimSprite::setLook(TDir newLook)
{
  if (newLook != look)
  {
//        kDebug()<<"setLook : " << newLook << ")" << endl;
    look=newLook;
    setFrame(0);
    update();
  }
}

/**
 * updates the sequence of images used by the underlying QCanvasSprite with
 * the ones taken from the image found at imgPath. It is function of the
 * direction of the look and the geometry of the sprite
 */
void AnimSprite::sequenceConstruction()
{
  QList<QPixmap> list;

  QSize size((int)(spriteWidth*frames*m_zoom), (int)(spriteHeight*nbVersions*m_zoom));
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  image.fill(0);
  QPainter p(&image);
  m_renderer.render(&p/*, svgid*/);
  QPixmap allpm = QPixmap::fromImage(image);
  for (unsigned int l = 0; l<nbVersions;l++)
  {
    for (unsigned int i = 0; i<frames;i++)
    {
//       kDebug()<< "constr s : "<<spriteWidth<<" "<<spriteHeight<<" "<<look-1<<endl;
      QPixmap pm =
        allpm.copy(
            (int)(spriteWidth*i*m_zoom), (int)(spriteHeight*l*m_zoom),
            (int)(spriteWidth*m_zoom), (int)(spriteHeight*m_zoom));
      list.push_back(pm);
    }
  }
  m_frames = list;

  setFrame(0);
}

void AnimSprite::changeSequence(const QString &imgPath, unsigned int newNbFrames, unsigned int nbDirs)
{
//    kDebug()<<"AnimSprite::changeSequence" <<endl;
  frames = newNbFrames;
  actFrame = 0;
  nbVersions = nbDirs;

  KStandardDirs *m_dirs = KGlobal::dirs();

//   kDebug() << "Sprite file name: " << GameLogic::GameAutomaton::single().skin() + "/Images/sprites/" + imgPath << endl;
  QString imgName = m_dirs-> findResource("appdata", GameLogic::GameAutomaton::single().skin() + "/Images/sprites/" + imgPath);

//   kDebug() << "imgName= " << imgName << endl;
    
//    QString imgName = m_dirs-> findResource("appdata", "Images/sprites/" + imgPath);
  if (imgName.isNull())
  {
      QMessageBox::critical(0, i18n("Error !"), i18n("Sprite images resource not found\nProgram cannot continue"));
      exit(2);
  }

  m_renderer.load(imgName);

  spriteHeight = m_renderer.defaultSize().height() / nbVersions;
  spriteWidth = m_renderer.defaultSize().width() / frames;

  sequenceConstruction();
  update();
//    kDebug()<<"OUT AnimSprite::changeSequence"<<endl;
}

void AnimSprite::nextFrame()
{
  if (frames <= 1)
  {
    return;
  }
  actFrame++;    // next image

  if (actFrame == (frames-1))
  {
    if (m_numberOfShots == 1)
    {
      setStatic();
      m_numberOfShots = std::numeric_limits<unsigned int>::max();
      kDebug() << "Emiting animationFinished" << endl;
      emit animationFinished(this);
      return;
    }
    else if (m_numberOfShots != std::numeric_limits<unsigned int>::max())
    {
      m_numberOfShots--;
      kDebug() << "numberOfShots is nom " << m_numberOfShots << endl;
    }
  }
  else if (actFrame > (frames-1))
  {
    actFrame=0; // come back to start
  }
  setFrame(actFrame);
}

void AnimSprite::setFrame(unsigned int numFrame)
{
// kDebug() << "AnimSprite::setFrame("<<numFrame<<") look="<<look<<" ; frames="<<frames<<" ; m_frames size="<<m_frames.size()<<endl;
  if (numFrame < m_frames.size())
  {
    setPixmap(m_frames[(look-1)*frames+numFrame]);
  }
}

void AnimSprite::moveIt()
{
/*  kDebug() << "moveIt: Position of " << (void*)this << " is: " 
    << pos() << " (destination point is: " << destinationPoint << ")" << endl;*/
  qreal delta = 5;
  switch (KsirkSettings::spritesSpeed())
  {
    case 0:
      delta = 2;
      break;
    case 1:
      delta = 5;
      break;
    case 2:
      delta = 10;
      break;
    case 3:
      setPos(destinationPoint);
      destinationPoint = QPointF();
      emit atDestination(this);
      return;
      break;
    default:
      delta = 5;
  }
//   kDebug() << "delta="<<delta<<endl;
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
//   kDebug() << "After x, position is: " << pos() << endl;
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
//   kDebug() << "New position of " << (void*)this << " is: " << pos() << endl;
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
//    kDebug()<<"AnimSprite::isLastFrame actFrame = "<<actFrame<<" frames = "<<frames-1<<endl;
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

void AnimSprite::setDestination(Country::Country* country)
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
    setLook(left);
}

/** tourne le sprite vers la droite */
void AnimSprite::setLookRight()
{
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
//    kDebug()<<"AnimSprite::setApproachDestByRight"<<endl;
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
//    kDebug()<<"AnimSprite::setApproachDestByLeft"<<endl;
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
        QMessageBox::critical(0, I18N_NOOP("Error !"), I18N_NOOP("Cannot find Max X  for sprite: no background !"));
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
        QMessageBox::critical(0, I18N_NOOP("Error !"), I18N_NOOP("Cannot find Max Y  for sprite: no background !"));
        exit(2);
    }
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
   kDebug() << "AnimSprite::setupTravel (" << srcPoint << ") (" << destPoint << ")" << endl;

  setDestination(dest);
  setDestinationPoint(destPoint);
  setPos(srcPoint);

  if (!src-> communicateWith(dest))
  {
      kError() << "Error in AnimSprite::setupTravel: " << src-> name() << "  and " 
              << dest-> name() << " do not communicate!\n";
      exit(2);
  }
  
  if ( (qAbs(srcPoint.x() - destPoint.x())) > ((backGnd-> boundingRect().width())/2) ) 
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

  if ( ((qAbs(srcPoint.y() - destPoint.y())) > ((backGnd-> boundingRect().height())/2)) )
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
  (src->pointFlag().x() < dest-> pointFlag().x()) ? setLookRight() : setLookLeft();
  setAnimated();
}

void AnimSprite::arrival()
{
  kDebug()<< "AnimSprite::arrival" << endl;
  if (x() < getDestination()-> pointFlag().x())
        setLookRight();
  else setLookLeft();
  repaint();
}

void AnimSprite::setupTravel(Country* src, Country* dest, const QPointF* dpi)
{
    if (dpi ==0) AnimSprite::setupTravel(src, dest, src->centralPoint(), dest-> centralPoint());
    else AnimSprite::setupTravel(src, dest, src->centralPoint(), *dpi);
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
//    kDebug() << "I'm a sprite; my state is : " << myState << endl;
    return myState;
}

/** sets the new state of the game */
void AnimSprite::setState(AnimSprite::State newState)
{
//    kDebug() << "Setting sprite's state to : " << newState << " (was : " << myState << ")" << endl;
    myState = newState;
}

void AnimSprite::saveXml(std::ostream& /*xmlStream*/)
{
}

QPixmap AnimSprite::image(unsigned int numFrame) const
{
  kDebug() << "image(" << numFrame << ") / " << m_frames.size() << endl;
  if (numFrame >= m_frames.size())
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
//   kDebug() << "AnimSprite::animate " << (void*)this << endl;
  if (!destinationPoint.isNull() && pos() != destinationPoint)
  {
    moveIt();
  }
  else if (m_animated && frames > 1)
  {
    nextFrame();
  }
//   kDebug() << "AnimSprite::animate finished for " << (void*)this << endl;
  
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


} // closing namespace Ksirk

#include "animsprite.moc"
