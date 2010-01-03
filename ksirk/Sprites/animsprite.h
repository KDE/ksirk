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


#ifndef ANIMSPRITE_H
#define ANIMSPRITE_H

#include "KsirkGlobalDefinitions.h"
#include "animspritespool.h"

#include <QGraphicsItem>
#include <QTimer>

#include <limits>
#include <QTextStream>

class QSvgRenderer;

namespace Ksirk
{

class BackGnd;
namespace GameLogic
{
  class Country;
}

/**
 * The AnimSprite objects are the animated images used for all individual
 * moving objects in KsirK. It is based on QGraphicsPixmapItem that handle
 * all sprite specific behavior (sequence of images...). The individual
 * images are taken from an SVG image that stores all the versions of the sprite.
 * A sprite also have a direction (that allows to select the images looking
 * toward left or right) and a destination point allowing to handle sprite
 * movement.
 *
 */
class AnimSprite : public QObject, public QGraphicsPixmapItem
{
  Q_OBJECT
public:
  /**
  * When used for fight display, the sprites can be attacker or
  * defendant. In the other cases, they are nothing particular (NONE)
  * This state is only used for explosion animation
  */
  enum State {NONE, ATTACKER, DEFENDANT};

  enum TDir {state, right, left, up, down, N, S, E, O, NO, SO, SE, NE};

  /**
    * This constructor allows to create a new @ref AnimSprite whose images are
    * taken from the given file name with the given number of frames and
    * number of look directions
    * @param svgid The id of the SVG element from which to load images
    * @param aBackGnd The background giving info about the world geometry and
    * access to the underlying QGraphicsScene
    * @param nbFrames The number of different frames in this sprite animation, 
    * thus the number of columns in the sprite image
    * @param nbDirs The number of different views on the sprite, 
    * thus the number of rows in the sprite image
    * @param visibility Measures how much this sprite is visible. It gives its
    * Z value on the graphics scene.
    */
  AnimSprite(const QString &svgid,
              unsigned int width,
              unsigned int height,
              unsigned int nbFrames, unsigned int nbDirs,
              double zoom,
              BackGnd* aBackGnd,
              unsigned int visibility = 100);

  /** The default destructor */
  virtual ~AnimSprite();

  /**
    * Moves the sprite by one step towards its destinationPoint.
    */
  void moveIt();

  /**
    * Hides and shows the sprite. Causes it to be repainted.
    */
  void repaint();

  /**
    * Return true if the current frame is the last one. False otherwise.
    */
  bool isLastFrame() const;

  //@{
  /**
    * Accessors to some variables
    */
  void setDestination(GameLogic::Country *);
  GameLogic::Country *getDestination();
  void setDestinationPoint(const QPointF &);
  const QPointF& getDestinationPoint() const;
  bool isAttacker() const;
  void setAttacker();
  bool isDefendant() const;
  void setDefendant();
  bool isNone() const;
  void setNone();
  inline BackGnd* getBackGnd() {return backGnd;}
  //@}

  /**
    * Bit to bit comparison
    */
  int operator==(const AnimSprite& Arg1) const;

  /**
    * executes setLook towards left or right according to the relative
    * abscissa of the current position point and the destination point
    */
  void turnTowardDestination();

  /**
    * Simplified changing of the images sequence of the sprite. Use default
    * values built from the given id
    * @param id The base id from which skin data is accessed
    */
  void changeSequence(const QString &id);

  /**
    * Change the images sequence of the sprite.
    * @param imgPath The path to the SVG file containing the sprite's new images
    * @param newNbFrames The number of frames of the sprite in the new image
    * @param nbDirs The number of directions of the sprite in the new image
    */
  void changeSequence(const QString &imgPath,
                       unsigned int width,
                       unsigned int height,
                       unsigned int newNbFrames, unsigned int nbDirs);

  /** Turns the sprite towards left */
  void setLookLeft();

  /** Turns the sprite towards right */
  void setLookRight();

  /** 
    * Test if the sprites looks to left
    * @return true if the sprite is directed towards left and false otherwise 
    */
  bool looksToLeft() const;

  /** 
    * Test if the sprites looks to right
    * @return true if the sprite is directed towards right and true otherwise 
    */
  bool looksToRight() const;

  /** Write property of bool approachDestByRight. */
  void setApproachDestByRight( const bool& _newVal);
  /** Read property of bool approachDestByRight. */
  bool getApproachDestByRight() const;

  /** Write property of bool approachDestByLeft . */
  void setApproachDestByLeft ( const bool& _newVal);
  /** Read property of bool approachDestByLeft . */
  bool getApproachDestByLeft () const;

  /** Write property of bool approachDestByTop. */
  void setApproachDestByTop( const bool& _newVal);
  /** Read property of bool approachDestByTop. */
  bool getApproachDestByTop() const;

  /** Write property of bool approachDestByBottom . */
  void setApproachDestByBottom ( const bool& _newVal);
  /** Read property of bool approachDestByBottom . */
  bool getApproachDestByBottom () const;

  /** 
    * Return the maximum value for x for this sprite by looking to its 
    * including background. Necessary for directed approaches. 
    */
  qreal getMaxX() const;

  /** 
    * Return the maximum value for y for this sprite by looking to its 
    * including background. Necessary for directed approaches. 
    */
  qreal getMaxY() const;

  /** returns the current state of the sprite */
  State getState() const;

  /** Return true if the state of the game is the argument; false otherwise */
  bool isMyState(State state) const;

  /** sets the new state of the game */
  void setState(State newState);

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
  void setupTravel(GameLogic::Country* src, GameLogic::Country* dest, 
    const QPointF& srcPoint, const QPointF& destPoint);

  /**
    * This virtual function chooses the approach mode of a sprite towards its
    * destination. It will be overloaded by subclasses:
    * if the distance between the origin and the destination is higher than half
    * the size of the map and if the origin and destination countries comunicate,
    * then the sprite should choose an approach by left or right, through the
    * edge of the map.
    */
  virtual void setupTravel(GameLogic::Country* src, GameLogic::Country* dest, 
    const QPointF* dpi=0);

  /**
    * Saves a XML representation of the sprite for game saving purpose
    * @param xmlStream The stream to write on
    */
  virtual void saveXml(QTextStream& xmlStream);
  
  /**
    * Retrieves the numFrame's frame image of this sprite in its current 
    * direction
    * @param numFrame The index of the image to retrieve
    * @return a copy of the numFrame's image of this sprite in its current 
    * direction
    */
  QPixmap image(unsigned int numFrame) const;

  void arrival();

  void setAnimated(unsigned int numberOfShots = std::numeric_limits<unsigned int>::max());

  void setStatic();

  void applyZoomFactor(qreal zoomFactor);

  void addDecoration(const QString& svgid, const QRectF& geometry);
  
public slots:
  void animate();

signals:

  /**
    * emitted when the coordinates of the sprite become equal to those of its
    * destination point
    */
  void atDestination(AnimSprite* sprite);

  void animationFinished(AnimSprite* sprite);

protected:
  virtual bool sceneEvent ( QEvent *  ) {return false;}

  /**
    * Set this sprite to display its numFrame's frame. If numFrame is greater
    * than the number of frames of the sprite, do nothing
    * @param numFrame The number of the frame to display
    */
  void setFrame(unsigned int numFrame);

  /**
    * Builds the sequence of images of this sprite using the SVG renderer and
    * data about zoom factor, look direction, etc. 
    */
  void sequenceConstruction();

  /**
    * changes the direction of this sprite's look
    */
  void setLook(TDir);

  bool m_animated;

  /**
   * Zoom factor
   */
  double m_zoom;
  
  private:
  QString m_svgid;

  /**
    * Change the active frame to the next one in the list. Use the first one
    * if the current frame was the last one
    *
    */
  void nextFrame();

  /**
  * Direction of the look of the sprite (left or right) ;
  * Allows to select the good image sequence
  */
  TDir look;

  /**
    * The number of versions of the sprite
    */
  unsigned int nbVersions;

  /**
    * The background onto which the sprite will be displayed
    */
  BackGnd *backGnd;

  /**
    * For a sprite moving from one country to another, the destination one ;
    * NULL otherwise.
    */
  GameLogic::Country *destination;

  /**
    * The coordinates of the destination (gun point or flag point for example)
    */
  QPointF destinationPoint;

  /**
    * the number of images in a version of the sprite
    */
  unsigned int frames;

  /**
    * the number of the current image in the current version of the sprite
    */
  unsigned int actFrame;

  /**
    * The attacking state of the sprite
    */
  State myState;

  /**
    * Position information needed to load graphics from the pool
    */
  double m_height, m_width;

  /** If this member is true, the sprite should approach its destination by
    * the left. So, if it is at the right side of its dest, it will continue
    * towards right up to the right side of the world. There, it will jump at
    * the left side and continue directly towards its destination.
    * When this member is set to true, the sprite should set
    * approachDestByRight to false. If both are false, the sprite will go
    * directly towards its destination.
    */
  bool approachDestByLeft ;

  /** If this member is true, the sprite should approach its destination by
    * the right. So, if it is at the left side of its dest, it will continue
    * towards left up to the left side of the world. There, it will jump at
    * the right side and continue directly towards its destination.
    * When this member is set to true, the sprite should set
    * approachDestByLeft to false. If both are false, the sprite will go
    * directly towards its destination.
    */
  bool approachDestByRight;

  /** If this member is true, the sprite should approach its destination by
    * the top. So, if it is under its dest, it will continue
    * towards the bottom down to the bottom side of the world. There, it will
    *  jump at the top side and continue directly towards its destination.
    * When this member is set to true, the sprite should set
    * approachDestByBottom to false. If both are false, the sprite will go
    * directly towards its destination.
    */
  bool approachDestByTop ;

  /** If this member is true, the sprite should approach its destination by
    * the bottom. So, if it is upper its dest, it will continue
    * towards the top up to the top side of the world. There, it will jump at
    * the bottom side and continue directly towards its destination.
    * When this member is set to true, the sprite should set
    * approachDestByTop to false. If both are false, the sprite will go
    * directly towards its destination.
    */
  bool approachDestByBottom;


  /**
    * Stores the images of this sprite for all its directions with the current
    * SVG file and zoom factor.
    */
  QList<QPixmap> m_frames;

  /**
    * This SVG renderer stores the SVG file of this sprite, renders it at the
    * desired zoom factor and the result is used to fill the frames list
    */
  QSvgRenderer* m_renderer;

  unsigned int m_numberOfShots;

  QTimer m_timer;

  QString m_skin;
};

} // closing namespace Ksirk

#endif // ANIMSPRITE_H
