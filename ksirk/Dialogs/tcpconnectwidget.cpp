/***************************************************************************
                          tcpconnectwidget.cpp  -  description
                             -------------------
    begin                : Sun Sep 20 2009
    copyright            : (C) 2009 by Gaël de Chalendar
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

#include "tcpconnectwidget.h"

#include "ksirk_debug.h"

namespace Ksirk
{

TcpConnectWidget::TcpConnectWidget(QWidget *parent) : QWidget(parent), Ui::TcpConnectWidget()
{
  qCDebug(KSIRK_LOG) << "";
  setupUi(this);
  QRegExp rx(".+");
  QRegExpValidator *v = new QRegExpValidator(rx, this);
  hostEdit->setValidator(v);

  connect(nextButton,SIGNAL(clicked(bool)),this,SIGNAL(next()));
  connect(previousButton,SIGNAL(clicked(bool)),this,SIGNAL(previous()));
  connect(cancelButton,SIGNAL(clicked(bool)),this,SIGNAL(cancel()));
}

TcpConnectWidget::~TcpConnectWidget()
{
}

}
