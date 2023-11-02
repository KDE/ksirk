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
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

#ifndef KSIRKCHATMODEL_H
#define KSIRKCHATMODEL_H

#include <QWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QColor>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kchatbasemodel.h>

#include "KsirkChatItem.h"
#include "../kgamewin.h"
namespace Ksirk
{
   class KGameWindow;
}

// namespace GameLogic
// {



class KsirkChatModel : public KChatBaseModel
{
  Q_OBJECT

  public:
    KsirkChatModel(QObject *parent, Ksirk::KGameWindow* game);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    //      QVariant headerData(int section, Qt::Orientation orientation,
                                 //                          int role = Qt::DisplayRole) const;
    void addMessage(const KsirkChatItem& message);
    void addMessage(const QString& fromName, const QString& text) override;

  private:
    QList< KsirkChatItem > m_messages;
    Ksirk::KGameWindow* m_game;
};

// } // closing namespace GameLogic

// } // closing namespace Ksirk


#endif // KSIRKCHATMODEL_H
