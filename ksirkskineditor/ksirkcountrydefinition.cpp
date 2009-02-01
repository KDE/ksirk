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
  flagx->setValue((country->pointFlag().x()));
  flagy->setValue(country->pointFlag().y());
  
  anchorx->setValue(country->anchorPoint().x());
  anchory->setValue(country->anchorPoint().y());
  
  centerx->setValue(country->centralPoint().x());
  centery->setValue(country->centralPoint().y());
  
  infantryx->setValue(country->pointInfantry().x());
  infantryy->setValue(country->pointInfantry().y());
  
  cavalryx->setValue(country->pointCavalry().x());
  cavalryy->setValue(country->pointCavalry().y());
  
  cannonx->setValue(country->pointCannon().x());
  cannony->setValue(country->pointCannon().y());
  
  anchorx->setValue(country->anchorPoint().x());
  anchory->setValue(country->anchorPoint().y());
  
  centerx->setValue(country->centralPoint().x());
  centery->setValue(country->centralPoint().y());
  
  neighbourslist->clear();
  foreach(Country* neighbour, country->neighbours())
  {
    neighbourslist->addItem(neighbour->name());
  }
}
