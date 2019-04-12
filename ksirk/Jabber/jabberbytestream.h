
/***************************************************************************
                   jabberbytestream.h  -  Byte Stream for Jabber
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

#ifndef JABBERBYTESTREAM_H
#define JABBERBYTESTREAM_H

#include <bytestream.h>
#include <QTcpSocket>
#include <memory>

/**
@author Kopete Developers
*/
class JabberByteStream : public ByteStream
{

Q_OBJECT

public:
	JabberByteStream ( QObject *parent = 0 );

	~JabberByteStream () override;

	void connect ( QString host, int port );
	bool isOpen () const override;
	void close () override;

	QTcpSocket *socket () const;

signals:
	void connected ();

protected:
	int tryWrite () override;

private slots:
	void slotConnected ();
	void slotConnectionClosed ();
	void slotReadyRead ();
	void slotBytesWritten ( qint64 );
  void slotError ( QAbstractSocket::SocketError );

private:
	std::shared_ptr<QTcpSocket> mSocket;
	bool mClosing;

};

#endif
