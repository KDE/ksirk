/*
    This file is part of the KDE games library
    Copyright (C) 2001 Burkhard Lehner (Burkhard.Lehner@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License either version 2
   of the License, or (at your option) any later version.as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

/*
  KMessageJabber, subclass of KMessageIO
*/

#ifndef KMESSAGEJABBER_H
#define KMESSAGEJABBER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QHostAddress>
#include "jabber_protocol_debug.h"

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgame/kmessageio.h>

#include "jabberclient.h"


/**
  \class KMessageJabber kmessageio.h <KGame/KMessageIO>
  
  This class implements the message communication using a TCP/IP socket. The
  object can connect to a server socket, or can use an already connected socket.
*/

class KMessageJabber : public KMessageIO
{
  Q_OBJECT

public:
  /**
    Connects to a server socket on /e host with /e port. host can be an
    numerical (e.g. "192.168.0.212") or symbolic (e.g. "wave.peter.org")
    IP address. You can immediately use the /e sendSystem() and
    /e sendBroadcast() methods. The messages are stored and sent to the
    receiver after the connection is established.

    If the connection could not be established (e.g. unknown host or no server
    socket at this port), the signal /e connectionBroken is emitted.
  */
  explicit KMessageJabber (const QString& peerJid, JabberClient* jabberClient, QObject *parent = 0 );

  /**
    Destructor, closes the Jabber connexion.
  */
  ~KMessageJabber () override;

  /**
  * The runtime idendifcation
  */
  int rtti() const override {return 3;}

  /**
    @return The jid this object is connected to. See QSocket::peerName.
  */
  QString peerName () const override;

  /**
    @return TRUE as this is a network IO.
  */
  bool isNetwork() const override { return true; }

  /**
    Returns true if the socket is in state /e connected.
  */
  bool isConnected () const override;

  /**
    Overwritten slot method from KMessageIO.

    Note: It is not declared as a slot method, since the slot is already
    defined in KMessageIO as a virtual method.
  */
  void send (const QByteArray &msg) override;

protected Q_SLOTS:
  virtual void slotMessageReceived ( const XMPP::Message &message );
  virtual void slotGroupChatLeft(const XMPP::Message&);
  virtual void slotResourceUnavailable(const Jid&, const Resource&);
  virtual void slotGroupChatPresence(const XMPP::Jid&, const XMPP::Status&);
  
protected:
  JabberClient* mClient;
  bool mAwaitingHeader;
  quint32 mNextBlockLength;
  QString mPeerJid;
};

#endif
