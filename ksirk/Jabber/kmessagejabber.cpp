/*
    This file is part of the KDE games library
    Copyright (C) 2001 Burkhard Lehner (Burkhard.Lehner@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

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

#include "kmessagejabber.h"
#include <QDataStream>
#include <QUuid>

// ----------------------KMessageJabber -----------------------

KMessageJabber::KMessageJabber (const QString& peerJid, JabberClient* jabberClient, QObject *parent)
  : KMessageIO (parent),
  mClient(jabberClient),
  mPeerJid(peerJid)
{
  kDebug() << peerJid;
//   initSocket ();
  connect(jabberClient, SIGNAL(messageReceived(const XMPP::Message&)), this, SLOT(slotMessageReceived(const XMPP::Message&)));
         
  connect(jabberClient, SIGNAL(resourceUnavailable (const Jid&, const Resource&)), this, SLOT(slotResourceUnavailable(const Jid&, const Resource&)));
                  
  connect(jabberClient, SIGNAL(groupChatLeft(const XMPP::Jid&)), this, SLOT(slotGroupChatLeft(const XMPP::Message&)));

  connect(jabberClient, SIGNAL(groupChatPresence(const XMPP::Jid&, const XMPP::Status&)), this, SLOT(slotGroupChatPresence(const XMPP::Jid&, const XMPP::Status&)));

  
  
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
    QByteArray msg = QByteArray::fromBase64(message.body().toAscii());
    kDebug() << mPeerJid << msg;
    emit received (msg);
  }
}

QString KMessageJabber::peerName () const
{
  return mPeerJid;
}

void KMessageJabber::slotGroupChatLeft(const XMPP::Message& msg)
{
  kDebug() << msg.from().full() << msg.to().full() << msg.body();
}

void KMessageJabber::slotResourceUnavailable(const Jid& jid, const Resource& resource)
{
  kDebug() << jid.full();
}

void KMessageJabber::slotGroupChatPresence(const XMPP::Jid& jid, const XMPP::Status& status)
{
  kDebug() << jid.full() << status.status();
  if (jid.full() == mPeerJid && !status.isAvailable())
  {
    emit connectionBroken();
  }
}

#include "kmessagejabber.moc"
