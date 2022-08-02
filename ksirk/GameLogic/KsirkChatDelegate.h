/***************************************************************************
                          KsirkChatItem.h  -  description
                             -------------------
    begin                : Mon Sep 26 2006
    copyright            : (C) 2006-2007 by Gaël de Chalendar (aka Kleag)
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

#ifndef KSIRKCHATDELEGATE_H
#define KSIRKCHATDELEGATE_H

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kchatbaseitemdelegate.h>

class KsirkChatDelegate : public KChatBaseItemDelegate
{
    Q_OBJECT

public:
    explicit KsirkChatDelegate(QObject *parent = nullptr);
    ~KsirkChatDelegate() override {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index, const QString& sender,
                            const QString& message) const override;
};

#endif // KSIRKCHATDELEGATE_H
