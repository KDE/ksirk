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

#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QGridLayout>

namespace Ksirk
{
   using namespace GameLogic;
   
   KRightDialog::KRightDialog(QDockWidget * parent, ONU * world)
   : QGroupBox(parent), m_parentWidget(parent),world(world)
   {
	rightContents = new QList<QLabel*>();
        mainLayout = new QGridLayout();
	mainLayout->setColumnMinimumWidth(0,30);
        initListLabel(5);
   }
   
   KRightDialog::~KRightDialog()
   {
      delete mainLayout;
   }

   void KRightDialog::displayCountryDetails(QPointF * countryPoint)
   {
      clearLayout();

      QLabel * flag;
      QPixmap picture;

      picture = world->countryAt(*countryPoint)->owner()->getFlag()->image(0);

      QString continent = world->countryAt(*countryPoint)->continent()->name();
      QString pays = world->countryAt(*countryPoint)->name();
      QString units = QString::number(world->countryAt(*countryPoint)->nbArmies());
      QString owner = world->countryAt(*countryPoint)->owner()->name();

      rightContents->at(0)->setText("Nationality: ");
      rightContents->at(1)->setText("Continent: "+continent);
      rightContents->at(2)->setText("Country: "+pays);
      rightContents->at(3)->setText("Total units: "+units);
      rightContents->at(4)->setText("Owner: "+owner);
      
      flag = new QLabel();
      flag->setPixmap(picture);
      flag->setFixedSize(picture.width(),picture.height());  

      mainLayout->addWidget(rightContents->at(0),0,0,Qt::AlignLeft);
      mainLayout->addWidget(flag,0,1,Qt::AlignLeft);
      mainLayout->addWidget(rightContents->at(1),1,0,Qt::AlignLeft);
      mainLayout->addWidget(rightContents->at(2),2,0,Qt::AlignLeft);
      mainLayout->addWidget(rightContents->at(3),3,0,Qt::AlignLeft);
      mainLayout->addWidget(rightContents->at(4),4,0,Qt::AlignLeft);

      setAlignment(Qt::AlignTop | Qt::AlignLeft);
      adjustSize();
      setLayout(mainLayout);
      m_parentWidget->show();
   }

   void KRightDialog::displayFightDetails()
   {
   }

   void KRightDialog::displayFightResult()
   {
   }

   void KRightDialog::initListLabel(int nb)
   {
      removeListLabel();
      for (int i=0;i<nb;i++)
      {
         rightContents->push_back(new QLabel());
         //rightContents->at(i)->adjustSize();
         //mainLayout->setAlignment(rightContents->at(i), Qt::AlignLeft);
      }
   }
   
   void KRightDialog::removeListLabel()
   {
      for (int i=0;i<rightContents->size();i++) rightContents->removeAt(i);
   }

   void KRightDialog::clearLabel()
   {
      for (int i=0;i<rightContents->size();i++)
      {
        rightContents->at(i)->setText("");
        rightContents->at(i)->setPixmap(0);
      }
   }
   
   void KRightDialog::clearLayout()
   {
      for (int i=0;i<rightContents->size();i++)
      {
        mainLayout->removeWidget(rightContents->at(i));
      }
   }
}
