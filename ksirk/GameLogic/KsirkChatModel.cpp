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
#include "KsirkChatModel.h"
#include "player.h"

#include <QLayout>
#include <QApplication>
#include <QPainter>
#include <QColorDialog>
#include <QInputDialog>
#include <QPixmap>
namespace Ksirk
{
   class KGameWindow;
}

// namespace GameLogic
// {


KsirkChatModel::KsirkChatModel(QObject* parent, Ksirk::KGameWindow* game)
     : KChatBaseModel(parent), m_messages(), m_game(game)
{
}

int KsirkChatModel::rowCount(const QModelIndex &parent) const
{
//   kDebug() << "KsirkChatModel::rowCount" << endl;
  if (parent.isValid())
    return 0;
  else
    return m_messages.size();
}

QVariant KsirkChatModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role == Qt::DisplayRole)
  {
    KsirkChatItem p = m_messages[index.row()];
    return QVariant::fromValue(p);
  }
  return QVariant();
}

void KsirkChatModel::addMessage(const KsirkChatItem& message)
{
  QString msg;
  if ((QString(message.first)).length() == 0) {
     msg = "No message...";
  } else {
     msg = "<b>"+message.first+":</b> "+message.second;
     if (msg.length() > 77) {
        msg.resize(64);
        msg += " ...";
     }
  }
  m_game->titleChatMessage()->setText(msg);

  int row;
  row = m_messages.size();
  beginInsertRows(QModelIndex(), row, row);
  m_messages.push_back(message);
  endInsertRows();
}

void KsirkChatModel::addMessage(const QString& fromName, const QString& text)
{
  addMessage(KsirkChatItem(fromName,text));
}


#include "KsirkChatModel.moc"

// } // closing namespace GameLogic

// } // closing namespace Ksirk

