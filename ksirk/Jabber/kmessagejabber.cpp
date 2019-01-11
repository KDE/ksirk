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
  subclass KMessageJabber of KMessageIO
*/

#include "jabber_protocol_debug.h"
#include "kmessagejabber.h"
#include <QDataStream>
#include <QUuid>

// ----------------------KMessageJabber -----------------------

KMessageJabber::KMessageJabber (const QString& peerJid, JabberClient* jabberClient, QObject *parent)
  : KMessageIO (parent),
  mClient(jabberClient),
  mPeerJid(peerJid)
{
  qCDebug(JABBER_PROTOCOL_LOG) << peerJid;
//   initSocket ();
  connect(jabberClient, &JabberClient::messageReceived, this, &KMessageJabber::slotMessageReceived);
         
  connect(jabberClient, &JabberClient::resourceUnavailable, this, &KMessageJabber::slotResourceUnavailable);
                  
  connect(jabberClient, SIGNAL(groupChatLeft(XMPP::Jid)), this, SLOT(slotGroupChatLeft(XMPP::Message)));

  connect(jabberClient, &JabberClient::groupChatPresence, this, &KMessageJabber::slotGroupChatPresence);

  
  
}

KMessageJabber::~KMessageJabber ()
{
//   delete mClient;
}

bool KMessageJabber::isConnected () const
{
  return mClient->isConnected ();
}

void KMessageJabber::send (const QByteArray &msg)
{
  XMPP::Message message(mPeerJid);
  message.setType("ksirkgame");
  message.setId(QUuid::createUuid().toString().remove("{").remove("}").remove("-"));
  message.setBody(msg.toBase64());
  mClient->sendMessage(message);
}

void KMessageJabber::slotMessageReceived(const XMPP::Message &message)
{
  if (message.from().full() == mPeerJid)
  {
    QByteArray msg = QByteArray::fromBase64(message.body().toLatin1());
    qCDebug(JABBER_PROTOCOL_LOG) << mPeerJid << msg;
    emit received (msg);
  }
}

QString KMessageJabber::peerName () const
{
  return mPeerJid;
}

void KMessageJabber::slotGroupChatLeft(const XMPP::Message& msg)
{
  qCDebug(JABBER_PROTOCOL_LOG) << msg.from().full() << msg.to().full() << msg.body();
}

void KMessageJabber::slotResourceUnavailable(const Jid& jid, const Resource& resource)
{
  qCDebug(JABBER_PROTOCOL_LOG) << jid.full() << resource.name();
}

void KMessageJabber::slotGroupChatPresence(const XMPP::Jid& jid, const XMPP::Status& status)
{
  qCDebug(JABBER_PROTOCOL_LOG) << jid.full() << status.status();
  if (jid.full() == mPeerJid && !status.isAvailable())
  {
    emit connectionBroken();
  }
}


