/***************************************************************************
                          dice.cpp  -  description
                             -------------------
    begin                : dim d� 29 2002
    copyright            : (C) 2002 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
