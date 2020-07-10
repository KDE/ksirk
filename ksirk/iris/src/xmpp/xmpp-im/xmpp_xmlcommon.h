/*
 * xmlcommon.h - helper functions for dealing with XML
 * Copyright (C) 2001, 2002  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * either version 2
   of the License, or (at your option) any later version.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef JABBER_XMLCOMMON_H
#define JABBER_XMLCOMMON_H

#include <qdom.h>
#include <QList>

#include "iris_export.h"

class QDateTime;
class QRect;
class QSize;
class QColor;
class QStringList;

class IRIS_EXPORT XDomNodeList
{
public:
	XDomNodeList();
	XDomNodeList(const XDomNodeList &from);
	XDomNodeList(const QDomNodeList &from);
	~XDomNodeList();
	XDomNodeList & operator=(const XDomNodeList &from);

	QDomNode at(int index) const { return item(index); }
	int count() const { return (int)length(); }
	bool isEmpty() const;
	QDomNode item(int index) const;
	uint length() const;
	int size() const { return (int)length(); }

	void append(const QDomNode &i);

	bool operator==(const XDomNodeList &a) const;

	bool operator!=(const XDomNodeList &a) const
	{
		return !operator==(a);
	}

private:
	QList<QDomNode> list;
};

QDateTime IRIS_EXPORT stamp2TS(const QString &ts);
bool IRIS_EXPORT stamp2TS(const QString &ts, QDateTime *d);
QString IRIS_EXPORT TS2stamp(const QDateTime &d);
QDomElement IRIS_EXPORT textTag(QDomDocument *doc, const QString &name, const QString &content);
QString IRIS_EXPORT tagContent(const QDomElement &e);
QDomElement IRIS_EXPORT findSubTag(const QDomElement &e, const QString &name, bool *found);
XDomNodeList IRIS_EXPORT childElementsByTagNameNS(const QDomElement &e, const QString &nsURI, const QString &localName);
QDomElement IRIS_EXPORT createIQ(QDomDocument *doc, const QString &type, const QString &to, const QString &id);
QDomElement IRIS_EXPORT queryTag(const QDomElement &e);
QString IRIS_EXPORT queryNS(const QDomElement &e);
void IRIS_EXPORT getErrorFromElement(const QDomElement &e, const QString &baseNS, int *code, QString *str);
QDomElement IRIS_EXPORT addCorrectNS(const QDomElement &e);

namespace XMLHelper {
	//QDomElement findSubTag(const QDomElement &e, const QString &name, bool *found);
  bool IRIS_EXPORT hasSubTag(const QDomElement &e, const QString &name);

  QDomElement IRIS_EXPORT emptyTag(QDomDocument *doc, const QString &name);
  QString IRIS_EXPORT subTagText(const QDomElement &e, const QString &name);

  QDomElement IRIS_EXPORT textTag(QDomDocument &doc, const QString &name, const QString &content);
  QDomElement IRIS_EXPORT textTag(QDomDocument &doc, const QString &name, int content);
  QDomElement IRIS_EXPORT textTag(QDomDocument &doc, const QString &name, bool content);
  QDomElement IRIS_EXPORT textTag(QDomDocument &doc, const QString &name, QSize &s);
  QDomElement IRIS_EXPORT textTag(QDomDocument &doc, const QString &name, QRect &r);
  QDomElement IRIS_EXPORT stringListToXml(QDomDocument &doc, const QString &name, const QStringList &l);

  void IRIS_EXPORT readEntry(const QDomElement &e, const QString &name, QString *v);
  void IRIS_EXPORT readNumEntry(const QDomElement &e, const QString &name, int *v);
  void IRIS_EXPORT readBoolEntry(const QDomElement &e, const QString &name, bool *v);
  void IRIS_EXPORT readSizeEntry(const QDomElement &e, const QString &name, QSize *v);
  void IRIS_EXPORT readRectEntry(const QDomElement &e, const QString &name, QRect *v);
  void IRIS_EXPORT readColorEntry(const QDomElement &e, const QString &name, QColor *v);

  void IRIS_EXPORT xmlToStringList(const QDomElement &e, const QString &name, QStringList *v);

  void IRIS_EXPORT setBoolAttribute(QDomElement e, const QString &name, bool b);
  void IRIS_EXPORT readBoolAttribute(QDomElement e, const QString &name, bool *v);

	//QString tagContent(const QDomElement &e); // obsolete;
}

#endif
