/* This file is part of KsirK.
 *   copyright (C) 2002-2007 Gael de Chalendar <kleag@free.fr>
 * 
 *   KsirK is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation, version 2.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 */
#include "aiplayerio.h"

namespace Ksirk {

namespace GameLogic {

AIPlayerIO::AIPlayerIO(AIPlayer *aiplayer)  
  : KGameIO(aiplayer)
{
}

AIPlayerIO::~AIPlayerIO()
{
}


}

}
