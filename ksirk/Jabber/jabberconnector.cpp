
/***************************************************************************
                   jabberconnector.cpp  -  Socket Connector for Jabber
                             -------------------
    begin                : Wed Jul 7 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

			   Kopete (C) 2004 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "jabber_protocol_debug.h"
#include "jabberconnector.h"
#include "jabberbytestream.h"
// #include "jabberprotocol.h"

JabberConnector::JabberConnector ( QObject *parent )
 : XMPP::Connector ( parent )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "New Jabber connector.";

	mErrorCode = 0;

	mByteStream = new JabberByteStream ( this );

	connect ( mByteStream, &JabberByteStream::connected, this, &JabberConnector::slotConnected );
	connect ( mByteStream, &ByteStream::error, this, &JabberConnector::slotError );

}

JabberConnector::~JabberConnector ()
{

	delete mByteStream;

}

void JabberConnector::connectToServer ( const QString &server )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Initiating connection to " << server;

	/*
	 * FIXME: we should use a SRV lookup to determine the
	 * actual server to connect to. As this is currently
	 * not supported yet, we're using setOptHostPort().
	 * For XMPP 1.0, we need to enable this!
	 */

	mErrorCode = 0;

  qCDebug(JABBER_PROTOCOL_LOG) << mHost << mPort;
  mByteStream->connect ( mHost, mPort );

}

void JabberConnector::slotConnected ()
{
	qCDebug(JABBER_PROTOCOL_LOG) << "We are connected.";

	// FIXME: setPeerAddress() is something different, find out correct usage later
	//KInetSocketAddress inetAddress = mStreamSocket->address().asInet().makeIPv6 ();
	//setPeerAddress ( QHostAddress ( inetAddress.ipAddress().addr () ), inetAddress.port () );

	emit connected ();

}

void JabberConnector::slotError ( int code )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Error detected: " << code;

	mErrorCode = code;
	emit error ();

}

int JabberConnector::errorCode ()
{

	return mErrorCode;

}

ByteStream *JabberConnector::stream () const
{

	return mByteStream;

}

void JabberConnector::done ()
{

	mByteStream->close ();

}

void JabberConnector::setOptHostPort ( const QString &host, quint16 port )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Manually specifying host " << host << " and port " << port;

	mHost = host;
	mPort = port;

}

void JabberConnector::setOptSSL ( bool ssl )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Setting SSL to " << ssl;

	setUseSSL ( ssl );

}

void JabberConnector::setOptProbe ( bool )
{
	// FIXME: Implement this.
}


