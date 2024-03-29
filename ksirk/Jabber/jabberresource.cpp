 /*
  * jabberresource.cpp
  *
  * Copyright (c) 2005-2006 by Michaël Larouche <larouche@kde.org>
  * Copyright (c) 2004 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) 2001-2006 by the Kopete developers  <kopete-devel@kde.org>
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

#include "jabberresource.h"

// Qt includes
#include <QTimer>

// KDE includes
#include "jabber_protocol_debug.h"

// libiris includes
#include <im.h>
#include <xmpp_tasks.h>

// Kopete includes
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabbercapabilitiesmanager.h"

class JabberResource::Private
{
public:
	Private( JabberAccount *t_account, const XMPP::Jid &t_jid, const XMPP::Resource &t_resource )
	 : account(t_account), jid(t_jid), resource(t_resource), capsEnabled(false)
	{
		// Make sure the resource is always set.
		jid.setResource(resource.name());
	}

	JabberAccount *account;
	XMPP::Jid jid;
	XMPP::Resource resource;
	
	QString clientName, clientSystem;
	XMPP::Features supportedFeatures;
	bool capsEnabled;
};

JabberResource::JabberResource ( JabberAccount *account, const XMPP::Jid &jid, const XMPP::Resource &resource )
	: d( new Private(account, jid, resource) )
{
	d->capsEnabled = account->protocol()->capabilitiesManager()->capabilitiesEnabled(jid);

	if ( account->isConnected () )
	{
		QTimer::singleShot ( account->client()->getPenaltyTime () * 1000, this, SLOT (slotGetTimedClientVersion()) );
		if(!d->capsEnabled)
		{
			QTimer::singleShot ( account->client()->getPenaltyTime () * 1000, this, SLOT (slotGetDiscoCapabilties()) );
		}
	}
}

JabberResource::~JabberResource ()
{
	delete d;
}

const XMPP::Jid &JabberResource::jid () const
{
	return d->jid;
}

const XMPP::Resource &JabberResource::resource () const
{
	return d->resource;
}

void JabberResource::setResource ( const XMPP::Resource &resource )
{
	d->resource = resource;

	// Check if the caps are now available.
	d->capsEnabled = d->account->protocol()->capabilitiesManager()->capabilitiesEnabled(d->jid);

	emit updated( this );
}

const QString &JabberResource::clientName () const
{
	return d->clientName;
}

const QString &JabberResource::clientSystem () const
{
	return d->clientSystem;
}

XMPP::Features JabberResource::features() const
{
	if(d->capsEnabled)
	{
		return d->account->protocol()->capabilitiesManager()->features(d->jid);
	}
	else
	{
		return d->supportedFeatures;
	}
}

void JabberResource::slotGetTimedClientVersion ()
{
	if ( d->account->isConnected () )
	{
		qCDebug(JABBER_PROTOCOL_LOG) << "Requesting client version for " << d->jid.full ();

		// request client version
		XMPP::JT_ClientVersion *task = new XMPP::JT_ClientVersion ( d->account->client()->rootTask () );
		// signal to ourselves when the vCard data arrived
		QObject::connect ( task, SIGNAL (finished()), this, SLOT (slotGotClientVersion()) );
		task->get ( d->jid );
		task->go ( true );
	}
}

void JabberResource::slotGotClientVersion ()
{
	XMPP::JT_ClientVersion *clientVersion = (XMPP::JT_ClientVersion *) sender ();

	if ( clientVersion->success () )
	{
		d->clientName = clientVersion->name () + ' ' + clientVersion->version ();
		d->clientSystem = clientVersion->os ();

		emit updated ( this );
	}
}

void JabberResource:: slotGetDiscoCapabilties ()
{
	if ( d->account->isConnected () )
	{
		qCDebug(JABBER_PROTOCOL_LOG) << "Requesting Client Features for " << d->jid.full ();

		XMPP:: JT_DiscoInfo *task = new XMPP::JT_DiscoInfo ( d->account->client()->rootTask () );
		// Retrive features when service discovery is done.
		QObject::connect ( task, SIGNAL (finished()), this, SLOT (slotGotDiscoCapabilities()) );
		task->get ( d->jid);
		task->go ( true );
	}
}

void JabberResource::slotGotDiscoCapabilities ()
{
	XMPP::JT_DiscoInfo *discoInfo = (XMPP::JT_DiscoInfo *) sender ();

	if ( discoInfo->success () )
	{
		d->supportedFeatures = discoInfo->item().features();
		
		emit updated ( this );
	}
}

#include "moc_jabberresource.cpp"
