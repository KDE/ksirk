 /*
  *   jabberprotocol.cpp  -  Base class for the Kopete Jabber protocol
  *
  * Copyright (c) 2002-2003 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
  * Copyright (c) 2006      by Olivier Goffart  <ogoffart at kde.org>
  *
  *  Kopete   (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kconfig.h>

#include <qmap.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <QList>

#include "im.h"
#include "xmpp.h"

#include <sys/utsname.h>

// #include "kopetecontact.h"
// #include "kopetecontactlist.h"
// #include "kopetemetacontact.h"
// #include "kopetechatsession.h"
// #include "kopeteonlinestatusmanager.h"
// #include "kopeteglobal.h"
// #include "kopeteprotocol.h"
// #include "kopeteplugin.h"
// #include "kopeteaccountmanager.h"
// #include "addcontactpage.h"
// #include "kopetecommandhandler.h"

#include "jabbercontact.h"
#include "jabberaddcontactpage.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabbereditaccountwidget.h"
#include "jabbercapabilitiesmanager.h"
#include "jabbertransport.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"
#include "dlgregister.h"

JabberProtocol *JabberProtocol::protocolInstance = 0;

K_PLUGIN_FACTORY( JabberProtocolFactory, registerPlugin<JabberProtocol>(); )
K_EXPORT_PLUGIN( JabberProtocolFactory( "kopete_jabber" ) )

JabberProtocol::JabberProtocol (QObject * parent)
: QObject(parent)
{

	kDebug (JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Loading ...";

	/* This is meant to be a singleton, so we will check if we have
	 * been loaded before. */
	if (protocolInstance)
	{
		kDebug (JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Warning: Protocol already " << "loaded, not initializing again.";
		return;
	}

	protocolInstance = this;

	// Init the Entity Capabilities manager.
	capsManager = new JabberCapabilitiesManager;
	capsManager->loadCachedInformation();
	
	registerAsProtocolHandler(QString::fromLatin1("xmpp"));
}

JabberProtocol::~JabberProtocol ()
{
	//disconnectAll();

	delete capsManager;
	capsManager = 0L;

	/* make sure that the next attempt to load Jabber
	 * re-initializes the protocol class. */
	protocolInstance = 0L;
}



AddContactPage *JabberProtocol::createAddContactWidget (QWidget * parent, Kopete::Account * i)
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Create Add Contact  Widget";
	return new JabberAddContactPage (i, parent);
}

KopeteEditAccountWidget *JabberProtocol::createEditAccountWidget (Kopete::Account * account, QWidget * parent)
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Edit Account Widget";
	JabberAccount *ja=dynamic_cast < JabberAccount * >(account);
	if(ja || !account)
		return new JabberEditAccountWidget (this,ja , parent);
	else
	{
		JabberTransport *transport = dynamic_cast < JabberTransport * >(account);
		if(!transport || !transport->account()->client() )
			return 0L;
		dlgRegister *registerDialog = new dlgRegister (transport->account(), transport->myself()->contactId());
		registerDialog->show (); 
		registerDialog->raise ();
		return 0l; //we make ourself our own dialog, not an editAccountWidget.
	}
}

Kopete::Account *JabberProtocol::createNewAccount (const QString & accountId)
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Create New Account. ID: " << accountId;
	if( Kopete::AccountManager::self()->findAccount( pluginId() , accountId ) )
		return 0L;  //the account may already exist if greated just above

	int slash=accountId.indexOf('/');
	if(slash>=0)
	{
		QString realAccountId=accountId.left(slash);
		JabberAccount *realAccount=dynamic_cast<JabberAccount*>(Kopete::AccountManager::self()->findAccount( pluginId() , realAccountId ));
		if(!realAccount) //if it doesn't exist yet, create it
		{
			realAccount = new JabberAccount( this, realAccountId );
			if(!Kopete::AccountManager::self()->registerAccount(  realAccount ) )
				return 0L;
		}
		if(!realAccount)
			return 0L;
		return new JabberTransport( realAccount , accountId );
	}
	else
	{
		return new JabberAccount (this, accountId);
	}
}

Kopete::OnlineStatus JabberProtocol::resourceToKOS ( const XMPP::Resource &resource )
{

	// update to offline by default
	Kopete::OnlineStatus status = JabberKOSOffline;

	if ( !resource.status().isAvailable () )
	{
		// resource is offline
		status = JabberKOSOffline;
	}
	else
	{
		if (resource.status ().show ().isEmpty ())
		{
			if (resource.status ().isInvisible ())
			{
				status = JabberKOSInvisible;
			}
			else
			{
				status = JabberKOSOnline;
			}
		}
		else
		if (resource.status ().show () == "chat")
		{
			status = JabberKOSChatty;
		}
		else if (resource.status ().show () == "away")
		{
			status = JabberKOSAway;
		}
		else if (resource.status ().show () == "xa")
		{
			status = JabberKOSXA;
		}
		else if (resource.status ().show () == "dnd")
		{
			status = JabberKOSDND;
		}
		else if (resource.status ().show () == "online")
		{ // the ApaSMSAgent sms gateway report status as "online" even if it's not in the RFC 3921 � 2.2.2.1 
			// See Bug 129059
			status = JabberKOSOnline;
		}
		else if (resource.status ().show () == "connecting")
		{ // this is for kopete internals
			status = JabberKOSConnecting;
		}
		else
		{
			kDebug (JABBER_DEBUG_GLOBAL) << "Unknown status <show>" << resource.status ().show () << "</show> for contact. One of your contact is probably using a broken client, ask him to report a bug";
		}
	}

	return status;

}

JabberCapabilitiesManager *JabberProtocol::capabilitiesManager()
{
	return capsManager;
}

JabberProtocol *JabberProtocol::protocol ()
{
	// return current instance
	return protocolInstance;
}

Kopete::Contact *JabberProtocol::deserializeContact (Kopete::MetaContact * metaContact,
										 const QMap < QString, QString > &serializedData, const QMap < QString, QString > & /* addressBookData */ )
{
//  kDebug (JABBER_DEBUG_GLOBAL) << "Deserializing data for metacontact " << metaContact->displayName () << "\n";

	QString contactId = serializedData["contactId"];
	QString displayName = serializedData["displayName"];
	QString accountId = serializedData["accountId"];
	QString jid = serializedData["JID"];

	QList<Kopete::Account*> accounts = Kopete::AccountManager::self ()->accounts (this);
	Kopete::Account *account = 0;
	QList<Kopete::Account*>::Iterator accountIt, accountItEnd = accounts.end();
	for(accountIt = accounts.begin(); accountIt != accountItEnd; ++accountIt)
	{
		if((*accountIt)->accountId() == accountId)
			account = *accountIt;
	}

	if (!account)
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "WARNING: Account for contact does not exist, skipping.";
		return 0;
	}
	
	JabberTransport *transport = dynamic_cast<JabberTransport*>(account);
	if( transport )
		transport->account()->addContact ( jid.isEmpty() ? contactId : jid ,  metaContact);
	else
		account->addContact (contactId,  metaContact);
	return account->contacts()[contactId];
}

XMPP::Status JabberProtocol::kosToStatus( const Kopete::OnlineStatus & status , const QString & message )
{
	XMPP::Status xmppStatus ( "", message );

	if( status.status() == Kopete::OnlineStatus::Offline )
	{
		xmppStatus.setIsAvailable( false );
	}

	switch ( status.internalStatus () )
	{
		case JabberProtocol::JabberFreeForChat:
			xmppStatus.setShow ( "chat" );
			break;

		case JabberProtocol::JabberOnline:
			xmppStatus.setShow ( "" );
			break;

		case JabberProtocol::JabberAway:
			xmppStatus.setShow ( "away" );
			break;

		case JabberProtocol::JabberXA:
			xmppStatus.setShow ( "xa" );
			break;

		case JabberProtocol::JabberDND:
			xmppStatus.setShow ( "dnd" );
			break;

		case JabberProtocol::JabberInvisible:
			xmppStatus.setIsInvisible ( true );
			break;
	}
	return xmppStatus;
}

#include <accountselector.h>
#include <kopeteuiglobal.h>
#include <kvbox.h>
#include "jabbercontactpool.h"
#include <kopeteview.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

void JabberProtocol::handleURL(const KUrl & kurl) const
{
	QUrl url=kurl; //QUrl has better query handling.
	if(url.scheme() != "xmpp" && !url.scheme().isEmpty() )
		return;

	url.setQueryDelimiters( '=' , ';' );
	QString accountid=url.authority();
	QString jid_str=url.path();
	if(jid_str.startsWith('/'))
		jid_str=jid_str.mid(1);
	XMPP::Jid jid = jid_str;
	QString action=url.queryItems().isEmpty() ? QString() : url.queryItems().first().first;
	 
	kDebug() << url.queryItemValue("body");

	if(jid.isEmpty())
	{
		return;
	}
	
	JabberAccount *account=0L;
	if(!accountid.isEmpty())
	{
		account=static_cast<JabberAccount*>(Kopete::AccountManager::self()->findAccount("JabberProtocol" , accountid));
	}
	if(!account)
	{
		QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts(const_cast<JabberProtocol*>(this));
		if (accounts.count() == 1)
			account = static_cast<JabberAccount*>(accounts.first());
		else
		{
			KDialog chooser(Kopete::UI::Global::mainWidget());
			chooser.setCaption( i18n("Choose Account") );
			chooser.setButtons( KDialog::Ok | KDialog::Cancel );
			chooser.setDefaultButton(KDialog::Ok);
			KVBox vb(&chooser);
			chooser.setMainWidget(&vb);
			QLabel label(&vb);
			label.setText(i18n("Choose an account to handle the URL %1" , kurl.prettyUrl()));
//			label.setSizePolicy(QSizePolicy::Minimum , QSizePolicy::MinimumExpanding);
			label.setWordWrap(true);
			AccountSelector accSelector(const_cast<JabberProtocol*>(this), &vb);
	//		accSelector.setSizePolicy(QSizePolicy::MinimumExpanding , QSizePolicy::MinimumExpanding);
			int ret = chooser.exec();
			account=qobject_cast<JabberAccount*>(accSelector.selectedItem());
			if (ret == QDialog::Rejected || account == 0)
				return;
		}
	}
	
	if(jid.isEmpty())
	{
		return;
	}
	
	JabberBaseContact *contact=account->contactPool()->findRelevantRecipient( jid );
		
	if(action.isEmpty() || action=="message")
	{
		if(!contact)
		{
			Kopete::MetaContact *metaContact = new Kopete::MetaContact ();
			metaContact->setTemporary (true);
			contact = account->contactPool()->addContact ( XMPP::RosterItem ( jid ), metaContact, false );
			Kopete::ContactList::self()->addMetaContact(metaContact);
		}
		contact->execute();
		
		if(url.hasQueryItem("body") || url.hasQueryItem("subject"))
		{ //TODO "thread"
			Kopete::ChatSession *kcs=contact->manager(Kopete::Contact::CanCreate);
			if(!kcs)
				return;

			Kopete::Message msg(account->myself(), kcs->members());
			msg.setPlainBody( url.queryItemValue("body") );
			msg.setSubject( url.queryItemValue("subject") );
			msg.setDirection( Kopete::Message::Outbound );

			KopeteView *kv=kcs->view(true);
			if(kv)
				kv->setCurrentMessage(msg);
		}
	}
	else if(action == "roster" || action == "subscribe")
	{
		if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
			i18n("Do you want to add '%1' to your contact list?", jid.full()),
			QString(), KGuiItem( i18n("Add") ), KGuiItem( i18n("Do Not Add") ))
				  != KMessageBox::Yes)
		{
			return;
		}
		Kopete::Group *group=0L;
		if( url.hasQueryItem("group"))
			group=Kopete::ContactList::self()->findGroup( url.queryItemValue("group") );
		account->addContact( jid.full() ,  url.queryItemValue("name") , group );
	}
	else if(action == "remove" || action== "unsubscribe")
	{
		if(!contact)
			return;
	
		if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
			i18n("Do you want to remove '%1' from your contact list?", jid.full()),
			QString(), KGuiItem( i18n("Remove") ), KGuiItem( i18n("Do Not Remove") ))
						!= KMessageBox::Yes)
		{
			return;
		}
		
		contact->deleteContact();
	}//TODO: probe
	else if(action == "join" || action=="invite")
	{
		if(!account->isConnected() || !account->client())
		{
			account->errorConnectFirst();
			return;
		}
		
		if(!contact)
		{
			QString nick=jid.resource();
			if(nick.isEmpty())
			{
				bool ok=true;
				nick = KInputDialog::getText(i18n("Please enter your nickname for the room %1", jid.bare()),
						i18n("Provide your nickname"),
						QString(),
						&ok);
				if (!ok)
					return;
			}
			if(action=="join" && url.hasQueryItem("password"))
				account->client()->joinGroupChat( jid.host() , jid.user() , nick, url.queryItemValue("password") );
			else
				account->client()->joinGroupChat( jid.host() , jid.user() , nick );
		}
		
		if(action=="invite" && url.hasQueryItem("jid") )
		{
			//NOTE: this is the obsolete, NOT RECOMMENDED protocol.
			//      iris doesn't implement groupchat yet
			//NOTE: This code is duplicated in JabberGroupChatManager::inviteContact
			XMPP::Message jabberMessage;
			XMPP::Jid jid = static_cast<const JabberBaseContact*>(account->myself())->rosterItem().jid() ;
			jabberMessage.setFrom ( jid );
			jabberMessage.setTo ( url.queryItemValue("jid") );
			jabberMessage.setInvite( jid.bare() );
			jabberMessage.setBody( i18n("You have been invited to %1", jid.bare() ) );

			// send the message
			account->client()->sendMessage ( jabberMessage );
		}
	}
	else if(action=="sendfile")
	{
		if(!contact)
		{
			Kopete::MetaContact *metaContact = new Kopete::MetaContact ();
			metaContact->setTemporary (true);
			contact = account->contactPool()->addContact ( XMPP::RosterItem ( jid ), metaContact, false );
			Kopete::ContactList::self()->addMetaContact(metaContact);
		}
		contact->sendFile();
	}//TODO: recvfile
	else
	{
		kWarning(JABBER_DEBUG_GLOBAL) << "unable to handle URL "<< kurl.prettyUrl();
	}

}

#include "jabberprotocol.moc"
