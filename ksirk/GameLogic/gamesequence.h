/*
    This file is part of KsirK.
    SPDX-FileCopyrightText: 2023 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSIRK_GAMELOGIC_GAMESEQUENCE_H
#define KSIRK_GAMELOGIC_GAMESEQUENCE_H

// KDEGames
#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgame/kgamesequence.h>

namespace Ksirk {

namespace GameLogic {

class GameAutomaton;

class GameSequence : public KGameSequence
{
    Q_OBJECT

public:
    explicit GameSequence(GameAutomaton *gameAutomation);
    ~GameSequence() override;

public:
    KPlayer *nextPlayer(KPlayer *last, bool exclusive = true) override;

private:
    GameAutomaton *const m_gameAutomation;
};

}
}

#endif
