/*
 * privacylist.cpp
 * Copyright (C) 2006  Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#include <algorithm>
 
#include <QDomElement>
#include <QStringList>

#include "jabber_protocol_debug.h"
// #include "jabberprotocol.h"

#include "privacylist.h"

#define ORDER_INCREMENT 10

PrivacyList::PrivacyList(const QString& name, const QList<PrivacyListItem>& items) : name_(name), items_(items) 
{ 
	std::sort(items_.begin(), items_.end());
}

PrivacyList::PrivacyList(const QDomElement& e)
{
	fromXml(e);
}

void PrivacyList::updateItem(int index, const PrivacyListItem& item) 
{
	unsigned int order = items_[index].order();
	items_[index] = item;
	items_[index].setOrder(order);
}

void PrivacyList::insertItem(int index, const PrivacyListItem& item) 
{ 
	items_.insert(index,item); 

	reNumber();
}

void PrivacyList::reNumber() 
{
	unsigned int order = 100;
	for (int i = 0; i < items_.size(); ++i) {
    	items_[i].setOrder(order);
		order += ORDER_INCREMENT;
	}
}


bool PrivacyList::moveItemUp(int index)
{
	if (index < items().count() && index > 0) {
		unsigned int order =items_[index].order();
		if (order == items_[index-1].order()) {
			reNumber();
			return true;
		}
		items_[index].setOrder(items_[index-1].order());
		items_[index-1].setOrder(order);
		items_.swapItemsAt(index,index-1);
		return true;
	}
	else {
		return false;
	}
}

bool PrivacyList::moveItemDown(int index)
{
	if (index >= 0 && index < items().count()-1) {
		unsigned int order =items_[index].order();
		if (order == items_[index+1].order()) {
			reNumber();
			return true;
		}
		items_[index].setOrder(items_[index+1].order());
		items_[index+1].setOrder(order);
		items_.swapItemsAt(index,index+1);
		return true;
	}
	else {
		return false;
	}
}

bool PrivacyList::onlyBlockItems() const
{
	bool allBlocked = true;
	bool fallThrough = false;
	QList<PrivacyListItem>::ConstIterator it;
    for (it = items_.begin(); it != items_.end() && allBlocked; ++it ) {
		if ((*it).type() == PrivacyListItem::FallthroughType && (*it).action() == PrivacyListItem::Allow && (*it).all()) {
			fallThrough = true;
		}
		else if ((*it).isBlock()) {
			if (fallThrough) 
				allBlocked = false;
		}
		else {
			allBlocked = false;
		}
	}
	return allBlocked;
}

QDomElement PrivacyList::toXml(QDomDocument& doc) const
{
	QDomElement list = doc.createElement("list");
	list.setAttribute("name",name()); 
	
	for (QList<PrivacyListItem>::ConstIterator it = items_.begin() ; it != items_.end(); it++) {
		list.appendChild((*it).toXml(doc));
	}

	return list;
}

void PrivacyList::fromXml(const QDomElement& el)
{
	//qCDebug(JABBER_PROTOCOL_LOG) << "Parsing privacy list";
	if (el.isNull() || el.tagName() != "list") {
		qCWarning(JABBER_PROTOCOL_LOG) << "Invalid root tag for privacy list.";
		return;
	}

	setName(el.attribute("name"));
	for(QDomNode n = el.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement e = n.toElement();
		if (!e.isNull())
			items_.append(PrivacyListItem(e));
	}

	std::sort(items_.begin(), items_.end());
}

QString PrivacyList::toString() const
{
	QString s;
	for (QList<PrivacyListItem>::ConstIterator it = items_.begin() ; it != items_.end(); it++) {
		s += QString("%1 (%2)\n").arg((*it).toString()).arg((*it).order());
	}
	return s;
}
