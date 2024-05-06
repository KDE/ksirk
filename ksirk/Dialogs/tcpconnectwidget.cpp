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

#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace Ksirk
{

TcpConnectWidget::TcpConnectWidget(QWidget *parent) : QWidget(parent), Ui::TcpConnectWidget()
{
  qCDebug(KSIRK_LOG) << "";
  setupUi(this);
  const QString nextIcon = isRightToLeft() ? QStringLiteral("go-previous") : QStringLiteral("go-next");
  nextButton->setIcon(QIcon::fromTheme(nextIcon));
  const QString previousIcon = isRightToLeft() ? QStringLiteral("go-next") : QStringLiteral("go-previous");
  previousButton->setIcon(QIcon::fromTheme(previousIcon));
  QRegularExpression rx(QStringLiteral("^.+$"));
  auto *v = new QRegularExpressionValidator(rx, this);
  hostEdit->setValidator(v);

  connect(nextButton,&QAbstractButton::clicked,this,&TcpConnectWidget::next);
  connect(previousButton,&QAbstractButton::clicked,this,&TcpConnectWidget::previous);
  connect(cancelButton,&QAbstractButton::clicked,this,&TcpConnectWidget::cancel);
}

TcpConnectWidget::~TcpConnectWidget()
{
}

}

#include "moc_tcpconnectwidget.cpp"
