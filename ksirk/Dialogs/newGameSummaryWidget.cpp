/***************************************************************************
                          newGameSummaryWidget.cpp  -  description
                             -------------------
    begin                : Fri Jul 31 2009
    copyright            : (C) 2009 by Gaël de Chalendar
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

#include "newGameSummaryWidget.h"
#include "kgamewin.h"
#include "newgamesetup.h"
#include "onu.h"
#include "GameLogic/newplayerdata.h"

#include <KLocale>
#include <KDebug>

namespace Ksirk
{

NewGameSummaryWidget::NewGameSummaryWidget(QWidget *parent) :
  QWidget(parent),
  Ui::NewGameSummary()
{
  kDebug();
  setupUi(this);
  connect(previousButton,SIGNAL(clicked()),this,SIGNAL(previous()));
  connect(cancelButton,SIGNAL(clicked()),this,SIGNAL(cancel()));
  playersTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}
  
NewGameSummaryWidget::~NewGameSummaryWidget()
{
}

void NewGameSummaryWidget::show(KGameWindow* game)
{
  kDebug() << game->newGameSetup()->worlds().keys();
  foreach (GameLogic::ONU* world, game->newGameSetup()->worlds())
  {
    if (world->skin() == game->newGameSetup()->skin())
    {
      skinSnapshotPixmap->setPixmap(world->snapshot());
      skinNameLabel->setText(i18n(world->name().toUtf8().data()));
      goalTypeLabel->setText(game->newGameSetup()->useGoals()?i18n("Reach a goal"):i18n("World conquest"));
    }
  }
  kDebug() << game->newGameSetup()->players().size() << game->newGameSetup()->nbPlayers() << game->newGameSetup()->nbLocalPlayers() << game->newGameSetup()->nbNetworkPlayers();
  playersTable->setRowCount(game->newGameSetup()->players().size());
  int row = 0;
  foreach (NewPlayerData* player, game->newGameSetup()->players())
  {
    kDebug() << "player" << player->name();
    QTableWidgetItem *nameItem = new QTableWidgetItem(player->name());
    playersTable->setItem(row, 0, nameItem);
    QTableWidgetItem *nationItem = new QTableWidgetItem(player->nation());
    playersTable->setItem(row, 1, nationItem);
    QTableWidgetItem *computerItem = new QTableWidgetItem(player->computer()?i18n("Yes"):i18n("No"));
    playersTable->setItem(row, 2, computerItem);
    QTableWidgetItem *netItem = new QTableWidgetItem(player->network()?i18n("Yes"):i18n("No"));
    playersTable->setItem(row, 3, netItem);
    row++;
  }
  if (game->automaton()->networkGameType() != GameLogic::GameAutomaton::None)
  {
    if (game->automaton()->isAdmin()
      && game->newGameSetup()->players().size() == game->newGameSetup()->nbPlayers())
    {
      finishButton->setEnabled(true);
    }
    else
    {
      finishButton->setEnabled(false);
    }
    previousButton->setEnabled(false);
  }
}

}

