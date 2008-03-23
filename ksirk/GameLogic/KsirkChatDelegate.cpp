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

#include "KsirkChatDelegate.h"
#include "KsirkChatItem.h"

#include <kdebug.h>
#include <QPainter>

KsirkChatDelegate::KsirkChatDelegate(QObject *parent) : 
    KChatBaseItemDelegate(parent)
{
//   kDebug() << "KsirkChatDelegate::KsirkChatDelegate" << endl;
}

void KsirkChatDelegate::paint(QPainter *painter, 
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const
{
//   kDebug() << "KsirkChatDelegate::paint" << endl;
  KsirkChatItem m  = index.model()->data(index, Qt::DisplayRole).value<KsirkChatItem>();
  if (!m.first.isEmpty())
  {
//     kDebug() << "  " <<m.first << " / " << m.second << endl;
    KChatBaseItemDelegate::paint(painter, option, index, m.first, m.second);
  }
  else
  {
    m.paint(painter, option,index.row());
  }
}

QSize KsirkChatDelegate::sizeHint(const QStyleOptionViewItem &  option ,
        const QModelIndex &  index ) const
{
//   kDebug() << "KsirkChatDelegate::sizeHint" << endl;
  KsirkChatItem m  = index.model()->data(index, Qt::DisplayRole).value<KsirkChatItem>();
  if (!m.first.isEmpty())
  {
    return KChatBaseItemDelegate::sizeHint(option, index, m.first, m.second);
  }
  QSize result = m.sizeHint(option);
//   kDebug() << "KsirkChatDelegate::sizeHint: " << result << endl;
  return result;
}

QSize KsirkChatDelegate::sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index, const QString& sender,
                            const QString& message) const
{
  return KChatBaseItemDelegate::sizeHint(option,index,sender,message);
}

#include "KsirkChatDelegate.moc"
