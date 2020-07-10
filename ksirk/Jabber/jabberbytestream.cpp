
/***************************************************************************
                   jabberbytestream.cpp  -  Byte Stream for Jabber
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

#include <QObject>
#include "jabber_protocol_debug.h"
#include "jabberbytestream.h"
// #include "jabberprotocol.h"

JabberByteStream::JabberByteStream ( QObject *parent )
 : ByteStream ( parent )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Instantiating new Jabber byte stream.";

	// reset close tracking flag
	mClosing = false;

	mSocket = nullptr;
}

void JabberByteStream::connect ( QString host, int port )
{
	qCDebug(JABBER_PROTOCOL_LOG) << Q_FUNC_INFO << "Connecting to " << host << ", port " << port ;

	mClosing = false;

	mSocket = std::make_shared<QTcpSocket>(new QTcpSocket());
  mSocket->connectToHost(host, port);

	QObject::connect ( mSocket.get(), SIGNAL (error(QAbstractSocket::SocketError)), this, SLOT (slotError(QAbstractSocket::SocketError)) );
	QObject::connect ( mSocket.get(), &QAbstractSocket::connected, this, &JabberByteStream::slotConnected );
	QObject::connect ( mSocket.get(), &QAbstractSocket::disconnected, this, &JabberByteStream::slotConnectionClosed );
	QObject::connect ( mSocket.get(), &QIODevice::readyRead, this, &JabberByteStream::slotReadyRead );
	QObject::connect ( mSocket.get(), &QIODevice::bytesWritten, this, &JabberByteStream::slotBytesWritten );
}

bool JabberByteStream::isOpen () const
{

	// determine if socket is open
	return socket()->isOpen ();

}

void JabberByteStream::close ()
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Closing stream.";

	// close the socket and set flag that we are closing it ourselves
	mClosing = true;
        if (mSocket) {
             qCDebug(JABBER_PROTOCOL_LOG) << Q_FUNC_INFO << "socket is not null";
	     mSocket->close();
             qCDebug(JABBER_PROTOCOL_LOG) << Q_FUNC_INFO << "socket closed";
             mSocket=nullptr;
        }
}

int JabberByteStream::tryWrite ()
{

	// send all data from the buffers to the socket
	QByteArray writeData = takeWrite();
	socket()->write ( writeData.data (), writeData.size () );

	return writeData.size ();

}

QTcpSocket *JabberByteStream::socket () const
{

	return mSocket.get();

}

JabberByteStream::~JabberByteStream ()
{

}

void JabberByteStream::slotConnected ()
{

	emit connected ();

}

void JabberByteStream::slotConnectionClosed ()
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Socket has been closed.";

	// depending on who closed the socket, emit different signals
	if ( !mClosing )
	{
		emit connectionClosed ();
	}
	else
	{
		emit delayedCloseFinished ();
	}

	mClosing = false;

}

void JabberByteStream::slotReadyRead ()
{
	qCDebug(JABBER_PROTOCOL_LOG) << "called:  available: " << socket()->bytesAvailable ();
	appendRead ( socket()->readAll() );

	emit readyRead ();

}

void JabberByteStream::slotBytesWritten ( qint64 bytes )
{

	emit bytesWritten ( bytes );

}

void JabberByteStream::slotError ( QAbstractSocket::SocketError code )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "Socket error '" <<  mSocket->errorString() <<  "' - Code : " << code;
	emit error ( code );
}


