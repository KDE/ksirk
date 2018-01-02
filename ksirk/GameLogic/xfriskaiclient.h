/*
 *   XFrisk - The classic board game for X
 *   Copyright (C) 1993-1999 Elan Feingold (elan@aetherworks.com)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 *
 *   $Id: aiClient.h,v 1.3 1999/11/13 21:58:30 morphy Exp $
 */

#ifndef _AICLIENT
#define _AICLIENT

/* Commands sent to the callback */
#define AI_INIT_ONCE        0
#define AI_INIT_GAME        1
#define AI_INIT_TURN        2
#define AI_FORTIFY          3 /* (Number of armies) */
#define AI_PLACE            4
#define AI_ATTACK           5
#define AI_MOVE             6
#define AI_EXCHANGE_CARDS   7
#define AI_SERVER_MESSAGE   8 /* (Text of the message) */
#define AI_MESSAGE          9 /* (Text of the message) */
#define AI_MOVE_MANUAL     10 /* (Number of armies) */

/* Actions computer players can perform -- PUBLIC INTERFACE */
// int AI_Attack(int iSrcCountry, int iDstCountry, 
// 	       int iAttackMode, int iDiceMode, int iMoveMode) {return -1;}
// int AI_ExchangeCards(int *piCards) {return -1;}

/* Attack modes */
#define ATTACK_ONCE      0
#define ATTACK_DOORDIE   1

/* Dice modes */
#define DICE_MAXIMUM     0
#define DICE_ONE         1
#define DICE_TWO         2
#define DICE_THREE       3

/* What to do with armies after taking a country.  If you wish to move
 * the maximum amount of armies into the country you might occupy with
 * the attack, pass ARMIES_MOVE_MAX.  If you want the minimum number
 * of armies moved, pass ARMIES_MOVE_MIN.  If you want finer control,
 * pass in ARMIES_MOVE_MANUAL, and your callback will be called with
 * the command AI_MOVE_MANUAL.
 */

#define ARMIES_MOVE_MAX    0
#define ARMIES_MOVE_MIN    1
#define ARMIES_MOVE_MANUAL 2

#endif
