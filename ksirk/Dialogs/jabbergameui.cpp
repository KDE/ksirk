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

#include "jabbergameui.h"
#include "ksirksettings.h"

#include <KDebug>

KsirkJabberGameWidget::KsirkJabberGameWidget(QWidget* parent) : QWidget(parent)
{
  kDebug();
  
  setupUi(this);
  jabberid->setText(Ksirk::KsirkSettings::jabberId());
  password->setText(Ksirk::KsirkSettings::jabberPassword());
  roomjid->setText(Ksirk::KsirkSettings::roomJid());
  roompassword->setText(Ksirk::KsirkSettings::roomPassword());
  nickname->setText(Ksirk::KsirkSettings::nickname());
}

#include "jabbergameui.moc"