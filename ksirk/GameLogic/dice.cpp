/***************************************************************************
                          dice.cpp  -  description
                             -------------------
    begin                : sat dec 29 2002
    copyright (C) 2002 by Gael de Chalendar (aka Kleag) <kleag@free.fr>
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

#include "dice.h"

#include <krandom.h>

namespace Ksirk
{

namespace GameLogic
{

Dice::Dice(){
}
Dice::~Dice(){
}

unsigned int Dice::roll(unsigned int max)
{
  return ( (KRandom::random() % max) + 1 ) ;
}

}
}
