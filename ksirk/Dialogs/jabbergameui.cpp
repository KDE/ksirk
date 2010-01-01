/* This file is part of KsirK.
   Copyright (C) 2008 Gael de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

#include "jabbergameui.h"
#include "kgamewin.h"
#include "GameLogic/gameautomaton.h"
#include "ksirksettings.h"

#include <KDebug>
#include <KStringHandler>
#include <kwallet.h>

#include <QUuid>

using namespace Ksirk;
using namespace Ksirk::GameLogic;

KsirkJabberGameWidget::KsirkJabberGameWidget(QWidget* parent) :
    QWidget(parent),
    m_automaton(0),
    m_previousGuiIndex(-1)
{
  kDebug();
  
  setupUi(this);
  stackedWidget->setCurrentIndex(0);
  jabberid->setText(Ksirk::KsirkSettings::jabberId());

  KWallet::Wallet* wallet = KWallet::Wallet::openWallet("ksirk", 0);
  if (wallet == 0)
  {
    password->setText(KStringHandler::obscure(Ksirk::KsirkSettings::jabberPassword()));
    roompassword->setText(KStringHandler::obscure(Ksirk::KsirkSettings::jabberPassword()));
  }
  else
  {
    if (wallet->createFolder("jabber") && wallet->setFolder("jabber"))
    {
      QByteArray pw;
      wallet->readEntry("password", pw);
      password->setText(QString::fromUtf8(pw.data()));
      wallet->readEntry("roompassword", pw);
      roompassword->setText(QString::fromUtf8(pw.data()));
    }
    delete wallet;
  }
  roomjid->setText(Ksirk::KsirkSettings::roomJid());
  nickname->setText(Ksirk::KsirkSettings::nickname());

  jabberstateled->setState(KLed::Off);
  chatroomstateled->setState(KLed::Off);
      
  connect(connectbutton,SIGNAL(clicked()),this,SLOT(slotJabberConnectButtonClicked()));
  connect(joingamebutton,SIGNAL(clicked()),this,SLOT(slotJoinJabberGame()));
  QObject::connect ( joinroombutton, SIGNAL (clicked()), this, SLOT(slotJoinRoom()));
  
  QObject::connect(jabberTable, SIGNAL(cellClicked(int,int)), this, SLOT(slotCellClicked(int,int)));
  
  QStringList headers;
  headers.push_back(i18n("Nickname"));
  headers.push_back(i18n("Skin"));
  headers.push_back(i18n("Nb players"));
  jabberTable->setHorizontalHeaderLabels(headers);

  QObject::connect(cancelbutton, SIGNAL(clicked()),this,SLOT(slotCancel()));
}

void KsirkJabberGameWidget::init(GameAutomaton* automaton)
{
  kDebug();
  m_automaton = automaton;
  jabberstateled->setState(
      (m_automaton->game()->jabberClient()->isConnected())
      ? KLed::On : KLed::Off);
  
  QObject::connect(startnewgamebutton,SIGNAL(clicked()),m_automaton->game(),SLOT(slotNewJabberGame()));
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL ( csDisconnected () ), this, SLOT ( slotJabberDisconnected() ) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL ( csError ( int ) ), this, SLOT ( slotJabberError( int ) ) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL ( tlsWarning ( QCA::TLS::IdentityResult, QCA::Validity ) ), this, SLOT ( slotHandleTLSWarning ( QCA::TLS::IdentityResult, QCA::Validity ) ) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL ( connected () ), this, SLOT ( slotJabberConnected() ) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL ( error ( JabberClient::ErrorCode ) ), this, SLOT ( slotJabberClientError( JabberClient::ErrorCode ) ) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL ( rosterRequestFinished ( bool ) ), this, SLOT ( slotRosterRequestFinished ( bool ) ) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (groupChatJoined (const XMPP::Jid&)), this, SLOT(slotGroupChatJoined(const XMPP::Jid&)));
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (groupChatLeft(const XMPP::Jid&)), this, SLOT(slotGroupChatLeft(const XMPP::Jid&)));
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (groupChatPresence(const XMPP::Jid&, const XMPP::Status&)), this, SLOT(slotGroupChatPresence(const XMPP::Jid&, const XMPP::Status&)));
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (groupChatError(const XMPP::Jid&, int, const QString&)), this, SLOT(slotGroupChatError(const XMPP::Jid&, int, const QString&)));
  QObject::connect(automaton, SIGNAL(newJabberGame(const QString&, int, const QString&)), this, SLOT(slotNewJabberGame(const QString&, int, const QString&) ) );
  QObject::connect(this, SIGNAL(cancelled(int)), m_automaton->game(), SLOT(slotJabberGameCanceled(int)));
}

void KsirkJabberGameWidget::slotJabberConnectButtonClicked()
{
  kDebug();
  
  KsirkSettings::setJabberId(jabberid->text());
  XMPP::Jid jid(jabberid->text()+'/'+jabberid->text());
  QString pass = password->text();


  KWallet::Wallet* wallet = KWallet::Wallet::openWallet("ksirk", 0);
  if (wallet == 0)
  {
    KsirkSettings::setJabberPassword(KStringHandler::obscure(pass));
  }
  else
  {
    if (wallet->createFolder("jabber") && wallet->setFolder("jabber"))
    {
      wallet->writeEntry("password", pass.toUtf8());
    }
    delete wallet;
  }
  
  KsirkSettings::self()->writeConfig();

  m_automaton->game()->jabberClient()->setOverrideHost ( true, jid.domain(), 5222 );
  JabberClient::ErrorCode res = m_automaton->game()->jabberClient()->connect(jid, pass);

  switch (res)
  {
    case JabberClient::Ok:
      kDebug() << "Succesfull connexion";
      m_automaton->game()->jabberClient()->requestRoster ();
      break;
    case JabberClient::InvalidPassword:
      kError() << "Password used to connect to the server was incorrect.";
      break;
    case JabberClient::AlreadyConnected:
      kError() << "A new connection was attempted while the previous one hasn't been closed.";
      break;
    case JabberClient::NoTLS:
      kError() << "Use of TLS has been forced (see @ref forceTLS) but TLS is not available, either server- or client-side.";
      break;
    case JabberClient::InvalidPasswordForMUC:
      kError() << "A password is require to enter on this Multi-User Chat";
      break;
    case JabberClient::NicknameConflict:
      kError() << "There is already someone with that nick connected to the Multi-User Chat";
      break;
    case JabberClient::BannedFromThisMUC:
      kError() << "You can't join this Multi-User Chat because you were bannished";
      break;
    case JabberClient::MaxUsersReachedForThisMuc:
      kError() << "You can't join this Multi-User Chat because it is full";
      break;
    default:
      kError() << "Unknown error";
  }
  
}

void KsirkJabberGameWidget::slotJabberDisconnected()
{
  kDebug();
  stackedWidget->setCurrentIndex(0);
  jabberstateled->setState(KLed::Off);
}

void KsirkJabberGameWidget::slotJabberError(int error)
{
  kDebug() << error;
}

void KsirkJabberGameWidget::slotHandleTLSWarning(QCA::TLS::IdentityResult result, QCA::Validity validity)
{
  kDebug() << result << validity;
}

void KsirkJabberGameWidget::slotJabberConnected()
{
  kDebug () << "Connected to Jabber server.";
  jabberstateled->setState(KLed::On);
}

void KsirkJabberGameWidget::slotJabberClientError(JabberClient::ErrorCode error)
{
  kDebug() << error;
}

void KsirkJabberGameWidget::slotRosterRequestFinished ( bool success )
{
  kDebug() << success;
  if ( success )
  {
    stackedWidget->setCurrentIndex(1);
  }
}

void KsirkJabberGameWidget::slotJoinRoom()
{
  kDebug () << "Joining group chat...";
  XMPP::Jid roomJid(roomjid->text());
  QString groupchatHost = roomJid.domain();
  m_automaton->game()->setGroupchatHost(groupchatHost);
  QString groupchatRoom = roomJid.node();
  m_automaton->game()->setGroupchatRoom(groupchatRoom);
  QString groupchatNick = nickname->text();
  m_automaton->game()->setGroupchatNick(groupchatNick);
  QString groupchatPassword = roompassword->text();
  m_automaton->game()->setGroupchatPassword(groupchatPassword);
  KsirkSettings::setRoomJid(groupchatRoom+'@'+groupchatHost);
  KsirkSettings::setNickname(groupchatNick);
  KWallet::Wallet* wallet = KWallet::Wallet::openWallet("ksirk", 0);
  if (wallet == 0)
  {
    KsirkSettings::setRoomPassword(KStringHandler::obscure(groupchatPassword));
  }
  else
  {
    if (wallet->createFolder("jabber") && wallet->setFolder("jabber"))
    {
      wallet->writeEntry("roompassword", groupchatPassword.toUtf8());
    }
    delete wallet;
  }
  KsirkSettings::self()->writeConfig();


  if (groupchatPassword.isEmpty())
    m_automaton->game()->jabberClient()->joinGroupChat ( groupchatHost, groupchatRoom, groupchatNick);
  else
    m_automaton->game()->jabberClient()->joinGroupChat ( groupchatHost, groupchatRoom, groupchatNick, groupchatPassword);
}

void KsirkJabberGameWidget::slotGroupChatJoined(const XMPP::Jid & jid)
{
  kDebug() << jid.full();
  chatroomstateled->setState(KLed::On);

  kDebug () << "Joined groupchat " << jid.full ();

  startnewgamebutton->setEnabled(true);
  joingamebutton->setEnabled(true);
  loadsavedgamebutton->setEnabled(true);
}

void KsirkJabberGameWidget::slotGroupChatLeft (const XMPP::Jid & jid)
{
  kDebug();
  Q_UNUSED(jid);
  chatroomstateled->setState(KLed::Off);

  startnewgamebutton->setEnabled(false);
  joingamebutton->setEnabled(false);
  loadsavedgamebutton->setEnabled(false);
}

void KsirkJabberGameWidget::slotGroupChatPresence (const XMPP::Jid & jid, const XMPP::Status & status)
{
  kDebug() << jid.full() << status.isAvailable();
  if (!status.isAvailable())
  {
    int i = 0;
    while ( i < jabberTable->rowCount())
    {
      if (jabberTable->itemAt(0,i)->text() == jid.full())
      {
        jabberTable->removeRow(i);
      }
      else
      {
        i++;
      }
    }
  }
}

void KsirkJabberGameWidget::slotGroupChatError (const XMPP::Jid & jid, int error, const QString & reason)
{
  kDebug() << jid.full() << error << reason;
}

void KsirkJabberGameWidget::slotNewJabberGame(const QString& nick,
                                        int nbPlayers,
                                        const QString& skin
                                        )
{
  kDebug() << nick << nbPlayers << skin;
  for (int i = 0; i < jabberTable->rowCount(); i++)
  {
    if (jabberTable->itemAt(0,i)->text() == nick)
    {
      kDebug() << "This game is already listed";
      return;
    }
  }
  jabberTable->setSortingEnabled(false);
  jabberTable->setRowCount(jabberTable->rowCount()+1);

  QTableWidgetItem *newItem = new QTableWidgetItem(nick);
  jabberTable->setItem(jabberTable->rowCount()-1,0,newItem);
  newItem = new QTableWidgetItem(skin);
  jabberTable->setItem(jabberTable->rowCount()-1,1,newItem);
  newItem = new QTableWidgetItem(QString::number(nbPlayers));
  jabberTable->setItem(jabberTable->rowCount()-1,2,newItem);

  jabberTable->setSortingEnabled(false);
}

void KsirkJabberGameWidget::slotCellClicked(int row, int column)
{
  Q_UNUSED(column);
  kDebug() << row;
  m_nick = jabberTable->item(row,0)->text();
  m_nbPlayers = jabberTable->item(row,2)->text().toInt();
  kDebug() << m_nick << m_nbPlayers;
}

void KsirkJabberGameWidget::slotJoinJabberGame()
{
  kDebug();
  m_automaton->joinJabberGame(m_nick);
}

void KsirkJabberGameWidget::slotCancel()
{
  kDebug();
  emit cancelled(m_previousGuiIndex);
}
                                        
#include "jabbergameui.moc"
