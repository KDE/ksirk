/***************************************************************************
                          KsirkChatItem.h  -  description
                             -------------------
    begin                : Mon Sep 26 2006
    copyright            : (C) 2006-2007 by Gael de Chalendar (aka Kleag)
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

#ifndef KSIRKCHATMODEL_H
#define KSIRKCHATMODEL_H

#include <qwidget.h>
#include <QListWidgetItem>
#include <qpushbutton.h>
#include <qcolor.h>
#include <QPixmap>

#include <kchatbasemodel.h>
#include "KsirkChatItem.h"
// namespace Ksirk
// {

// namespace GameLogic
// {



class KsirkChatModel : public KChatBaseModel
{
  Q_OBJECT

  public:
    KsirkChatModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    //      QVariant headerData(int section, Qt::Orientation orientation,
                                 //                          int role = Qt::DisplayRole) const;
    void addMessage(const KsirkChatItem& message);
    void addMessage(const QString& fromName, const QString& text);

  private:
    QList< KsirkChatItem > m_messages;
};

// } // closing namespace GameLogic

// } // closing namespace Ksirk


#endif // KSIRKCHATMODEL_H
