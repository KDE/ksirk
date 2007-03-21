/* This file is part of KsirK.
   Copyright (C) 2005-2007 Gaël de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KSIRK_GAMELOGICAIPLAYERIO_H
#define KSIRK_GAMELOGICAIPLAYERIO_H

#include "aiplayer.h"

#include <kgame/kgameio.h>

namespace Ksirk {

namespace GameLogic {

#define AIPLAYERIO 32

/**
  * This is the IO device used by AI players
  * @author Gaël de Chalendar (aka Kleag)
  */
class AIPlayerIO : public KGameIO
{
public:
  /** Constructor of the IO device for the given AI player. */
  AIPlayerIO(AIPlayer* aiplayer);

  /** Default destructor. */
  ~AIPlayerIO();

  /** 
    * The KGame IO devices must have a rtti function returning a value 
    * different for each different IO class. 
    */
  virtual int rtti () const {return AIPLAYERIO;}
};

}

}

#endif
