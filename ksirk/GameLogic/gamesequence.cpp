/*
    This file is part of KsirK.
    SPDX-FileCopyrightText: 2023 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamesequence.h"

// own
#include "gameautomaton.h"

namespace Ksirk{
namespace GameLogic {

GameSequence::GameSequence(GameAutomaton *gameAutomation)
    : m_gameAutomation(gameAutomation)
{
}

GameSequence::~GameSequence() = default;

KPlayer *GameSequence::nextPlayer(KPlayer *last, bool exclusive)
{
    return m_gameAutomation->doNextPlayer(last, exclusive);
}

}
}
