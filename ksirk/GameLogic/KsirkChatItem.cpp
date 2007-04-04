/***************************************************************************
                          KsirkChatItem.cpp  -  description
                             -------------------
    begin                : Mon Sep 26 2006
    copyright            : (C) 2006-2007 by Gaël de Chalendar (aka Kleag)
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

#include "KsirkChatItem.h"
#include "player.h"

#include <qlayout.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qcolordialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <QPixmap>
#include <QTextDocument>
#include <iostream>

KsirkChatItem::KsirkChatItem () : KChatBaseMessage()
{ 
}

KsirkChatItem::KsirkChatItem(const QString& sender, const QString& message): KChatBaseMessage(sender,message)
{
}

KsirkChatItem::KsirkChatItem(const KsirkChatItem& item) : KChatBaseMessage(item),
  m_pixmaps(item.m_pixmaps),
  m_strings(item.m_strings),
  m_order(item.m_order)

{
}

void KsirkChatItem::paint(QPainter* p, 
                const QStyleOptionViewItem &option, int row)
{
//   std::cerr << "KsirkChatItem::paint" << std::endl;
  Q_UNUSED(row);

  QTextDocument fake; // used to allow to compute lines height
  fake.setHtml("gpl");
  fake.setDefaultFont(option.font);
  fake.adjustSize();
  fake.setTextWidth ( -1 );
  
  qreal h = fake.size().height();
  unsigned int x = 0;
  for (int i = 0 ; i < m_order.size(); i++)
  {
    QTextDocument rt;
    rt.setHtml(m_strings[i]);
    rt.setDefaultFont(option.font);
    rt.adjustSize();
    rt.setTextWidth ( -1 );
    QPixmap px(rt.size().toSize());
    px.fill();
    QPainter pa(&px);
    rt.drawContents(&pa);
    switch (m_order[i])
    {
    case Text:
//       kDebug() << "  paint string '" << m_strings[i] << "' at " << x << ", " << row*h << endl;
      p->drawPixmap(option.rect.x()+x,option.rect.y(),px);
      x += px.width();
    break;
    case Pixmap:
      if (! m_pixmaps[i].isNull())
      {
//         kDebug() << "  paint pixmap at " << x << ", " << row*h << endl;
        QPixmap scaled = m_pixmaps[i].scaledToHeight((int)h);
        p->drawPixmap(option.rect.x()+x,option.rect.y(),scaled);
        x+= scaled.width();
      }
    break;
    default: ;
    }
  }
}

QSize KsirkChatItem::sizeHint(const QStyleOptionViewItem &option)
{
  unsigned int w = 0;
  QTextDocument fake; // used to allow to compute lines height
  fake.setHtml("gpl");
  fake.setDefaultFont(option.font);
  fake.adjustSize();
  fake.setTextWidth ( -1 );
  
  qreal h = fake.size().height();
  for (int i = 0 ; i < m_order.size(); i++)
  {
    QTextDocument rt;
    rt.setHtml(m_strings[i]);
    rt.adjustSize();
    rt.setTextWidth ( -1 );
    QPixmap px(rt.size().toSize());
    switch (m_order[i])
    {
    case Text:
      w += px.width();
    break;
    case Pixmap:
      if (! m_pixmaps[i].isNull())
      {
        QPixmap scaled = m_pixmaps[i].scaledToHeight((int)h);
         w+= scaled.width();
      }
    break;
    default: ;
    }
  }
//   kDebug() << "KsirkChatItem::sizeHint: " << QSize(w,h) << endl;
  return QSize(w,(int)h);
}

KsirkChatItem& KsirkChatItem::operator<<(const QString& text)
{
  m_strings.push_back(text);
  m_pixmaps.push_back(QPixmap ());
  m_order.push_back(Text);
  return *this;
}

KsirkChatItem& KsirkChatItem::operator<<(const QPixmap& pixmap)
{
  m_strings.push_back("");
  m_pixmaps.push_back(pixmap);
  m_order.push_back(Pixmap);
  return *this;
}
