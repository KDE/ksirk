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

#ifndef INVASIONSLIDER_H
#define INVASIONSLIDER_H

#include <QDialog>

class QLabel;
class QSlider;

namespace Ksirk
{
 class KGameWindow;
 
namespace GameLogic
{
  class Country;
}
/**
  *
  * @author Gael de Chalendar (aka Kleag)
  * @version $Id: kgamewin.h 243 2007-02-24 00:22:58Z kleag $
  */
class InvasionSlider: public QDialog
{
  Q_OBJECT

public:
  enum InvasionType {Invasion, Moving};
  
  InvasionSlider(KGameWindow* game, GameLogic::Country *,GameLogic::Country *, InvasionType invasionType = Invasion);
  
  
  protected:

    
public Q_SLOTS:

  void slideMove(int v);
  void slideReleased();
  void slideClose();
  void slideCancel();
  
private:
  KGameWindow* m_game;
  
  int m_nbRArmy;
  int m_nbLArmy;
  int m_currentSlideValue;
  int m_previousSlideValue;

  QLabel * m_nbLArmies;
  QLabel * m_nbRArmies;
  QSlider * m_invadeSlide;
};

} // closing namespace Ksirk

#endif // INVASIONSLIDER_H

