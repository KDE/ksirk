
/***************************************************************************
                jabberclient.cpp - Generic Jabber Client Class
                             -------------------
    begin                : Sat May 25 2005
    copyright            : (C) 2005 by Till Gerken <till@tantalo.net>
                           (C) 2006 by Michaël Larouche <larouche@kde.org>
    Copyright 2006 by Tommi Rantala <tommi.rantala@cs.helsinki.fi>

         Kopete (C) 2001-2006 Kopete developers
         <kopete-devel@kde.org>.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabberclient.h"
#include "jabber_protocol_debug.h"

#include <QTimer>
#include <QRegExp>

#include <bsocket.h>
#include <filetransfer.h>
#include <xmpp_tasks.h>

#include "jabberconnector.h"
#include "privacymanager.h"

#define JABBER_PENALTY_TIME  2

using namespace XMPP;

class JabberClient::Private
{
public:
  Private()
  : jabberClient(nullptr), jabberClientStream(nullptr), jabberClientConnector(nullptr), jabberTLS(nullptr),
           jabberTLSHandler(nullptr), privacyManager(nullptr)
  {}
  ~Private()
  {
    if ( jabberClient )
    {
      jabberClient->close ();
    }
    
    delete jabberClient;
    delete jabberClientStream;
    delete jabberClientConnector;
    delete jabberTLSHandler;
    delete jabberTLS;
    // privacyManager will be deleted with jabberClient, its parent's parent
  }

  // connection details
  XMPP::Jid jid;
  QString password;

  // XMPP backend
  XMPP::Client *jabberClient;
  XMPP::ClientStream *jabberClientStream;
  JabberConnector *jabberClientConnector;
  QCA::TLS *jabberTLS;
  XMPP::QCATLSHandler *jabberTLSHandler;
  QCA::Initializer qcaInit;
  PrivacyManager *privacyManager;

  // ignore TLS warnings
  bool ignoreTLSWarnings;

  // current S5B server instance
  static XMPP::S5BServer *s5bServer;
  // address list being handled by the S5B server instance
  static QStringList s5bAddressList;
  // port of S5B server
  static int s5bServerPort;

  // local IP address
  QString localAddress;

  // whether TLS (or direct SSL in case of the old protocol) should be used
  bool forceTLS;

  // whether direct SSL connections should be used
  bool useSSL;

  // use XMPP 1.0 or the older protocol version
  bool useXMPP09;

  // whether SSL support should be probed in case the old protocol is used
  bool probeSSL;

  // override the default server name and port (only pre-XMPP 1.0)
  bool overrideHost;
  QString server;
  int port;

  // allow transmission of plaintext passwords
  bool allowPlainTextPassword;

  // enable file transfers
  bool fileTransfersEnabled;

  // current penalty time
  int currentPenaltyTime;

  // client information
  QString clientName, clientVersion, osName;

  // timezone information
  QString timeZoneName;
  int timeZoneOffset;

  // Caps(JEP-0115: Entity Capabilities) information
  QString capsNode, capsVersion;
  DiscoItem::Identity discoIdentity;
};

XMPP::S5BServer *JabberClient::Private::s5bServer = nullptr;
QStringList JabberClient::Private::s5bAddressList;
int JabberClient::Private::s5bServerPort = 8010;

JabberClient::JabberClient ()
{
  d = new Private();

  cleanUp ();

  // initiate penalty timer
  QTimer::singleShot ( JABBER_PENALTY_TIME * 1000, this, SLOT (slotUpdatePenaltyTime()) );

}

JabberClient::~JabberClient ()
{
  delete d;
}

void JabberClient::cleanUp ()
{
  if ( d->jabberClient )
  {
    d->jabberClient->close ();
  }
  
  delete d->jabberClient;
  delete d->jabberClientStream;
  delete d->jabberClientConnector;
  delete d->jabberTLSHandler;
  delete d->jabberTLS;
  // privacyManager will be deleted with jabberClient, its parent's parent

  d->jabberClient = nullptr;
  d->jabberClientStream = nullptr;
  d->jabberClientConnector = nullptr;
  d->jabberTLSHandler = nullptr;
  d->jabberTLS = nullptr;
  d->privacyManager = nullptr;

  d->currentPenaltyTime = 0;

  d->jid = XMPP::Jid ();
  d->password.clear();

  setForceTLS ( false );
  setUseSSL ( false );
  setUseXMPP09 ( false );
  setProbeSSL ( false );

  setOverrideHost ( false );

  setAllowPlainTextPassword ( true );

  setFileTransfersEnabled ( false );
  setS5BServerPort ( 8010 );

  setClientName ( QString() );
  setClientVersion ( QString() );
  setOSName ( QString() );

  setTimeZone ( "UTC", 0 );

  setIgnoreTLSWarnings ( false );

}

void JabberClient::slotUpdatePenaltyTime ()
{

  if ( d->currentPenaltyTime >= JABBER_PENALTY_TIME )
    d->currentPenaltyTime -= JABBER_PENALTY_TIME;
  else
    d->currentPenaltyTime = 0;

  QTimer::singleShot ( JABBER_PENALTY_TIME * 1000, this, SLOT (slotUpdatePenaltyTime()) );

}

void JabberClient::setIgnoreTLSWarnings ( bool flag )
{

  d->ignoreTLSWarnings = flag;

}

bool JabberClient::ignoreTLSWarnings ()
{

  return d->ignoreTLSWarnings;

}

bool JabberClient::setS5BServerPort ( int port )
{

  d->s5bServerPort = port;

  if ( fileTransfersEnabled () )
  {
    return s5bServer()->start ( port );
  }

  return true;

}

int JabberClient::s5bServerPort () const
{

  return d->s5bServerPort;

}

XMPP::S5BServer *JabberClient::s5bServer ()
{

  if ( !d->s5bServer )
  {
    d->s5bServer = new XMPP::S5BServer ();
    QObject::connect ( d->s5bServer, SIGNAL (destroyed()), this, SLOT (slotS5BServerGone()) );

    /*
     * Try to start the server at the default port here.
     * We have no way of notifying the caller of an error.
     * However, since the caller will usually also
     * use setS5BServerPort() to ensure the correct
     * port, we can return an error code there.
     */
    if ( fileTransfersEnabled () )
    {
      s5bServer()->start ( d->s5bServerPort );
    }
  }

  return d->s5bServer;

}

void JabberClient::slotS5BServerGone ()
{

  d->s5bServer = nullptr;

  if ( d->jabberClient )
    d->jabberClient->s5bManager()->setServer( nullptr );

}

void JabberClient::addS5BServerAddress ( const QString &address )
{
  QStringList newList;

  d->s5bAddressList.append ( address );

  // now filter the list without dupes
  foreach( QStringList::const_reference str, d->s5bAddressList )
  {
    if ( !newList.contains ( str ) )
      newList.append ( str );
  }

  s5bServer()->setHostList ( newList );

}

void JabberClient::removeS5BServerAddress ( const QString &address )
{
  QStringList newList;

  int idx = d->s5bAddressList.indexOf( address );

  if ( idx != -1 )
    d->s5bAddressList.removeAt(idx);

  if ( d->s5bAddressList.isEmpty () )
  {
    delete d->s5bServer;
    d->s5bServer = nullptr;
  }
  else
  {
    // now filter the list without dupes
    foreach( QStringList::const_reference str, d->s5bAddressList )
    {
      if ( !newList.contains ( str ) )
        newList.append ( str );
    }

    s5bServer()->setHostList ( newList );
  }

}

void JabberClient::setForceTLS ( bool flag )
{

  d->forceTLS = flag;

}

bool JabberClient::forceTLS () const
{

  return d->forceTLS;

}

void JabberClient::setUseSSL ( bool flag )
{

  d->useSSL = flag;

}

bool JabberClient::useSSL () const
{

  return d->useSSL;

}

void JabberClient::setUseXMPP09 ( bool flag )
{

  d->useXMPP09 = flag;

}

bool JabberClient::useXMPP09 () const
{

  return d->useXMPP09;

}

void JabberClient::setProbeSSL ( bool flag )
{

  d->probeSSL = flag;

}

bool JabberClient::probeSSL () const
{

  return d->probeSSL;

}

void JabberClient::setOverrideHost ( bool flag, const QString &server, int port )
{

  d->overrideHost = flag;
  d->server = server;
  d->port = port;

}

bool JabberClient::overrideHost () const
{

  return d->overrideHost;

}

void JabberClient::setAllowPlainTextPassword ( bool flag )
{

  d->allowPlainTextPassword = flag;

}

bool JabberClient::allowPlainTextPassword () const
{

  return d->allowPlainTextPassword;

}

void JabberClient::setFileTransfersEnabled ( bool flag, const QString &localAddress )
{

  d->fileTransfersEnabled = flag;
  d->localAddress = localAddress;

}

QString JabberClient::localAddress () const
{

  return d->localAddress;

}

bool JabberClient::fileTransfersEnabled () const
{

  return d->fileTransfersEnabled;

}

void JabberClient::setClientName ( const QString &clientName )
{

  d->clientName = clientName;

}

QString JabberClient::clientName () const
{

  return d->clientName;

}

void JabberClient::setClientVersion ( const QString &clientVersion )
{

  d->clientVersion = clientVersion;

}

QString JabberClient::clientVersion () const
{

  return d->clientVersion;

}

void JabberClient::setOSName ( const QString &osName )
{

  d->osName = osName;

}

QString JabberClient::osName () const
{

  return d->osName;

}

void JabberClient::setCapsNode( const QString &capsNode )
{
  d->capsNode = capsNode;
}

QString JabberClient::capsNode() const
{
  return d->capsNode;
}

void JabberClient::setCapsVersion( const QString &capsVersion )
{
  d->capsVersion = capsVersion;
}

QString JabberClient::capsVersion() const
{
  return d->capsVersion;
}

QString JabberClient::capsExt() const
{
  if(d->jabberClient)
  {
    return d->jabberClient->capsExt();
  }

  return QString();
}
void JabberClient::setDiscoIdentity( DiscoItem::Identity identity )
{
  d->discoIdentity = identity;
}

DiscoItem::Identity JabberClient::discoIdentity() const
{
  return d->discoIdentity;
}

void JabberClient::setTimeZone ( const QString &timeZoneName, int timeZoneOffset )
{

  d->timeZoneName = timeZoneName;
  d->timeZoneOffset = timeZoneOffset;

}

QString JabberClient::timeZoneName () const
{

  return d->timeZoneName;

}

int JabberClient::timeZoneOffset () const
{

  return d->timeZoneOffset;

}

int JabberClient::getPenaltyTime ()
{

  int currentTime = d->currentPenaltyTime;

  d->currentPenaltyTime += JABBER_PENALTY_TIME;

  return currentTime;

}

XMPP::Client *JabberClient::client () const
{

  return d->jabberClient;

}

XMPP::ClientStream *JabberClient::clientStream () const
{

  return d->jabberClientStream;

}

JabberConnector *JabberClient::clientConnector () const
{

  return d->jabberClientConnector;

}

XMPP::Task *JabberClient::rootTask () const
{

  if ( client () )
  {
    return client()->rootTask ();
  }
  else
  {
    return nullptr;
  }

}

XMPP::FileTransferManager *JabberClient::fileTransferManager () const
{

  if ( client () )
  {
    return client()->fileTransferManager ();
  }
  else
  {
    return nullptr;
  }

}

PrivacyManager *JabberClient::privacyManager () const
{
  return d->privacyManager;
}

XMPP::Jid JabberClient::jid () const
{

  return d->jid;

}

JabberClient::ErrorCode JabberClient::connect ( const XMPP::Jid &jid, const QString &password, bool auth )
{
  /*
   * Close any existing connection.
   */
  if ( d->jabberClient )
  {
    d->jabberClient->close ();
  }
  d->jid = jid;
  d->password = password;

  /*
   * Return an error if we should force TLS but it's not available.
   */
  if ( ( forceTLS () || useSSL () || probeSSL () ) && !QCA::isSupported ("tls" ) )
  {
    qCDebug(JABBER_PROTOCOL_LOG) << "no TLS";
    return NoTLS;
  }

  /*
   * Instantiate connector, responsible for dealing with the socket.
   * This class uses KDE's socket code, which in turn makes use of
   * the global proxy settings.
   */
  if ( d->jabberClientConnector == nullptr)
  {
    d->jabberClientConnector = new JabberConnector;
  }

  d->jabberClientConnector->setOptSSL ( useSSL () );

  if ( useXMPP09 () )
  {
    if ( overrideHost () )
    {
      d->jabberClientConnector->setOptHostPort ( d->server, d->port );
    }

    d->jabberClientConnector->setOptProbe ( probeSSL () );

  }

  /*
   * Setup authentication layer
   */
  if ( QCA::isSupported ("tls") )
  {
    qCDebug(JABBER_PROTOCOL_LOG) << "QCA tls";
    if (d->jabberTLS == nullptr)
      d->jabberTLS = new QCA::TLS;
    d->jabberTLS->setTrustedCertificates(QCA::systemStore());
    if (d->jabberTLSHandler == nullptr)
      d->jabberTLSHandler = new QCATLSHandler(d->jabberTLS);
    d->jabberTLSHandler->setXMPPCertCheck(true);

    QObject::connect ( d->jabberTLSHandler, SIGNAL (tlsHandshaken()), SLOT (slotTLSHandshaken()) );
  }

  /*
   * Instantiate client stream which handles the network communication by referring
   * to a connector (proxying etc.) and a TLS handler (security layer)
   */
  if (d->jabberClientStream != nullptr)
    delete d->jabberClientStream;
  d->jabberClientStream = new XMPP::ClientStream ( d->jabberClientConnector, d->jabberTLSHandler );

  {
    using namespace XMPP;
    QObject::connect ( d->jabberClientStream, SIGNAL (needAuthParams(bool,bool,bool)),
           this, SLOT (slotCSNeedAuthParams(bool,bool,bool)) );
    QObject::connect ( d->jabberClientStream, SIGNAL (authenticated()),
           this, SLOT (slotCSAuthenticated()) );
    QObject::connect ( d->jabberClientStream, SIGNAL (connectionClosed()),
           this, SLOT (slotCSDisconnected()) );
    QObject::connect ( d->jabberClientStream, SIGNAL (delayedCloseFinished()),
           this, SLOT (slotCSDisconnected()) );
    QObject::connect ( d->jabberClientStream, SIGNAL (warning(int)),
           this, SLOT (slotCSWarning(int)) );
    QObject::connect ( d->jabberClientStream, SIGNAL (error(int)),
           this, SLOT (slotCSError(int)) );
  }

  d->jabberClientStream->setOldOnly ( useXMPP09 () );

  /*
   * Initiate anti-idle timer (will be triggered every 55 seconds).
   */
  d->jabberClientStream->setNoopTime ( 55000 );

  /*
   * Allow plaintext password authentication or not?
   */
  d->jabberClientStream->setAllowPlain( allowPlainTextPassword () ?  XMPP::ClientStream::AllowPlain : XMPP::ClientStream::NoAllowPlain );

  /*
   * Setup client layer.
   */
    qCDebug(JABBER_PROTOCOL_LOG) << "Setup client layer";
    if (d->jabberClient != nullptr)
      delete d->jabberClient;
    d->jabberClient = new XMPP::Client ( this );
  
  /*
   * Setup privacy manager
   */
  if (d->privacyManager == nullptr)
    d->privacyManager = new PrivacyManager ( rootTask() );

  /*
   * Enable file transfer (IP and server will be set after connection
   * has been established.
   */
  if ( fileTransfersEnabled () )
  {
    d->jabberClient->setFileTransferEnabled ( true );

    {
      using namespace XMPP;
      QObject::connect ( d->jabberClient->fileTransferManager(), SIGNAL (incomingReady()),
             this, SLOT (slotIncomingFileTransfer()) );
    }
  }

  /* This should only be done here to connect the signals, otherwise it is a
   * bad idea.
   */
  {
    using namespace XMPP;
    qCDebug(JABBER_PROTOCOL_LOG) << "Connecting subscription";
    QObject::connect ( d->jabberClient, SIGNAL (subscription(Jid,QString,QString)),
    this, SLOT (slotSubscription(Jid,QString)) );
    qCDebug(JABBER_PROTOCOL_LOG) << "Connecting rosterRequestFinished";
    QObject::connect ( d->jabberClient, SIGNAL (rosterRequestFinished(bool,int,QString)),
    this, SLOT (slotRosterRequestFinished(bool,int,QString)) );
    QObject::connect ( d->jabberClient, SIGNAL (rosterItemAdded(RosterItem)),
           this, SLOT (slotNewContact(RosterItem)) );
    QObject::connect ( d->jabberClient, SIGNAL (rosterItemUpdated(RosterItem)),
           this, SLOT (slotContactUpdated(RosterItem)) );
    QObject::connect ( d->jabberClient, SIGNAL (rosterItemRemoved(RosterItem)),
           this, SLOT (slotContactDeleted(RosterItem)) );
    QObject::connect ( d->jabberClient, SIGNAL (resourceAvailable(Jid,Resource)),
           this, SLOT (slotResourceAvailable(Jid,Resource)) );
    QObject::connect ( d->jabberClient, SIGNAL (resourceUnavailable(Jid,Resource)),
           this, SLOT (slotResourceUnavailable(Jid,Resource)) );
    QObject::connect ( d->jabberClient, SIGNAL (messageReceived(Message)),
           this, SLOT (slotReceivedMessage(Message)) );
    QObject::connect ( d->jabberClient, SIGNAL (groupChatJoined(Jid)),
           this, SLOT (slotGroupChatJoined(Jid)) );
    QObject::connect ( d->jabberClient, SIGNAL (groupChatLeft(Jid)),
           this, SLOT (slotGroupChatLeft(Jid)) );
    QObject::connect ( d->jabberClient, SIGNAL (groupChatPresence(Jid,Status)),
           this, SLOT (slotGroupChatPresence(Jid,Status)) );
    QObject::connect ( d->jabberClient, SIGNAL (groupChatError(Jid,int,QString)),
           this, SLOT (slotGroupChatError(Jid,int,QString)) );
    //QObject::connect ( d->jabberClient, SIGNAL (debugText(QString)),
    //       this, SLOT (slotPsiDebug(QString)) );
    QObject::connect ( d->jabberClient, SIGNAL (xmlIncoming(QString)),
           this, SLOT (slotIncomingXML(QString)) );
    QObject::connect ( d->jabberClient, SIGNAL (xmlOutgoing(QString)),
           this, SLOT (slotOutgoingXML(QString)) );
  }

  d->jabberClient->setClientName ( clientName () );
  d->jabberClient->setClientVersion ( clientVersion () );
  d->jabberClient->setOSName ( osName () );

  // Set caps information
  d->jabberClient->setCapsNode( capsNode() );
  d->jabberClient->setCapsVersion( capsVersion() );
  
  // Set Disco Identity
  d->jabberClient->setIdentity( discoIdentity() );

  d->jabberClient->setTimeZone ( timeZoneName (), timeZoneOffset () );

  qCDebug(JABBER_PROTOCOL_LOG) << "connect to server" << jid.domain() << jid.node() << jid.resource() << jid.bare() << jid.full() << auth;
  d->jabberClient->connectToServer ( d->jabberClientStream, jid, auth );

  return Ok;
}

void JabberClient::disconnect ()
{

  if ( d->jabberClient )
  {
    d->jabberClient->close ();
  }
  else
  {
    cleanUp ();
  }

}

void JabberClient::disconnect( XMPP::Status &reason )
{
    if ( d->jabberClient )
    {
        if ( d->jabberClientStream->isActive() )
        {
            XMPP::JT_Presence *pres = new JT_Presence(rootTask());
            reason.setIsAvailable( false );
            pres->pres( reason );
            pres->go();

            d->jabberClientStream->close();
            d->jabberClient->close();
        }
    }
    else
    {
        cleanUp();
    }
}

bool JabberClient::isConnected () const
{

  if ( d->jabberClient )
  {
    return d->jabberClient->isActive ();
  }

  return false;

}

void JabberClient::joinGroupChat ( const QString &host, const QString &room, const QString &nick )
{

  client()->groupChatJoin ( host, room, nick );

}

void JabberClient::joinGroupChat ( const QString &host, const QString &room, const QString &nick, const QString &password )
{
  client()->groupChatJoin ( host, room, nick, password );

}

void JabberClient::leaveGroupChat ( const QString &host, const QString &room )
{

  client()->groupChatLeave ( host, room );

}

void JabberClient::setGroupChatStatus( const QString & host, const QString & room, const XMPP::Status & status )
{
  client()->groupChatSetStatus( host, room, status);
}

void JabberClient::changeGroupChatNick( const QString & host, const QString & room, const QString & nick, const XMPP::Status & status )
{
  client()->groupChatChangeNick( host, room, nick, status );
}


void JabberClient::sendMessage ( const XMPP::Message &message )
{

  client()->sendMessage ( message );

}

void JabberClient::send ( const QString &packet )
{

  client()->send ( packet );

}

void JabberClient::requestRoster ()
{

  client()->rosterRequest ();

}

void JabberClient::slotPsiDebug ( const QString & _msg )
{
  QString msg = _msg;

  msg = msg.replace( QRegExp( "<password>[^<]*</password>\n" ), "<password>[Filtered]</password>\n" );
  msg = msg.replace( QRegExp( "<digest>[^<]*</digest>\n" ), "<digest>[Filtered]</digest>\n" );

  emit debugMessage ( "Psi: " + msg );

}

void JabberClient::slotIncomingXML ( const QString & _msg )
{
  QString msg = _msg;

  msg = msg.replace( QRegExp( "<password>[^<]*</password>\n" ), "<password>[Filtered]</password>\n" );
  msg = msg.replace( QRegExp( "<digest>[^<]*</digest>\n" ), "<digest>[Filtered]</digest>\n" );

  emit debugMessage ( "XML IN: " + msg );
  emit incomingXML ( msg );
}

void JabberClient::slotOutgoingXML ( const QString & _msg )
{
  QString msg = _msg;

  msg = msg.replace( QRegExp( "<password>[^<]*</password>\n" ), "<password>[Filtered]</password>\n" );
  msg = msg.replace( QRegExp( "<digest>[^<]*</digest>\n" ), "<digest>[Filtered]</digest>\n" );

  emit debugMessage ( "XML OUT: " + msg );
  emit outgoingXML ( msg );
}

void JabberClient::slotTLSHandshaken ()
{

  emit debugMessage ( "TLS handshake done, testing certificate validity..." );

  // FIXME: in the future, this should be handled by KDE, not QCA

  QCA::TLS::IdentityResult identityResult = d->jabberTLS->peerIdentityResult();
  QCA::Validity            validityResult = d->jabberTLS->peerCertificateValidity();

  if ( identityResult == QCA::TLS::Valid && validityResult == QCA::ValidityGood )
  {
    emit debugMessage ( "Identity and certificate valid, continuing." );

    // valid certificate, continue
    d->jabberTLSHandler->continueAfterHandshake ();
  }
  else
  {
    emit debugMessage ( "Certificate is not valid, asking user what to do next." );

    // certificate is not valid, query the user
    if ( ignoreTLSWarnings () )
    {
      emit debugMessage ( "We are supposed to ignore TLS warnings, continuing." );
      d->jabberTLSHandler->continueAfterHandshake ();
    }

    emit tlsWarning ( identityResult, validityResult );
  }

}

void JabberClient::continueAfterTLSWarning ()
{

  if ( d->jabberTLSHandler )
  {
    d->jabberTLSHandler->continueAfterHandshake ();
  }

}

void JabberClient::slotCSNeedAuthParams ( bool user, bool pass, bool realm )
{
  emit debugMessage ( "Sending auth credentials..." );

  if ( user )
  {
    d->jabberClientStream->setUsername ( jid().node () );
  }

  if ( pass )
  {
    d->jabberClientStream->setPassword ( d->password );
  }

  if ( realm )
  {
    d->jabberClientStream->setRealm ( jid().domain () );
  }

  d->jabberClientStream->continueAfterParams ();

}

void JabberClient::slotCSAuthenticated ()
{
  qCDebug(JABBER_PROTOCOL_LOG);
  emit debugMessage ( "Connected to Jabber server." );

  /*
   * Determine local IP address.
   * FIXME: This is ugly!
   */
  if ( localAddress().isEmpty () )
  {
    // code for Iris-type bytestreams
    ByteStream *irisByteStream = d->jabberClientConnector->stream();
    if ( irisByteStream->inherits ( "BSocket" ) || irisByteStream->inherits ( "XMPP::BSocket" ) )
    {
      d->localAddress = ( (BSocket *)irisByteStream )->address().toString ();
    }

    // code for the KDE-type bytestream
    JabberByteStream *kdeByteStream = dynamic_cast<JabberByteStream*>(d->jabberClientConnector->stream());
    if ( kdeByteStream )
    {
      d->localAddress = kdeByteStream->socket()->peerName();
    }
  }

  if ( fileTransfersEnabled () )
  {
    // setup file transfer
    addS5BServerAddress ( localAddress () );
    d->jabberClient->s5bManager()->setServer ( s5bServer () );
  }

  // start the client operation
  d->jabberClient->start ( jid().domain (), jid().node (), d->password, jid().resource () );

  emit connected ();
}

void JabberClient::slotCSDisconnected ()
{

  /* FIXME:
   * We should delete the XMPP::Client instance here,
   * but timers etc prevent us from doing so. (Psi does
   * not like to be deleted from a slot).
   */

  emit debugMessage ( "Disconnected, freeing up file transfer port..." );

  // delete local address from S5B server
  removeS5BServerAddress ( localAddress () );

  emit csDisconnected ();

}

void JabberClient::slotCSWarning ( int warning )
{

  emit debugMessage ( "Client stream warning." );

  /*
   * FIXME: process all other warnings
   */
  switch ( warning )
  {
    //case XMPP::ClientStream::WarnOldVersion:
    case XMPP::ClientStream::WarnNoTLS:
      if ( forceTLS () )
      {
        disconnect ();
        emit error ( NoTLS );
        return;
      }
      break;
  }

  d->jabberClientStream->continueAfterWarning ();

}

void JabberClient::slotCSError ( int error )
{

  emit debugMessage ( "Client stream error." );

  emit csError ( error );

}

void JabberClient::slotRosterRequestFinished ( bool success, int /*statusCode*/, const QString &/*statusString*/ )
{

  emit rosterRequestFinished ( success );

}

void JabberClient::slotIncomingFileTransfer ()
{

  emit incomingFileTransfer ();

}

void JabberClient::slotNewContact ( const XMPP::RosterItem &item )
{

  emit newContact ( item );

}

void JabberClient::slotContactDeleted ( const RosterItem &item )
{

  emit contactDeleted ( item );

}

void JabberClient::slotContactUpdated ( const RosterItem &item )
{

  emit contactUpdated ( item );

}

void JabberClient::slotResourceAvailable ( const Jid &jid, const Resource &resource )
{

  emit resourceAvailable ( jid, resource );

}

void JabberClient::slotResourceUnavailable ( const Jid &jid, const Resource &resource )
{

  emit resourceUnavailable ( jid, resource );

}

void JabberClient::slotReceivedMessage ( const Message &message )
{

  emit messageReceived ( message );

}

void JabberClient::slotGroupChatJoined ( const Jid &jid )
{

  emit groupChatJoined ( jid );

}

void JabberClient::slotGroupChatLeft ( const Jid &jid )
{

  emit groupChatLeft ( jid );

}

void JabberClient::slotGroupChatPresence ( const Jid &jid, const Status &status)
{

  emit groupChatPresence ( jid, status );

}

void JabberClient::slotGroupChatError ( const Jid &jid, int error, const QString &reason)
{

  emit groupChatError ( jid, error, reason );

}

void JabberClient::slotSubscription(const XMPP::Jid& jid, const QString& type)
{

  emit subscription ( jid, type );

}

#include "moc_jabberclient.cpp"
