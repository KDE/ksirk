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
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSIRKCHATDELEGATE_H
#define KSIRKCHATDELEGATE_H

#include <kchatbase.h>
#include <kchatbaseitemdelegate.h>

class KsirkChatModel;

class KsirkChatDelegate : public KChatBaseItemDelegate
{
    Q_OBJECT

public:
    explicit KsirkChatDelegate(QObject *parent = 0);
    virtual ~KsirkChatDelegate() {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;

    virtual KChatBaseModel* model();

private:
  KsirkChatModel* m_model;
};

#endif // KSIRKCHATDELEGATE_H
