/*
 * privacymanager.cpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "xmpp_xmlcommon.h"
#include "xmpp_jid.h"
#include "privacymanager.h"

#include <kdebug.h>
// #include "jabberprotocol.h"

#define PRIVACY_NS "jabber:iq:privacy"

using namespace XMPP;

// -----------------------------------------------------------------------------
//

PrivacyListListener::PrivacyListListener ( Task* parent ) : Task ( parent ) {
}

bool PrivacyListListener::take ( const QDomElement &e ) {
	if ( e.tagName() != "iq" || e.attribute ( "type" ) != "set" )
		return false;

	QString ns = queryNS ( e );
	if ( ns == "jabber:iq:privacy" ) {
		// TODO: Do something with update

		// Confirm receipt
		QDomElement iq = createIQ ( doc(), "result", e.attribute ( "from" ), e.attribute ( "id" ) );
		send ( iq );
		return true;
	}

	return false;
}

// -----------------------------------------------------------------------------

GetPrivacyListsTask::GetPrivacyListsTask ( Task* parent ) : Task ( parent ) {
	iq_ = createIQ ( doc(), "get", "", id() );
	QDomElement query = doc()->createElement ( "query" );
	query.setAttribute ( "xmlns",PRIVACY_NS );
	iq_.appendChild ( query );
}

void GetPrivacyListsTask::onGo() {
	send ( iq_ );
}

bool GetPrivacyListsTask::take ( const QDomElement &x ) {
	if ( !iqVerify ( x, "", id() ) )
		return false;

	//kDebug() << "Got reply for privacy lists.";
	if ( x.attribute ( "type" ) == "result" ) {
		QDomElement tag, q = queryTag ( x );

		for ( QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling() ) {
			QDomElement e = n.toElement();
			if ( e.tagName() == "active" )
				active_ = e.attribute ( "name" );
			else if ( e.tagName() == "default" )
				default_ = e.attribute ( "name" );
			else if ( e.tagName() == "list" )
				lists_.append ( e.attribute ( "name" ) );
			else
				kWarning () << "Unknown tag in privacy lists.";

		}
		setSuccess();
	}
	else {
		setError ( x );
	}
	return true;
}

const QStringList& GetPrivacyListsTask::lists() {
	return lists_;
}

const QString& GetPrivacyListsTask::defaultList() {
	return default_;
}

const QString& GetPrivacyListsTask::activeList() {
	return active_;
}

// -----------------------------------------------------------------------------


SetPrivacyListsTask::SetPrivacyListsTask ( Task* parent ) : Task ( parent ), changeDefault_ ( false ), changeActive_ ( false ), changeList_ ( false ), list_ ( "" ) {
}

void SetPrivacyListsTask::onGo() {
	QDomElement iq_ = createIQ ( doc(), "set", "", id() );
	QDomElement query = doc()->createElement ( "query" );
	query.setAttribute ( "xmlns",PRIVACY_NS );
	iq_.appendChild ( query );

	QDomElement e;
	if ( changeDefault_ ) {
		//kDebug() << "Changing default privacy list.";
		e = doc()->createElement ( "default" );
		if ( !value_.isEmpty() )
			e.setAttribute ( "name",value_ );
	}
	else if ( changeActive_ ) {
		//kDebug() << "Changing active privacy list.";
		e = doc()->createElement ( "active" );
		if ( !value_.isEmpty() )
			e.setAttribute ( "name",value_ );
	}
	else if ( changeList_ ) {
		//kDebug() << "Changing privacy list.";
		e = list_.toXml ( *doc() );
	}
	else {
		kWarning () << "Empty active/default list change request.";
		return;
	}

	query.appendChild ( e );
	send ( iq_ );
}

void SetPrivacyListsTask::setActive ( const QString& active ) {
	value_ = active;
	changeDefault_ = false;
	changeActive_ = true;
	changeList_ = false;
}

void SetPrivacyListsTask::setDefault ( const QString& d ) {
	value_ = d;
	changeDefault_ = true;
	changeActive_ = false;
	changeList_ = true;
}

void SetPrivacyListsTask::setList ( const PrivacyList& list ) {
	//kDebug() << "setList: " << list.toString();
	list_ = list;
	changeDefault_ = false;
	changeActive_ = false;
	changeList_ = true;
}

bool SetPrivacyListsTask::take ( const QDomElement &x ) {
	if ( !iqVerify ( x, "", id() ) )
		return false;

	if ( x.attribute ( "type" ) == "result" ) {
		//kDebug() << "Got successful reply for list change.";
		setSuccess();
	}
	else {
		kWarning () << "Got error reply for list change.";
		setError ( x );
	}
	return true;
}


// -----------------------------------------------------------------------------

GetPrivacyListTask::GetPrivacyListTask ( Task* parent, const QString& name ) : Task ( parent ), name_ ( name ), list_ ( PrivacyList ( "" ) ) {
	iq_ = createIQ ( doc(), "get", "", id() );
	QDomElement query = doc()->createElement ( "query" );
	query.setAttribute ( "xmlns",PRIVACY_NS );
	iq_.appendChild ( query );
	QDomElement list = doc()->createElement ( "list" );
	list.setAttribute ( "name",name );
	query.appendChild ( list );
}

void GetPrivacyListTask::onGo() {
	//kDebug() << "privacy.cpp: Requesting privacy list %1." << name_;
	send ( iq_ );
}

bool GetPrivacyListTask::take ( const QDomElement &x ) {
	if ( !iqVerify ( x, "", id() ) )
		return false;

	//kDebug() << qPrintable (QString("Got privacy list %1 reply.").arg(name_));
	if ( x.attribute ( "type" ) == "result" ) {
		QDomElement q = queryTag ( x );
		bool found;
		QDomElement listTag = findSubTag ( q,"list",&found );
		if ( found ) {
			list_ = PrivacyList ( listTag );
		}
		else {
			kWarning () << "No valid list found.";
		}
		setSuccess();
	}
	else {
		setError ( x );
	}
	return true;
}

const PrivacyList& GetPrivacyListTask::list() {
	return list_;
}


// -----------------------------------------------------------------------------

PrivacyManager::PrivacyManager ( XMPP::Task* rootTask ) : rootTask_ ( rootTask ), getDefault_waiting_ ( false ), block_waiting_ ( false )
{
	listener_ = new PrivacyListListener ( rootTask_ );
}

PrivacyManager::~PrivacyManager()
{
	delete listener_;
}

void PrivacyManager::requestListNames()
{
	GetPrivacyListsTask* t = new GetPrivacyListsTask ( rootTask_ );
	connect ( t,SIGNAL ( finished() ),SLOT ( receiveLists() ) );
	t->go ( true );
}

void PrivacyManager::requestList ( const QString& name )
{
	GetPrivacyListTask* t = new GetPrivacyListTask ( rootTask_, name );
	connect ( t,SIGNAL ( finished() ),SLOT ( receiveList() ) );
	t->go ( true );
}

void PrivacyManager::block ( const QString& target )
{
	block_targets_.push_back ( target );
	if ( !block_waiting_ ) {
		block_waiting_ = true;
		connect ( this,SIGNAL ( defaultListAvailable ( const PrivacyList& ) ),SLOT ( block_getDefaultList_success ( const PrivacyList& ) ) );
		connect ( this,SIGNAL ( defaultListError() ),SLOT ( block_getDefaultList_error() ) );
		getDefaultList();
	}
}

void PrivacyManager::block_getDefaultList_success ( const PrivacyList& l_ )
{
	PrivacyList l = l_;
	disconnect ( this,SIGNAL ( defaultListAvailable ( const PrivacyList& ) ),this,SLOT ( block_getDefault_success ( const PrivacyList& ) ) );
	disconnect ( this,SIGNAL ( defaultListError() ),this,SLOT ( block_getDefault_error() ) );
	block_waiting_ = false;
	while ( !block_targets_.isEmpty() )
		l.insertItem ( 0,PrivacyListItem::blockItem ( block_targets_.takeFirst() ) );
	changeList ( l );
}

void PrivacyManager::block_getDefaultList_error()
{
	disconnect ( this,SIGNAL ( defaultListAvailable ( const PrivacyList& ) ),this,SLOT ( block_getDefault_success ( const PrivacyList& ) ) );
	disconnect ( this,SIGNAL ( defaultListError() ),this,SLOT ( block_getDefault_error() ) );
	block_waiting_ = false;
	block_targets_.clear();
}

void PrivacyManager::getDefaultList()
{
	connect ( this,SIGNAL ( listsReceived ( const QString&, const QString&, const QStringList& ) ),SLOT ( getDefault_listsReceived ( const QString&, const QString&, const QStringList& ) ) );
	connect ( this,SIGNAL ( listsError() ),SLOT ( getDefault_listsError() ) );
	requestListNames();
}

void PrivacyManager::getDefault_listsReceived ( const QString& defaultList, const QString&, const QStringList& )
{
	disconnect ( this,SIGNAL ( listsReceived ( const QString&, const QString&, const QStringList& ) ),this,SLOT ( getDefault_listsReceived ( const QString&, const QString&, const QStringList& ) ) );
	disconnect ( this,SIGNAL ( listsError() ),this,SLOT ( getDefault_listsError() ) );

	getDefault_default_ = defaultList;
	if ( !defaultList.isEmpty() ) {
		getDefault_waiting_ = true;
		connect ( this,SIGNAL ( listReceived ( const PrivacyList& ) ),SLOT ( getDefault_listReceived ( const PrivacyList& ) ) );
		connect ( this,SIGNAL ( listError() ),SLOT ( getDefault_listError() ) );
		requestList ( defaultList );
	}
	else {
		emit defaultListAvailable ( PrivacyList ( "" ) );
	}
}

void PrivacyManager::getDefault_listsError()
{
	disconnect ( this,SIGNAL ( listsReceived ( const QString&, const QString&, const QStringList& ) ),this,SLOT ( getDefault_listsReceived ( const QString&, const QString&, const QStringList& ) ) );
	disconnect ( this,SIGNAL ( listsError() ),this,SLOT ( getDefault_listsError() ) );
	emit defaultListError();
}

void PrivacyManager::getDefault_listReceived ( const PrivacyList& l )
{
	if ( l.name() == getDefault_default_ && getDefault_waiting_ ) {
		disconnect ( this,SIGNAL ( listReceived ( const PrivacyList& ) ),this,SLOT ( getDefault_listReceived ( const PrivacyList& ) ) );
		disconnect ( this,SIGNAL ( listError() ),this,SLOT ( getDefault_listError() ) );
		getDefault_waiting_ = false;
		emit defaultListAvailable ( l );
	}
}

void PrivacyManager::getDefault_listError()
{
	emit defaultListError();
}

void PrivacyManager::changeDefaultList ( const QString& name )
{
	SetPrivacyListsTask* t = new SetPrivacyListsTask ( rootTask_ );
	t->setDefault ( name );
	connect ( t,SIGNAL ( finished() ),SLOT ( changeDefaultList_finished() ) );
	t->go ( true );
}

void PrivacyManager::changeDefaultList_finished()
{
	SetPrivacyListsTask *t = ( SetPrivacyListsTask* ) sender();
	if ( !t ) {
		kWarning () << "Unexpected sender.";
		return;
	}

	if ( t->success() ) {
		emit changeDefaultList_success();
	}
	else {
		emit changeDefaultList_error();
	}
}

void PrivacyManager::changeActiveList ( const QString& name )
{
	SetPrivacyListsTask* t = new SetPrivacyListsTask ( rootTask_ );
	t->setActive ( name );
	connect ( t,SIGNAL ( finished() ),SLOT ( changeActiveList_finished() ) );
	t->go ( true );
}

void PrivacyManager::changeActiveList_finished()
{
	SetPrivacyListsTask *t = ( SetPrivacyListsTask* ) sender();
	if ( !t ) {
		kWarning () << "Unexpected sender.";
		return;
	}

	if ( t->success() ) {
		emit changeActiveList_success();
	}
	else {
		emit changeActiveList_error();
	}
}

void PrivacyManager::changeList ( const PrivacyList& list )
{
	SetPrivacyListsTask* t = new SetPrivacyListsTask ( rootTask_ );
	t->setList ( list );
	connect ( t,SIGNAL ( finished() ),SLOT ( changeList_finished() ) );
	t->go ( true );
}

void PrivacyManager::changeList_finished()
{
	SetPrivacyListsTask *t = ( SetPrivacyListsTask* ) sender();
	if ( !t ) {
		kWarning () << "Unexpected sender.";
		return;
	}

	if ( t->success() ) {
		emit changeList_success();
	}
	else {
		emit changeList_error();
	}
}

void PrivacyManager::receiveLists()
{
	GetPrivacyListsTask *t = ( GetPrivacyListsTask* ) sender();
	if ( !t ) {
		kWarning () << "Unexpected sender.";
		return;
	}

	if ( t->success() ) {
		emit listsReceived ( t->defaultList(),t->activeList(),t->lists() );
	}
	else {
		kDebug() << "Error in lists receiving.";
		emit listsError();
	}
}

void PrivacyManager::receiveList()
{
	GetPrivacyListTask *t = ( GetPrivacyListTask* ) sender();
	if ( !t ) {
		kDebug() << "Unexpected sender.";
		return;
	}

	if ( t->success() ) {
		emit listReceived ( t->list() );
	}
	else {
		kDebug() << "Error in list receiving.";
		emit listError();
	}
}

