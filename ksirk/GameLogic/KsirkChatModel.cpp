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
#include <qlayout.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qcolordialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <QPixmap>
#include "KsirkChatModel.h"
#include "player.h"

// namespace Ksirk
// {

// namespace GameLogic
// {


KsirkChatModel::KsirkChatModel(QObject *parent)
     : KChatBaseModel(parent), m_messages()
{
}

int KsirkChatModel::rowCount(const QModelIndex &parent) const
{
  kDebug() << "KsirkChatModel::rowCount" << endl;
  if (parent.isValid())
    return 0;
  else
    return m_messages.size();
}

QVariant KsirkChatModel::data(const QModelIndex &index, int role) const
{
  kDebug() << "KsirkChatModel::data " << index.row() << " ; m_messages size=" << m_messages.size() << endl;
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
  kDebug() << "KsirkChatModel::addMessage" << endl;
  int row;
  row = m_messages.size();
  beginInsertRows(QModelIndex(), row, row);
  m_messages.push_back(message);
  endInsertRows();
}

void KsirkChatModel::addMessage(const QString& fromName, const QString& text)
{
  int row;
  row = m_messages.size();
  beginInsertRows(QModelIndex(), row, row);
  m_messages.push_back(KsirkChatItem(fromName,text));
  endInsertRows();
}


#include "KsirkChatModel.moc"

// } // closing namespace GameLogic

// } // closing namespace Ksirk

