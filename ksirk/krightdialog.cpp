/* This file is part of KsirK.
   Copyright (C) 2008 Gael Clouet <pelouas@hotmail.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

/* begin                : Thu JAN 22 2008 */

#include "krightdialog.h"
#include "Sprites/animsprite.h"
#include <qdrawutil.h>
#include <QColor>
#include <QBrush>
#include <QBitmap>
#include <QPixmap>
#include <QString>
#include <QScrollBar>
#include <QScrollArea>
#include <QPalette>
#include <QMovie>

namespace Ksirk
{
  using namespace GameLogic;

KRightDialog::KRightDialog(QDockWidget * parent, ONU * world,KGameWindow* m_game)
  : QWidget(parent),
  mainLayout(new QGridLayout(this)),
  m_parentWidget(parent),
  world(world),
  rightContents(),
  flag1(0),
  flag2(0),
  milieu(0),
  milieu2(0),
  btRecycleWidget(0),
  btValidWidget(0),
  game(m_game),
  buttonStopAttack(0),
  buttonStopDefense(0)
{
    setLayout(mainLayout);
//      setBaseSize(220,360);

    // load the armie image
    KConfig config(world->getConfigFileName());
    KConfigGroup onugroup = config.group("onu");
    QString skin = onugroup.readEntry("skinpath");

    InfantrySprite *sprite = new InfantrySprite(1.0, m_game->backGnd());
    sprite-> setDestination(0);             // Sprite immobile
    soldat = sprite->image(0).scaled(24,24,Qt::KeepAspectRatioByExpanding);
    delete sprite;
    
    // load the stopAttackAuto image
    QString imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/stopAttackAuto.png");
    stopAttackAuto.load(imageFileName);

    // load the recycle image
    imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + '/' +CM_RECYCLING);
    recycleContinue.load(imageFileName);

    // load the finish recycle image
    imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + '/' + CM_RECYCLINGFINISHED);
    recycleDone.load(imageFileName);

    // load the next player image
    imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + '/' + CM_NEXTPLAYER);
    recycleNextPlayer.load(imageFileName);

    show();
}

KRightDialog::~KRightDialog()
{
  delete mainLayout;
}

void KRightDialog::displayCountryDetails(const QPointF& countryPoint)
{
  qCDebug(KSIRK_LOG);
  Country* country = world->countryAt(countryPoint);
  if (country == 0)
  {
    return;
  }
  clearLayout();
  QHBoxLayout * unit = new QHBoxLayout();
  QHBoxLayout * drap = new QHBoxLayout();
  flag1 = new QLabel();
  flag2 = new QLabel();
  initListLabel(7);

  QPixmap picture;

  picture = country->owner()->getFlag()->image(0);

  QString continent = i18n(country->continent()->name().toUtf8().data());
  QString pays = i18n(country->name().toUtf8().data());
  QString units = QString::number(country->nbArmies());
  QString owner = country->owner()->name();

  rightContents.at(0)->setText(i18n("<b>Nationality:</b>"));
  rightContents.at(1)->setText(i18n("<b>Continent:</b> %1", continent));
  rightContents.at(2)->setText(i18n("<b>Country:</b> %1", pays));

  rightContents.at(3)->setPixmap(soldat);
  rightContents.at(6)->setText(units);
  rightContents.at(4)->setText(i18n("<b>Owner:</b> %1", owner));
  rightContents.at(5)->setText(i18n("<b><u>Country details</u></b>"));

  flag1->setPixmap(QPixmap());
  flag2->setPixmap(QPixmap());
  flag1->setPixmap(picture);

  drap->addWidget(rightContents.at(0));
  drap->addWidget(flag1);
  mainLayout->addWidget(rightContents.at(5),0,0,Qt::AlignLeft);
  mainLayout->addWidget(rightContents.at(1),2,0,Qt::AlignLeft);
  mainLayout->addWidget(rightContents.at(2),3,0,Qt::AlignLeft);

  mainLayout->addWidget(rightContents.at(4),5,0,Qt::AlignLeft);

  unit->addWidget(rightContents.at(3));
  unit->addWidget(rightContents.at(6));

  mainLayout->addLayout(unit,4,0,Qt::AlignLeft);
  mainLayout->addLayout(drap,1,0,Qt::AlignLeft);

  m_parentWidget->show();
  repaint();
}

void KRightDialog::displayFightDetails(Country * attaker, Country * defender,int nb_A, int nb_D)
{
  qCDebug(KSIRK_LOG);

  clearLayout();
  initListLabel(10);

  haut = new QWidget();
  bas = new QWidget();
  QGridLayout * hautGrid = new QGridLayout(this);
  QGridLayout * basGrid = new QGridLayout(this);

  flag1 = new QLabel();
  flag2 = new QLabel();

  QHBoxLayout * box1 = new QHBoxLayout();
  QHBoxLayout * box2 = new QHBoxLayout();
  QHBoxLayout * box3 = new QHBoxLayout();
  QHBoxLayout * box4 = new QHBoxLayout();

  QGridLayout * tp = new QGridLayout();
  infoProcess = new QLabel();
  infoProcess->setWordWrap(true);
  infoProcess->setText(i18n("<i>Fighting in progress...</i>"));
  loadingLabel = new QLabel();
  QLabel *l3 = new QLabel();
  QLabel *l4 = new QLabel();

  KConfig config(world->getConfigFileName());
  KConfigGroup onugroup = config.group("onu");
  QString skin = onugroup.readEntry("skinpath");
  QString imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/loader.gif");

  QMovie * loading = new QMovie(imageFileName);
  loadingLabel->setMovie(loading);
  loading->start();

  tp->addWidget(l3,0,0);
  tp->addWidget(infoProcess,1,0,Qt::AlignCenter);
  tp->addWidget(loadingLabel,2,0,Qt::AlignCenter);
  tp->addWidget(l4,3,0);

  milieu2 = new QWidget(this);
  milieu2->setAutoFillBackground(true);
  QPalette tempP(milieu2->palette());
  tempP.setColor(QPalette::Window,QColor(190,190,190));
  milieu2->setPalette(tempP);

  milieu2->setLayout(tp);

  haut->setLayout(hautGrid);
  bas->setLayout(basGrid);

  //ATTACKER
  QString owner_A = attaker->owner()->name();
  QString nb_units_A = QString::number(attaker->nbArmies());
  QString pays_A = attaker->name();

  QString owner_D = defender->owner()->name();
  QString nb_units_D = QString::number(defender->nbArmies());
  QString pays_D = defender->name();

  QPixmap picture1;
  QPixmap picture2;
  picture1 = attaker->owner()->getFlag()->image(0);
  picture2 = defender->owner()->getFlag()->image(0);

  rightContents.at(0)->setText("<u><b>"+i18n(pays_A.toUtf8().data())+"</b></u>");
  flag1->setPixmap(picture1);
  rightContents.at(1)->setText("<i>("+owner_A+")</i> ");

  if (!soldat.isNull())
    rightContents.at(2)->setPixmap(soldat);
  rightContents.at(3)->setText("<b>"+nb_units_A+"</b>");

  rightContents.at(4)->setText(i18np("<font color=\"red\">Attack</font> with 1 army.<br>","<font color=\"red\">Attack</font> with %1 armies.<br>", nb_A));


  rightContents.at(5)->setText("<u><b>"+i18n(pays_D.toUtf8().data())+"</b></u> ");
  flag2->setPixmap(picture2);
  rightContents.at(6)->setText("<i>("+owner_D+")</i> ");

  if (!soldat.isNull())
    rightContents.at(7)->setPixmap(soldat);
  rightContents.at(8)->setText("<b>"+nb_units_D+"</b> ");

  rightContents.at(9)->setText(i18np("<font color=\"blue\">Defend</font> with 1 army.<br>","<font color=\"blue\">Defend</font> with %1 armies.<br>", nb_D));

  box1->addWidget(rightContents.at(0));
  box1->addWidget(flag1);

  box2->addWidget(rightContents.at(2));
  box2->addWidget(rightContents.at(3));

  box3->addWidget(rightContents.at(5));
  box3->addWidget(flag2);

  box4->addWidget(rightContents.at(7));
  box4->addWidget(rightContents.at(8));

  hautGrid->addLayout(box1,0,0,Qt::AlignCenter);
  hautGrid->addWidget(rightContents.at(1),1,0,Qt::AlignCenter);
  hautGrid->addLayout(box2,2,0,Qt::AlignLeft);
  hautGrid->addWidget(rightContents.at(4),3,0,Qt::AlignLeft);

  basGrid->addLayout(box3,0,0,Qt::AlignCenter);
  basGrid->addWidget(rightContents.at(6),1,0,Qt::AlignCenter);
  basGrid->addLayout(box4,2,0,Qt::AlignLeft);
  basGrid->addWidget(rightContents.at(9),3,0,Qt::AlignLeft);

  mainLayout->addWidget(haut,0,0);
  mainLayout->addWidget(milieu2,1,0);
  mainLayout->addWidget(bas,2,0);

  if (game->automaton()->isAttackAuto()
    && !game->automaton()->currentPlayer()->isAI()
      && !game->automaton()->currentPlayer()->isVirtual())
  {
      buttonStopAttack = new QPushButton(stopAttackAuto,i18n("Stop Auto-Attack"));
      mainLayout->addWidget(buttonStopAttack,3,0);
      connect(buttonStopAttack, &QAbstractButton::clicked, this, &KRightDialog::slotStopAttackAuto);
  }
  if (game->automaton()->isDefenseAuto()
    && !game->automaton()->currentPlayer()->isAI()
    && !game->automaton()->currentPlayer()->isVirtual())
  {
    buttonStopDefense = new QPushButton(stopAttackAuto,i18n("Stop Auto-Defense"));
    mainLayout->addWidget(buttonStopDefense,4,0);
    connect(buttonStopDefense, &QAbstractButton::clicked, this, &KRightDialog::slotStopDefenseAuto);
  }

  mainLayout->update();
  m_parentWidget->show();
  repaint();
}

 void KRightDialog::displayRecycleDetails(GameLogic::Player * player, int nbAvailArmies)
 {
    qCDebug(KSIRK_LOG) << player->name() << nbAvailArmies;
    this->show();

    clearLayout();
    initListLabel(4);

    flag1 = new QLabel();
    flag2 = new QLabel();

    QGridLayout* recycleLayout = new QGridLayout();
    QGridLayout* btRecycleLayout = new QGridLayout();
    QGridLayout* btValidLayout = new QGridLayout();

    QPushButton* buttonValid = new QPushButton(recycleNextPlayer, i18n("Valid"), this);
    QPushButton* buttonRecycle = new QPushButton(recycleContinue, i18n("Recycle"), this);
    QPushButton* buttonRecycleDone = new QPushButton(recycleDone, i18n("Done"), this);

    connect(buttonValid, &QAbstractButton::clicked, game, &KGameWindow::slotNextPlayer);
    connect(buttonRecycle, &QAbstractButton::clicked, game, &KGameWindow::slotRecycling);
    connect(buttonRecycleDone, &QAbstractButton::clicked, game, &KGameWindow::slotRecyclingFinished);

    QHBoxLayout* title = new QHBoxLayout();

    haut = new QWidget();


    // Widgets which contains buttons
    btRecycleWidget = new QWidget();
    btValidWidget = new QWidget();

    btRecycleLayout->addWidget(buttonRecycle,0,0,Qt::AlignCenter);
    btRecycleLayout->addWidget(buttonRecycleDone,0,1,Qt::AlignCenter);

    btValidLayout->addWidget(buttonValid,0,0,Qt::AlignCenter);

    btRecycleWidget->setLayout(btRecycleLayout);
    btValidWidget->setLayout(btValidLayout);


    rightContents.at(0)->setText("<u><b>"+player->name()+"</b></u> ");
    flag1->setPixmap(player->getFlag()->image(0));

    rightContents.at(1)->setText(i18np("%1 army to place", "%1 armies to place", nbAvailArmies));

    title->addWidget(rightContents.at(0));
    title->addWidget(flag1);

    recycleLayout->addLayout(title,0,0,Qt::AlignCenter);
    recycleLayout->addWidget(rightContents.at(1),1,0,Qt::AlignCenter);

    recycleLayout->addWidget(rightContents.at(2),2,0,Qt::AlignCenter);
    recycleLayout->addWidget(rightContents.at(3),3,0,Qt::AlignCenter);

    recycleLayout->addWidget(btRecycleWidget,4,0,Qt::AlignCenter);
    recycleLayout->addWidget(btValidWidget,5,0,Qt::AlignCenter);

    haut->setLayout(recycleLayout);
    mainLayout->addWidget(haut,0,0);

    // hide buttons initialy
    btRecycleWidget->hide();
    if (nbAvailArmies > 0 || game->getState() == GameLogic::GameAutomaton::INTERLUDE || player->isVirtual() || player->isAI())
    {
      btValidWidget->hide();
    }
    else
    {
      btValidWidget->show();
    }

    mainLayout->update();
    m_parentWidget->show();
    repaint();
 }

void KRightDialog::updateRecycleDetails(GameLogic::Country* country, bool recyclePhase, int nbAvailArmies)
{
  qCDebug(KSIRK_LOG) << (void*)country << recyclePhase << nbAvailArmies;
  this->show();
  if (btValidWidget == 0)
  {
    if (country == 0)
    {
      return;
    }
    displayRecycleDetails(country->owner(),nbAvailArmies);
  }

  if (recyclePhase)
  {
    rightContents.at(0)->setText(i18n("<u><b>Change some<br>placements?</b></u> "));
    flag1->hide();
    rightContents.at(1)->setText(QString());
    rightContents.at(2)->setText(QString());
    rightContents.at(3)->setText(QString());

    // show "redistribute" and "end redistribute" buttons
    if (!game->automaton()->allLocalPlayersComputer())
    {
      btRecycleWidget->show();
    }
    btValidWidget->hide();
  }
  else
  {
    rightContents.at(1)->setText(i18np("1 army to place", "%1 armies to place", nbAvailArmies));
    rightContents.at(2)->setText("<b>"+i18n(country->name().toUtf8().data())+"</b>");
    rightContents.at(3)->setText(i18n("<b>Armies:</b> %1", country->nbArmies()));
    if (nbAvailArmies > 0)
    {
      btValidWidget->hide();
    }
    else
    {
      if (!game->currentPlayer()->isVirtual() && !game->currentPlayer()->isAI())
      {
        btValidWidget->show();
      }
    }
  }
  qCDebug(KSIRK_LOG) << "before update and repaint";
  mainLayout->update();
  repaint();

}

void KRightDialog::displayFightResult(int A1=0, int A2=0, int A3=0, int D1=0, int D2=0,int nbA=0,int nbD=0, bool win=false)
{
  qCDebug(KSIRK_LOG);

  // Bug 309863. Should not happen anymore because Country details and Fight results shouldn't be mixed.
  // If, by any chance, occurs again, do not crash and log the incident. In that case label just won't be refreshed.
  if (loadingLabel != 0)
  {
    QMovie* movie = loadingLabel->movie();
    delete movie;
    loadingLabel->clear();
  }
  else
    qCDebug(KSIRK_LOG) << "Item (loadingLabel) has already been cleared!";

  if (infoProcess != 0)
    infoProcess->clear();
  else
    qCDebug(KSIRK_LOG) << "Item (infoProcess) has already been cleared!";

  milieu = new QWidget(this);
  milieu->setAutoFillBackground(true);
  QPalette tempP(milieu->palette());
  tempP.setColor(QPalette::Window,QColor(190,190,190));
  milieu->setPalette(tempP);

  QGridLayout * milieuGrid = new QGridLayout();
  QHBoxLayout * deAtt = new QHBoxLayout();
  QHBoxLayout * deDef = new QHBoxLayout();

  if(A1!=0 && A1!=-1)
  {
    QLabel * de1 = new QLabel();
    de1->setPixmap(game->getDice(KGameWindow::Red,A1));
    rightContents.insert(0,de1);deAtt->addWidget(de1);
  }
  if(A2!=0 && A2!=-1)
  {
    QLabel * de2= new QLabel();
    de2->setPixmap(game->getDice(KGameWindow::Red,A2));
    rightContents.insert(0,de2);deAtt->addWidget(de2);
  }
  if(A3!=0 && A3!=-1)
  {
    QLabel * de3= new QLabel();
    de3->setPixmap(game->getDice(KGameWindow::Red,A3));
    rightContents.insert(0,de3);deAtt->addWidget(de3);
  }
  if(D1!=0 && D1!=-1)
  {
    QLabel * de4= new QLabel();
    de4->setPixmap(game->getDice(KGameWindow::Blue,D1));
    rightContents.insert(0,de4);deDef->addWidget(de4);
  }
  if(D2!=0 && D2!=-1)
  {
    QLabel * de5= new QLabel();
    de5->setPixmap(game->getDice(KGameWindow::Blue,D2));
    rightContents.insert(0,de5);deDef->addWidget(de5);
  }
  QLabel * rLabelR = new QLabel(i18n("<font color=\"red\">lost armies: %1</font>", nbA));
      rLabelR->setWordWrap(true);

  rightContents.insert(0,rLabelR);

  QLabel * rLabelB = new QLabel(i18n("<font color=\"blue\">lost armies: %1</font>", nbD));
      rLabelB->setWordWrap(true);

  rightContents.insert(0,rLabelB);

  milieuGrid->addWidget(rightContents.at(1),0,0,Qt::AlignCenter);
  milieuGrid->addLayout(deAtt,1,0,Qt::AlignCenter);
  milieuGrid->addLayout(deDef,2,0,Qt::AlignCenter);
  milieuGrid->addWidget(rightContents.at(0),4,0,Qt::AlignCenter);

  milieu->setLayout(milieuGrid);
  mainLayout->addWidget(milieu,1,0);

  if (buttonStopAttack != 0 && win)
  {
      buttonStopAttack->setEnabled(false);
  }
  repaint();
}

void KRightDialog::initListLabel(int nb)
{
  qCDebug(KSIRK_LOG);

  removeListLabel();
  for (int i=0;i<nb;i++)
  {
    QLabel* label = new QLabel();
    label->setWordWrap ( true );
    rightContents.push_back(label);
  }
  clearLabel();
}

void KRightDialog::removeListLabel()
{
  qCDebug(KSIRK_LOG);
  while (rightContents.size() > 0)
  {
    QLabel* label = rightContents.first();
    rightContents.removeFirst();
    delete label;
  }
}

void KRightDialog::clearLabel()
{
  qCDebug(KSIRK_LOG);
  for (int i=0;i<rightContents.size();i++)
  {
    rightContents.at(i)->setText("");
    rightContents.at(i)->setPixmap(QPixmap());
  }
}

void KRightDialog::clearLayout()
{
  qCDebug(KSIRK_LOG);
  while (rightContents.size() > 0)
  {
    QLabel* obj = rightContents.first();
    rightContents.removeFirst();
    mainLayout->removeWidget(obj);
    delete obj;
  }
  if (flag1 != 0)
  {
    mainLayout->removeWidget(flag1);
    delete flag1;
    flag1 = 0;
  }
  if (flag2 != 0)
  {
    mainLayout->removeWidget(flag2);
    delete flag2;
    flag2 = 0;
  }
  if (btRecycleWidget != 0)
  {
    delete btRecycleWidget;
    btRecycleWidget = 0;
  }
  if (btValidWidget != 0)
  {
    delete btValidWidget;
    btValidWidget = 0;
  }
  if (buttonStopAttack != 0)
  {
    mainLayout->removeWidget(buttonStopAttack);
    delete buttonStopAttack;
    buttonStopAttack = 0;
  }
  if (buttonStopDefense != 0)
  {
    mainLayout->removeWidget(buttonStopDefense);
    delete buttonStopDefense;
    buttonStopDefense = 0;
  }
  if(bas != 0 && mainLayout->indexOf(bas)!=-1)
  {
    mainLayout->removeWidget(bas);
    delete bas;
  }
  if(milieu != 0)
  {
    mainLayout->removeWidget(milieu);
    delete milieu;
    milieu = 0;
  }
  if (milieu2 != 0)
  {
    mainLayout->removeWidget(milieu2);
    QMovie* movie = loadingLabel->movie();
    loadingLabel->clear();
    delete movie;
    delete loadingLabel;
    loadingLabel = 0;
    delete infoProcess;
    infoProcess = 0;
    delete milieu2;
    milieu2 = 0;
  }
  if(mainLayout->indexOf(haut)!=-1)
  {
    mainLayout->removeWidget(haut);
    delete haut;
  }
}

void KRightDialog::slotStopAttackAuto()
{
  qCDebug(KSIRK_LOG);
  this->game->automaton()->setAttackAuto(false);
  this->buttonStopAttack->setEnabled(false);
}

void KRightDialog::slotStopDefenseAuto()
{
  qCDebug(KSIRK_LOG);
  this->game->automaton()->setDefenseAuto(false);
  this->buttonStopDefense->setEnabled(false);
}

}


