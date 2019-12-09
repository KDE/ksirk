/* This file is part of KsirK.
   Copyright (C) 2008 Gael de Chalendar <kleag@free.fr>

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

#ifndef KSIRKNATIONALITYDEFINITION_H
#define KSIRKNATIONALITYDEFINITION_H

#include "ui_ksirknationalitydefinition.h"

#include <QDockWidget>

class KsirkNationalityDefinitionWidget : public QDockWidget, public Ui::KsirkNationalityDefinition
{
public:
  explicit KsirkNationalityDefinitionWidget(QWidget* parent) : QDockWidget(parent)
  {
    setupUi(this);
  }
};

#endif
