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

#ifndef KSIRKCHATITEM_H
#define KSIRKCHATITEM_H

#include <QWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QColor>
#include <QPixmap>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kchatbasemodel.h>

/**
  * This is the class of the items displayed in the chat dialog at the bottom
  * of the KsirK window. This is a special listbox item able to display several
  * strings and pixmaps and build using stream-like operators
  */
class KsirkChatItem : public KChatBaseMessage
{
public:
  /** The kind of elements displayable. */
  enum ElemType {Text,Pixmap,BaseMessage};

  /** 
    * Default constructor
    */
  KsirkChatItem ();

  /** 
    */
  KsirkChatItem (const QString& name, const QString& message);

  /** 
    * Default destructor
    */
  ~KsirkChatItem() override {}

  KsirkChatItem(const KsirkChatItem& item);

  //@{
  /** Stream-like operators to add text and pixmap elements */
  KsirkChatItem& operator<<(const QString& text);
  KsirkChatItem& operator<<(const QPixmap& pixmap);
  //@}

  /** Reimplementation to paint text and pixmap items from left to right in the
    * correct order */
  virtual void paint(QPainter* p, const QStyleOptionViewItem &option, int row);

  QSize sizeHint(const QStyleOptionViewItem &option);

private:
  QList<QPixmap> m_pixmaps;
  QList<QString> m_strings;
  QList<ElemType> m_order;
} ; // class KsirkChatItem
Q_DECLARE_METATYPE(KsirkChatItem)

#endif // KSIRKCHATITEM_H
