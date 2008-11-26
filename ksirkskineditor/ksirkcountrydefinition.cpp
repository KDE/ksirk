/* This file is part of KsirK.
   Copyright (C) 2008 Gael de Chalendar <kleag@free.fr>

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

#include "ksirkcountrydefinition.h"
#include "country.h"

using namespace KsirkSkinEditor;

KsirkCountryDefinitionWidget::KsirkCountryDefinitionWidget(QWidget* parent) : QDockWidget(parent)
{
  setupUi(this);
}

void KsirkCountryDefinitionWidget::initWith(const Country* country)
{
  flagx->setText(QString::number(country->pointFlag().x()));
  flagy->setText(QString::number(country->pointFlag().y()));
  
  anchorx->setText(QString::number(country->anchorPoint().x()));
  anchory->setText(QString::number(country->anchorPoint().y()));
  
  centerx->setText(QString::number(country->centralPoint().x()));
  centery->setText(QString::number(country->centralPoint().y()));
  
  infantryx->setText(QString::number(country->pointInfantry().x()));
  infantryy->setText(QString::number(country->pointInfantry().y()));
  
  cavalryx->setText(QString::number(country->pointCavalry().x()));
  cavalryy->setText(QString::number(country->pointCavalry().y()));
  
  cannonx->setText(QString::number(country->pointCannon().x()));
  cannony->setText(QString::number(country->pointCannon().y()));
  
  anchorx->setText(QString::number(country->anchorPoint().x()));
  anchory->setText(QString::number(country->anchorPoint().y()));
  
  centerx->setText(QString::number(country->centralPoint().x()));
  centery->setText(QString::number(country->centralPoint().y()));
  
  neighbourslist->clear();
  foreach(Country* neighbour, country->neighbours())
  {
    neighbourslist->addItem(neighbour->name());
  }
}
