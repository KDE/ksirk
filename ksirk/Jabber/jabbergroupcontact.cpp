 /*
  * jabbercontact.cpp  -  Regular Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2004 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2006      by Olivier Goffart <ogoffart@kde.org>
  *
  * Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#include "jabbergroupcontact.h"
#include "jabber_protocol_debug.h"

#include <KLocalizedString>
#include <KFileDialog>
#include <KInputDialog>
#include <QAction>

#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberclient.h"
#include "jabberfiletransfer.h"
#include "jabbergroupchatmanager.h"
#include "jabbergroupmembercontact.h"
#include "jabbercontactpool.h"
#include "kopetemetacontact.h"
#include "xmpp_tasks.h"

/**
 * JabberGroupContact constructor
 */
JabberGroupContact::JabberGroupContact (const XMPP::RosterItem &rosterItem, JabberAccount *account, Kopete::MetaContact * mc)
	: JabberBaseContact ( XMPP::RosterItem ( rosterItem.jid().userHost () ), account, mc) , mNick( rosterItem.jid().resource() )
{
	setIcon( "jabber_group" );
	
	// initialize here, we need it set before we instantiate the manager below
	mManager = 0;

	setFileCapable ( false );

	/**
	 * Add our own nick as first subcontact (we need to do that here
	 * because we need to set this contact as myself() of the message
	 * manager).
	 */
	mSelfContact = addSubContact ( rosterItem );

	/**
	 * Instantiate a new message manager without members.
	 */
	mManager = new JabberGroupChatManager ( protocol (), mSelfContact,
											Kopete::ContactPtrList (), XMPP::Jid ( rosterItem.jid().userHost () ) );

	connect ( mManager, SIGNAL (closing(Kopete::ChatSession*)), this, SLOT (slotChatSessionDeleted()) );
	
	connect ( account->myself() , SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) ,
			  this , SLOT(slotStatusChanged()) ) ;

	/**
	 * FIXME: The first contact in the list of the message manager
	 * needs to be our own contact. This is a flaw in the Kopete
	 * API because it can't deal with groupchat properly.
	 * If we are alone in a room, we are myself() already and members()
	 * is empty. This makes at least the history plugin crash.
	 */
	mManager->addContact ( this );
	
	
	
	/**
	 * Let's construct the window:
	 *  otherwise, the ref count of maznager is equal to zero. 
	 *   and if we receive a message before the window is shown,
	 *   it will be deleted and we will be out of the channel
	 * In all case, there are no reason to don't show it.
	 */
	mManager->view( true , "kopete_chatwindow" );
}

JabberGroupContact::~JabberGroupContact ()
{

	qCDebug(JABBER_PROTOCOL_LOG) ;

	if(mManager) 
	{
		mManager->deleteLater();
	}
	
	foreach ( Kopete::Contact *contact,  mContactList )
	{
		/*if(mManager)
		mManager->removeContact( contact );*/
		qCDebug(JABBER_PROTOCOL_LOG) << "Deleting KC " << contact->contactId ();
		contact->deleteLater();
	}

	foreach ( Kopete::MetaContact *metaContact, mMetaContactList )
	{
		qCDebug(JABBER_PROTOCOL_LOG) << "Deleting KMC " << metaContact->metaContactId ();
		metaContact->deleteLater();
	}
}

QList<QAction *> *JabberGroupContact::customContextMenuActions ()
{
	QList<QAction *> *actionCollection = new QList<QAction*>();

	QAction *actionSetNick = new QAction(this);
	actionSetNick->setText( i18n ("Change nickname") );
	actionSetNick->setIcon( QIcon::fromTheme(QLatin1String( "jabber_changenick" )) );
	connect(actionSetNick, SIGNAL(triggered(bool)), this, SLOT(slotChangeNick()));

	actionCollection->append( actionSetNick );

	return actionCollection;
}

Kopete::ChatSession *JabberGroupContact::manager ( Kopete::Contact::CanCreateFlags canCreate )
{
	if(!mManager && canCreate == Kopete::Contact::CanCreate)
	{
		qCWarning(JABBER_PROTOCOL_LOG) << "somehow, the chat manager was removed, and the contact is still there";
		mManager = new JabberGroupChatManager ( protocol (), mSelfContact,
				Kopete::ContactPtrList (), XMPP::Jid ( rosterItem().jid().userHost() ) );

		mManager->addContact ( this );
		
		connect ( mManager, SIGNAL (closing(Kopete::ChatSession*)), this, SLOT (slotChatSessionDeleted()) );
		
		//if we have to recreate the manager, we probably have to connect again to the chat.
		slotStatusChanged();
	}
	return mManager;

}

void JabberGroupContact::handleIncomingMessage (const XMPP::Message & message)
{
	// message type is always chat in a groupchat
	QString viewType = "kopete_chatwindow";
	Kopete::Message *newMessage = 0L;
	
	qCDebug(JABBER_PROTOCOL_LOG) << "Received a message";

	/**
	 * Don't display empty messages, these were most likely just carrying
	 * event notifications or other payload.
	 */
	if ( message.body().isEmpty () )
		return;

	manager(CanCreate); //force to create mManager
	
	Kopete::ContactPtrList contactList = manager()->members();

	// check for errors
	if ( message.type () == "error" )
	{
		newMessage = new Kopete::Message( this, contactList );
		newMessage->setPlainBody( i18n("Your message could not be delivered: \"%1\", Reason: \"%2\"", 
										  message.body (), message.error().text ) );
		newMessage->setTimestamp( message.timeStamp() );
		newMessage->setSubject( message.subject() );
		newMessage->setDirection( Kopete::Message::Inbound );
		newMessage->setRequestedPlugin( viewType );
	}
	else
	{
		// retrieve and reformat body
		QString body = message.body ();

		if( !message.xencrypted().isEmpty () )
		{
			body = QString ("-----BEGIN PGP MESSAGE-----\n\n") + message.xencrypted () + QString ("\n-----END PGP MESSAGE-----\n");
		}

		// locate the originating contact
		JabberBaseContact *subContact = account()->contactPool()->findExactMatch ( message.from () );

		if ( !subContact )
		{
			qCWarning(JABBER_PROTOCOL_LOG) << "the contact is not in the list   : " <<  message.from().full();
			return;
			/**
			 * We couldn't find the contact for this message. That most likely means
			 * that it originated from a history backlog or something similar and
			 * the sending person is not in the channel anymore. We need to create
			 * a new contact for this which does not show up in the manager.
			 */
			subContact = addSubContact ( XMPP::RosterItem ( message.from () ), false );
		}

		// convert XMPP::Message into Kopete::Message
		newMessage = new Kopete::Message ( subContact, contactList );
		newMessage->setDirection( subContact != mManager->myself() ? Kopete::Message::Inbound : Kopete::Message::Outbound );
		newMessage->setTimestamp( message.timeStamp() );
		newMessage->setPlainBody( body );
		newMessage->setRequestedPlugin( viewType );
	}

	// append message to manager
	mManager->appendMessage ( *newMessage );

	delete newMessage;

}

JabberBaseContact *JabberGroupContact::addSubContact ( const XMPP::RosterItem &rosterItem, bool addToManager )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Adding new subcontact " << rosterItem.jid().full () << " to room " << mRosterItem.jid().full ();

	// see if this contact already exists, skip creation otherwise
	JabberBaseContact *subContact = dynamic_cast<JabberGroupMemberContact *>( account()->contactPool()->findExactMatch ( rosterItem.jid () ) );

	if ( subContact )
	{
		qCDebug(JABBER_PROTOCOL_LOG) << "Contact already exists, not adding again.";
		return subContact;
	}
	
	// Create new meta contact that holds the groupchat contact.
	Kopete::MetaContact *metaContact = new Kopete::MetaContact ();
	metaContact->setTemporary ( true );
	mMetaContactList.append ( metaContact );

	// now add contact to the pool, no dirty flag
	subContact = account()->contactPool()->addGroupContact ( rosterItem, false, metaContact, false );

	/**
	 * Add the contact to our message manager first. We need
	 * to check the pointer for validity, because this method
	 * gets called from the constructor, where the manager
	 * does not exist yet.
	 */
	if ( mManager && addToManager )
		mManager->addContact ( subContact );

	// now, add the contact also to our own list
	mContactList.append ( subContact );
	
	connect(subContact , SIGNAL(contactDestroyed(Kopete::Contact*)) , this , SLOT(slotSubContactDestroyed(Kopete::Contact*)));

	return subContact;

}

void JabberGroupContact::removeSubContact ( const XMPP::RosterItem &rosterItem )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Removing subcontact " << rosterItem.jid().full () << " from room " << mRosterItem.jid().full ();

	// make sure that subcontacts are only removed from the room contact, which has no resource
	if ( !mRosterItem.jid().resource().isEmpty () )
	{
		qCDebug(JABBER_PROTOCOL_LOG) << "WARNING: Trying to remove subcontact from subcontact!";
		return;
	}

	// find contact in the pool
	JabberGroupMemberContact *subContact = dynamic_cast<JabberGroupMemberContact *>( account()->contactPool()->findExactMatch ( rosterItem.jid () ) );

	if ( !subContact )
	{
		qCDebug(JABBER_PROTOCOL_LOG) << "WARNING: Subcontact couldn't be located!";
		return;
	}
	
	if(mManager && subContact->contactId() == mManager->myself()->contactId() )
	{
		//HACK WORKAROUND FIXME KDE4
		//impossible to remove myself, or we will die
		//subContact->setNickName( mNick ); //this is even worse than nothing
		return;
	}

	// remove the contact from the message manager first
	if(mManager)
		mManager->removeContact ( subContact );

	// remove the contact's meta contact from our internal list
	mMetaContactList.removeAll ( subContact->metaContact () );

	// remove the contact from our internal list
	mContactList.removeAll ( subContact );

	// delete the meta contact first
	delete subContact->metaContact ();

	// finally, delete the contact itself from the pool
	account()->contactPool()->removeContact ( rosterItem.jid () );

}

void JabberGroupContact::sendFile ( const KUrl &sourceURL, const QString &/*fileName*/, uint /*fileSize*/ )
{
	QString filePath;

	// if the file location is null, then get it from a file open dialog
	if ( !sourceURL.isValid () )
		filePath = KFileDialog::getOpenFileName( KUrl(), "*", 0L, i18n ( "Kopete File Transfer" ) );
	else
		filePath = sourceURL.path(KUrl::RemoveTrailingSlash);

	QFile file ( filePath );

	if ( file.exists () )
	{
		// send the file
		new JabberFileTransfer ( account (), this, filePath );
	}

}

void JabberGroupContact::slotChatSessionDeleted ()
{

	mManager = 0;

	if ( account()->isConnected () )
	{
		account()->client()->leaveGroupChat ( mRosterItem.jid().host (), mRosterItem.jid().user () );
	}
	
	//deleteLater(); //we will be deleted later when the account will know we have left

}

void JabberGroupContact::slotStatusChanged( )
{
	if( !account()->isConnected() )
	{
		//we need to remove all contact, because when we connect again, we will not receive the notificaion they are gone.
		QList<Kopete::Contact*> copy_contactlist=mContactList;
		foreach ( Kopete::Contact *contact, copy_contactlist )
		{
			removeSubContact( XMPP::Jid(contact->contactId()) );
		}
		return;
	}
	
	
	if( !isOnline() )
	{
		//HACK WORKAROUND   XMPP::client->d->groupChatList must contains us.
		account()->client()->joinGroupChat( rosterItem().jid().host() , rosterItem().jid().user() , mNick );
	}
	
	//TODO: away message
	XMPP::Status newStatus = account()->protocol()->kosToStatus( account()->myself()->onlineStatus() );
	account()->client()->setGroupChatStatus( rosterItem().jid().host() , rosterItem().jid().user() , newStatus );
}

void JabberGroupContact::slotChangeNick( )
{
	
	bool ok;
	QString futureNewNickName = KInputDialog::getText( i18n( "Change nickname - Jabber Plugin" ),
			i18n( "Please enter the new nickname you want to have in the room <i>%1</i>" , rosterItem().jid().userHost()),
			mNick, &ok );
	if ( !ok || !account()->isConnected())
		return;
	
	mNick=futureNewNickName;
	
	XMPP::Status status = account()->protocol()->kosToStatus( account()->myself()->onlineStatus() );
	account()->client()->changeGroupChatNick( rosterItem().jid().host() , rosterItem().jid().user()  , mNick , status);

}

void JabberGroupContact::slotSubContactDestroyed( Kopete::Contact * deadContact )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "cleaning dead subcontact " << deadContact->contactId() << " from room " << mRosterItem.jid().full ();

	mMetaContactList.removeAll ( deadContact->metaContact () );
	mContactList.removeAll ( deadContact );

}

#include "moc_jabbergroupcontact.cpp"
