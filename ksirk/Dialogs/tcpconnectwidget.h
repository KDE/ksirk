/***************************************************************************
                          tcpconnectwidget.h  -  description
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
#ifndef TCPCONNECTWIDGET_H
#define TCPCONNECTWIDGET_H

#include "ui_tcpconnectwidget.h"

#include <QWidget>
#include <QMap>

namespace Ksirk
{

class TcpConnectWidget : public QWidget, public Ui::TcpConnectWidget
{
  Q_OBJECT
public:
  TcpConnectWidget(QWidget *parent=0);
  
  ~TcpConnectWidget() override;
  
Q_SIGNALS:
  void next();
  void previous();
  void cancel();
  
};

}

#endif // TCPCONNECTWIDGET_H
