/* This file is part of KsirK.
   Copyright (C) 2002-2007 Gael de Chalendar <kleag@free.fr>

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

#ifndef DICE_H
#define DICE_H

namespace Ksirk
{

namespace GameLogic
{


/**
  * This class implements a dice : roll it with its static method to obtain a 
  * random number between 1 and 6.
  * @author Gaël de Chalendar (aka Kleag) 
  */

class Dice
{
public: 
	Dice();
	~Dice();
	
	/** rolls the dice.
	@param max the maximum of the dice (six by default)
	@return a random integer between one and the argument (six by default) */
	static unsigned int roll(unsigned int max=6);

};

}
}

#endif
