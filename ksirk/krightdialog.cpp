/* This file is part of KsirK.

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

/* begin                : Thu JAN 22 2008 */


#include "krightdialog.h"
#include "GameLogic/country.h"
#include "GameLogic/player.h"
#include "Sprites/animsprite.h"

#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QGridLayout>
#include <kcmdlineargs.h>
#include <QScrollBar>
#include <QScrollArea>

namespace Ksirk
{
   using namespace GameLogic;
   
   KRightDialog::KRightDialog(QDockWidget * parent, ONU * world,KGameWindow* m_game)
   : QWidget(parent),
   m_parentWidget(parent),
   world(world),
   game(m_game),
   btRecycleWidget(0),
   btValidWidget(0),
   buttonStopAttack(0),
   flag1(0),
   flag2(0)
   {
	rightContents = new QList<QLabel*>();
        mainLayout = new QGridLayout(this);
	//mainLayout->setColumnMinimumWidth(0,30);
        setLayout(mainLayout);
        flag1 = new QLabel();flag2 = new QLabel();


	setFixedSize(220,360);

//scrollArea->setVerticalScrollBar(scroll);
//scrollArea->setMinimumWidth(width());
//scrollArea->setWidgetResizable(false);
//KStandardDirs* m_dirs=KGlobal::dirs();
//QString iconFileName = m_dirs-> findResource("appdata", dynamic_cast<KGameWindow*>(parent->parentWidget())->automaton()->skin() + "/Images/SoldatAGenoux1.png");

    // load the armie image
    KConfig config(world->getConfigFileName());
    KConfigGroup onugroup = config.group("onu");
    QString skin = onugroup.readEntry("skinpath");
    QString imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/sprites/infantry.svg");
    soldat.load(imageFileName);

    // load the stopAttackAuto image
    imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/stopAttackAuto.png");
    stopAttackAuto.load(imageFileName);

    // load the recycle image
    imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/distributionArmees.png");
    recycleContinue.load(imageFileName);

    // load the finish recycle image
    imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/distributionArmeesFinie.png");
    recycleDone.load(imageFileName);

    // load the next player image
    imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/joueurSuivant.png");
    recycleNextPlayer.load(imageFileName);

    //soldat.setFixedSize(8,8);
    show();
}

   KRightDialog::~KRightDialog()
   {
      delete mainLayout;
   }

   void KRightDialog::displayCountryDetails(QPointF * countryPoint)
   {

      clearLayout();
      QHBoxLayout * unit = new QHBoxLayout();
      QHBoxLayout * drap = new QHBoxLayout();
      initListLabel(7);

      //QGridLayout * contentLayout = new QGridLayout();

      QPixmap picture;

      picture = world->countryAt(*countryPoint)->owner()->getFlag()->image(0);

      QString continent = world->countryAt(*countryPoint)->continent()->name();
      QString pays = world->countryAt(*countryPoint)->name();
      QString units = QString::number(world->countryAt(*countryPoint)->nbArmies());
      QString owner = world->countryAt(*countryPoint)->owner()->name();

      rightContents->at(0)->setText(I18N_NOOP("<B>Nationality:</B>"));
      rightContents->at(1)->setText(I18N_NOOP("<B>Continent:</B> "+continent));
      rightContents->at(2)->setText(I18N_NOOP("<B>Country:</B> "+pays));

rightContents->at(3)->setPixmap(soldat.scaled(35,35,Qt::KeepAspectRatioByExpanding));rightContents->at(3)->setFixedSize(35,35);
      rightContents->at(6)->setText(I18N_NOOP(units));
      rightContents->at(4)->setText(I18N_NOOP("<B>Owner:</B> "+owner));
      rightContents->at(5)->setText(I18N_NOOP("<B><U>Country details</U></B>"));

      flag1->setPixmap(0);
      flag2->setPixmap(0);
      rightContents->push_back(flag1);
      flag1->setPixmap(picture);
      flag1->setFixedSize(picture.width(),picture.height());  

      drap->addWidget(rightContents->at(0));
      drap->addWidget(flag1);
      mainLayout->addWidget(rightContents->at(5),0,0,Qt::AlignLeft);
      mainLayout->addWidget(rightContents->at(1),2,0,Qt::AlignLeft);
      mainLayout->addWidget(rightContents->at(2),3,0,Qt::AlignLeft);
      
      mainLayout->addWidget(rightContents->at(4),5,0,Qt::AlignLeft);
      //mainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
      
      unit->addWidget(rightContents->at(3));
      unit->addWidget(rightContents->at(6));
      
      mainLayout->addLayout(unit,4,0,Qt::AlignLeft);
      mainLayout->addLayout(drap,1,0,Qt::AlignLeft);
      //mainLayout->addItem(contentLayout,0,0);

      m_parentWidget->show();
      repaint();
   }

   void KRightDialog::displayFightDetails(Country * attaker, Country * defender,int nb_A, int nb_D)
   {

      clearLayout();
      initListLabel(10);      QGridLayout * hautGrid = new QGridLayout(this);
      QGridLayout * basGrid = new QGridLayout(this);
QHBoxLayout * box1 = new QHBoxLayout();
QHBoxLayout * box2 = new QHBoxLayout();
QHBoxLayout * box3 = new QHBoxLayout();
QHBoxLayout * box4 = new QHBoxLayout();


     haut = new QWidget();
     bas = new QWidget();

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
      
      rightContents->at(0)->setText(I18N_NOOP("<u><b>"+pays_A+"</b></u> "));
      flag1->setPixmap(picture1);
      rightContents->at(1)->setText(I18N_NOOP("<i>("+owner_A+")</i> "));
      
      rightContents->at(2)->setPixmap(soldat.scaled(35,35,Qt::KeepAspectRatioByExpanding));
      rightContents->at(2)->setFixedSize(35,35);
      rightContents->at(3)->setText(I18N_NOOP("<b>"+nb_units_A+"</b> "));

      rightContents->at(4)->setText(I18N_NOOP("<font color=\"red\">Attack </font>with "+QString::number(nb_A)+" armies.<br>"));

      
      rightContents->at(5)->setText(I18N_NOOP("<u><b>"+pays_D+"</b></u> "));
      flag2->setPixmap(picture2);
      rightContents->at(6)->setText(I18N_NOOP("<i>("+owner_D+")</i> "));
      
      rightContents->at(7)->setPixmap(soldat.scaled(35,35,Qt::KeepAspectRatioByExpanding));
      rightContents->at(7)->setFixedSize(35,35);
      rightContents->at(8)->setText(I18N_NOOP("<b>"+nb_units_D+"</b> "));

      rightContents->at(9)->setText(I18N_NOOP("<font color=\"blue\">Defend </font>with "+QString::number(nb_D)+" armies.<br>"));

      box1->addWidget(rightContents->at(0));
      box1->addWidget(flag1);
      
      box2->addWidget(rightContents->at(2));
      box2->addWidget(rightContents->at(3));
      
      box3->addWidget(rightContents->at(5));
      box3->addWidget(flag2);

      box4->addWidget(rightContents->at(7));
      box4->addWidget(rightContents->at(8));

      hautGrid->addLayout(box1,0,0,Qt::AlignLeft);
      hautGrid->addWidget(rightContents->at(1),1,0,Qt::AlignCenter);
      hautGrid->addLayout(box2,2,0,Qt::AlignLeft);
      hautGrid->addWidget(rightContents->at(4),3,0,Qt::AlignCenter);
      
      basGrid->addLayout(box3,0,0,Qt::AlignLeft);
      basGrid->addWidget(rightContents->at(6),1,0,Qt::AlignCenter);
      basGrid->addLayout(box4,2,0,Qt::AlignLeft);
      basGrid->addWidget(rightContents->at(9),3,0,Qt::AlignCenter);

      mainLayout->addWidget(haut,0,0);
      mainLayout->addWidget(bas,2,0);

      if (game->automaton()->isAttackAuto()) {
         buttonStopAttack = new QPushButton(stopAttackAuto,"Stop Attack-auto");
         mainLayout->addWidget(buttonStopAttack,3,0);
         connect(buttonStopAttack, SIGNAL(clicked()), this, SLOT(slotStopAttackAuto()));
      }

      //mainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
      
      mainLayout->update();
      m_parentWidget->show();
      repaint();
   }

   void KRightDialog::displayRecycleDetails(GameLogic::Player * player) {
      clearLayout();
      initListLabel(4);

      QGridLayout* recycleLayout = new QGridLayout();
      QGridLayout* btRecycleLayout = new QGridLayout();
      QGridLayout* btValidLayout = new QGridLayout();

      QPushButton* buttonValid = new QPushButton(recycleNextPlayer, "Valid", this);
      QPushButton* buttonRecycle = new QPushButton(recycleContinue, "Recycle", this);
      QPushButton* buttonRecycleDone = new QPushButton(recycleDone, "Done", this);

      connect(buttonValid, SIGNAL(clicked()), game, SLOT(slotNextPlayer()));
      connect(buttonRecycle, SIGNAL(clicked()), game, SLOT(slotRecycling()));
      connect(buttonRecycleDone, SIGNAL(clicked()), game, SLOT(slotRecyclingFinished()));

      QHBoxLayout* title = new QHBoxLayout();

      // Widget where display informations
      haut = new QWidget();


      // Widgets which contains buttons
      btRecycleWidget = new QWidget();
      btValidWidget = new QWidget();

      btRecycleLayout->addWidget(buttonRecycle,0,0,Qt::AlignCenter);
      btRecycleLayout->addWidget(buttonRecycleDone,0,1,Qt::AlignCenter);

      btValidLayout->addWidget(buttonValid,0,0,Qt::AlignCenter);

      btRecycleWidget->setLayout(btRecycleLayout);
      btValidWidget->setLayout(btValidLayout);


      rightContents->at(0)->setText(I18N_NOOP("<u><b>"+player->name()+"</b></u> "));
      flag1->setPixmap(player->getFlag()->image(0));

//      rightContents->at(2)->setText(I18N_NOOP("<u>test 1</u>"));
//      rightContents->at(3)->setText(I18N_NOOP("<u>test 2</u>"));

      rightContents->at(1)->setText(I18N_NOOP(QString::number(game->availArmies())+" armies to place"));

      title->addWidget(rightContents->at(0));
      title->addWidget(flag1);

      recycleLayout->addLayout(title,0,0,Qt::AlignCenter);
      recycleLayout->addWidget(rightContents->at(1),1,0,Qt::AlignCenter);

      recycleLayout->addWidget(rightContents->at(2),2,0,Qt::AlignCenter);
      recycleLayout->addWidget(rightContents->at(3),3,0,Qt::AlignCenter);

      recycleLayout->addWidget(btRecycleWidget,4,0,Qt::AlignCenter);
      recycleLayout->addWidget(btValidWidget,5,0,Qt::AlignCenter);

      haut->setLayout(recycleLayout);
      mainLayout->addWidget(haut,0,0);

      // hide buttons initialy
      btRecycleWidget->hide();
      if (game->availArmies() > 0 || game->getState() == GameLogic::GameAutomaton::INTERLUDE) {
        btValidWidget->hide();
      } else {
        btValidWidget->show();
      }

      mainLayout->update();
      m_parentWidget->show();
      repaint();
   }

   void KRightDialog::updateRecycleDetails(GameLogic::Country* country, bool recyclePhase) {

      rightContents->at(1)->setText(I18N_NOOP(QString::number(game->availArmies())+" armies to place"));

      if (recyclePhase) {
        rightContents->at(2)->setText(QString());
        rightContents->at(3)->setText(QString());

        // show "redistribute" and "end redistribute" buttons
        btRecycleWidget->show();
        btValidWidget->hide();
      } else {
        rightContents->at(2)->setText(I18N_NOOP("<b>"+country->name()+"</b>"));
        rightContents->at(3)->setText(I18N_NOOP("<b>Armies : "+QString::number(country->nbArmies())));
        if (game->availArmies() > 0) {
          btValidWidget->hide();
        } else {
          btValidWidget->show();
        }
      }

      mainLayout->update();
      repaint();

   }

   void KRightDialog::displayFightResult(int A1=0, int A2=0, int A3=0, int D1=0, int D2=0,int nbA=0,int nbD=0, bool win=false)
   {

      QGridLayout * milieuGrid = new QGridLayout();   
      QHBoxLayout * deAtt = new QHBoxLayout();
      QHBoxLayout * deDef = new QHBoxLayout();
      milieu = new QWidget();

if(A1!=0 || A1!=-1)
{kDebug()<<"De attack 1" << endl;
      QLabel * de1 = new QLabel();
      de1->setPixmap(game->getDice(KGameWindow::Red,A1));
      rightContents->insert(0,de1);deAtt->addWidget(de1);
}
if(A2!=0 || A2!=-1)
{kDebug()<<"De attack 2" << endl;
      QLabel * de2= new QLabel();
      de2->setPixmap(game->getDice(KGameWindow::Red,A2));
      rightContents->insert(0,de2);deAtt->addWidget(de2);
}
if(A3!=0 || A3!=-1)
{kDebug()<<"De attack 3" << endl;
      QLabel * de3= new QLabel();
      de3->setPixmap(game->getDice(KGameWindow::Red,A3));
      rightContents->insert(0,de3);deAtt->addWidget(de3);
}
if(D1!=0 || D1!=-1)
{kDebug()<<"De defense 1" << endl;
      QLabel * de4= new QLabel();
      de4->setPixmap(game->getDice(KGameWindow::Blue,D1));
      rightContents->insert(0,de4);deDef->addWidget(de4);
}
if(D2!=0 || D2!=-1)
{kDebug()<<"De defense 2" << endl;
      QLabel * de5= new QLabel();
      de5->setPixmap(game->getDice(KGameWindow::Blue,D2));
      rightContents->insert(0,de5);deDef->addWidget(de5);
}
           rightContents->insert(0,new QLabel(I18N_NOOP("<font color=\"red\">loose armies : "+QString::number(nbA)+"</font>")));
rightContents->insert(0,new QLabel(I18N_NOOP("<font color=\"blue\">loose armies : "+QString::number(nbD)+"</font>")));
      /*deAtt->addWidget(rightContents->at(6)); 
      deAtt->addWidget(rightContents->at(5));
      deAtt->addWidget(rightContents->at(4));
      deDef->addWidget(rightContents->at(3));
      deDef->addWidget(rightContents->at(2));*/
    
milieuGrid->addWidget(rightContents->at(1),0,0,Qt::AlignCenter);
milieuGrid->addLayout(deAtt,1,0,Qt::AlignCenter);
milieuGrid->addLayout(deDef,2,0,Qt::AlignCenter);
milieuGrid->addWidget(rightContents->at(0),4,0,Qt::AlignCenter); 

      milieu->setLayout(milieuGrid);
      mainLayout->addWidget(milieu,1,0);

      if (buttonStopAttack != 0 && win) {
         buttonStopAttack->setEnabled(false);
      }

      repaint();
   }

   void KRightDialog::initListLabel(int nb)
   {
      removeListLabel();
      for (int i=0;i<nb;i++)
      {
         rightContents->push_back(new QLabel());
      }
      clearLabel();
   }
   
   void KRightDialog::removeListLabel()
   {
      while (rightContents->size() > 0) {
        QLabel* label = rightContents->first();
        rightContents->removeFirst();
        delete label;
      }
   }

   void KRightDialog::clearLabel()
   {
      for (int i=0;i<rightContents->size();i++)
      {
        rightContents->at(i)->setText("");rightContents->at(i)->setPixmap(0);
      }
   }
   
   void KRightDialog::clearLayout()
   {
     for (int i=0;i<rightContents->size();i++) { mainLayout->removeWidget(rightContents->at(i)); }
     if(mainLayout->indexOf(bas)!=-1) {
        mainLayout->removeWidget(bas);
     }
     if(mainLayout->indexOf(milieu)!=-1) {
        mainLayout->removeWidget(milieu);
     }
     if(mainLayout->indexOf(haut)!=-1) {
        mainLayout->removeWidget(haut);
     }
     if (btRecycleWidget != 0) {
        delete btRecycleWidget;
        btRecycleWidget = 0;
     }
     if (btValidWidget != 0) {
        delete btValidWidget;
        btValidWidget = 0;
     }
     if (buttonStopAttack != 0) {
        delete buttonStopAttack;
        buttonStopAttack = 0;
     }
     delete flag1;
     flag1 = new QLabel();
     delete flag2;
     flag2 = new QLabel();
   }

   void KRightDialog::slotStopAttackAuto()
   {
       kDebug()<<"Recept signal button stop attack auto" << endl;
       this->game->automaton()->setAttackAuto(false);
       this->buttonStopAttack->setEnabled(false);
   }
}
