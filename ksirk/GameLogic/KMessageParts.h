/***************************************************************************
                          kstringvector.h  -  description
                             -------------------
    begin                : Mon Sep 26 2005
    copyright            : (C) 2005-2007 by Gael de Chalendar (aka Kleag)
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

#ifndef KSIRK_GAMELOGICKMESSAGEPARTS_H
#define KSIRK_GAMELOGICKMESSAGEPARTS_H

#include <QList>
#include <QString>
#include <QPixmap>

namespace Ksirk {

namespace GameLogic {

/**
  * This class holds a serie of strings and pixmaps that have to be transferred
  * on the network and displayed in a specified order.
  * 
  * It stores one collection of strings and one collection of pixmaps, each of 
  * the same size. When an item at position i is a string the corresponding 
  * pixmap is an empty one and conversely.
  * @author Gaël de Chalendar
  */
class KMessageParts
{
public:
  /** The possible types of the elements. */
  enum ElemType {Text,Pixmap,StringId};

  /** Inner class to iterate on the message parts with the various standard 
    * functions of a STL iterator */
  class iterator
  {
    friend class KMessageParts;
  public:
    iterator& operator++(int)
    {
      m_pixmaps_it++;
      m_strings_it++;
      m_order_it++;
      return *this;
    }
    bool operator==(const iterator& it)
    {
      return ( (m_pixmaps_it == it.m_pixmaps_it)
              && (m_strings_it == it.m_strings_it)
              && (m_order_it == it.m_order_it) );
    }
    bool operator!=(const iterator& it)
    {
      return !((*this)==it);
    }
    iterator& operator=(const iterator& it)
    {
      m_pixmaps_it = it.m_pixmaps_it;
      m_strings_it = it.m_strings_it;
      m_order_it = it.m_order_it;
      return *this;
    }
    QPixmap& curPix() {return *m_pixmaps_it;}
    QString& curStr() {return *m_strings_it;}
    bool curIsPix() { return (*m_order_it == Pixmap);}
    bool curIsStr() { return (*m_order_it == Text);}
    
  private:
    QList<QPixmap>::iterator m_pixmaps_it;
    QList<QString>::iterator m_strings_it;
    QList<ElemType>::iterator m_order_it;
  };
  friend class iterator;

  /** Default constructor */
  KMessageParts();

  /** Default destructor */
  ~KMessageParts();

  //@{
  /** push and pop operators for strings */
  KMessageParts& operator<<(const QString& s);
  KMessageParts& operator>>(QString& s);
  //@}

  //@{
  /** push and pop operators for pixmaps */
  KMessageParts& operator<<(const QPixmap& s);
  KMessageParts& operator>>(QPixmap& s);
  //@}

  //@{
  /** Testers of the type of the next item */
  bool nextIsText();
  bool nextIsPixmap();
  //@}

  /** 
    * Tester of the parts collection emptiness
    * @return true if this collection is empty and false otherwise 
    */
  bool empty();

  /** 
    * Tester of the parts collection size.
    * @return The size of the collection.
    */
  unsigned int size();

  /** Removes all elements from this collection */
  void clear();

  //@{
  /** Returns iterators on the first and past-the-end elements of this 
    * collection */
  iterator begin();
  iterator end();
  //@}

private:
  QList<QPixmap> m_pixmaps;
  QList<QString> m_strings;
  QList<ElemType> m_order;
};

} // closing namespace GameLogic

} // closing namespace Ksirk

#endif
