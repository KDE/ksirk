/* This file is part of KsirK.
   Copyright (C) 2008 Gael de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.

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

#include "ksirk_debug.h"
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
  qCDebug(KSIRK_LOG);
  
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
  qCDebug(KSIRK_LOG);
  m_automaton = automaton;
  jabberstateled->setState(
      (m_automaton->game()->jabberClient()->isConnected())
      ? KLed::On : KLed::Off);
  
  QObject::connect(startnewgamebutton,SIGNAL(clicked()),m_automaton->game(),SLOT(slotNewJabberGame()));
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (csDisconnected()), this, SLOT (slotJabberDisconnected()) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (csError(int)), this, SLOT (slotJabberError(int)) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (tlsWarning(QCA::TLS::IdentityResult,QCA::Validity)), this, SLOT (slotHandleTLSWarning(QCA::TLS::IdentityResult,QCA::Validity)) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (connected()), this, SLOT (slotJabberConnected()) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (error(JabberClient::ErrorCode)), this, SLOT (slotJabberClientError(JabberClient::ErrorCode)) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (rosterRequestFinished(bool)), this, SLOT (slotRosterRequestFinished(bool)) );
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (groupChatJoined(XMPP::Jid)), this, SLOT(slotGroupChatJoined(XMPP::Jid)));
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (groupChatLeft(XMPP::Jid)), this, SLOT(slotGroupChatLeft(XMPP::Jid)));
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (groupChatPresence(XMPP::Jid,XMPP::Status)), this, SLOT(slotGroupChatPresence(XMPP::Jid,XMPP::Status)));
  QObject::connect ( m_automaton->game()->jabberClient(), SIGNAL (groupChatError(XMPP::Jid,int,QString)), this, SLOT(slotGroupChatError(XMPP::Jid,int,QString)));
  QObject::connect(automaton, SIGNAL(newJabberGame(QString,int,QString)), this, SLOT(slotNewJabberGame(QString,int,QString)) );
  QObject::connect(this, SIGNAL(cancelled(int)), m_automaton->game(), SLOT(slotJabberGameCanceled(int)));
}

void KsirkJabberGameWidget::slotJabberConnectButtonClicked()
{
  qCDebug(KSIRK_LOG);
  
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
  
  KsirkSettings::self()->save();

  m_automaton->game()->jabberClient()->setOverrideHost ( true, jid.domain(), 5222 );
  JabberClient::ErrorCode res = m_automaton->game()->jabberClient()->connect(jid, pass);

  switch (res)
  {
    case JabberClient::Ok:
      qCDebug(KSIRK_LOG) << "Succesfull connexion";
      m_automaton->game()->jabberClient()->requestRoster ();
      break;
    case JabberClient::InvalidPassword:
      qCCritical(KSIRK_LOG) << "Password used to connect to the server was incorrect.";
      break;
    case JabberClient::AlreadyConnected:
      qCCritical(KSIRK_LOG) << "A new connection was attempted while the previous one hasn't been closed.";
      break;
    case JabberClient::NoTLS:
      qCCritical(KSIRK_LOG) << "Use of TLS has been forced (see @ref forceTLS) but TLS is not available, either server- or client-side.";
      break;
    case JabberClient::InvalidPasswordForMUC:
      qCCritical(KSIRK_LOG) << "A password is require to enter on this Multi-User Chat";
      break;
    case JabberClient::NicknameConflict:
      qCCritical(KSIRK_LOG) << "There is already someone with that nick connected to the Multi-User Chat";
      break;
    case JabberClient::BannedFromThisMUC:
      qCCritical(KSIRK_LOG) << "You can't join this Multi-User Chat because you were bannished";
      break;
    case JabberClient::MaxUsersReachedForThisMuc:
      qCCritical(KSIRK_LOG) << "You can't join this Multi-User Chat because it is full";
      break;
    default:
      qCCritical(KSIRK_LOG) << "Unknown error";
  }
  
}

void KsirkJabberGameWidget::slotJabberDisconnected()
{
  qCDebug(KSIRK_LOG);
  stackedWidget->setCurrentIndex(0);
  jabberstateled->setState(KLed::Off);
}

void KsirkJabberGameWidget::slotJabberError(int error)
{
  qCDebug(KSIRK_LOG) << error;
}

void KsirkJabberGameWidget::slotHandleTLSWarning(QCA::TLS::IdentityResult result, QCA::Validity validity)
{
  qCDebug(KSIRK_LOG) << result << validity;
}

void KsirkJabberGameWidget::slotJabberConnected()
{
  qCDebug(KSIRK_LOG) << "Connected to Jabber server.";
  jabberstateled->setState(KLed::On);
}

void KsirkJabberGameWidget::slotJabberClientError(JabberClient::ErrorCode error)
{
  qCDebug(KSIRK_LOG) << error;
}

void KsirkJabberGameWidget::slotRosterRequestFinished ( bool success )
{
  qCDebug(KSIRK_LOG) << success;
  if ( success )
  {
    stackedWidget->setCurrentIndex(1);
  }
}

void KsirkJabberGameWidget::slotJoinRoom()
{
  qCDebug(KSIRK_LOG) << "Joining group chat...";
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
  KsirkSettings::self()->save();


  if (groupchatPassword.isEmpty())
    m_automaton->game()->jabberClient()->joinGroupChat ( groupchatHost, groupchatRoom, groupchatNick);
  else
    m_automaton->game()->jabberClient()->joinGroupChat ( groupchatHost, groupchatRoom, groupchatNick, groupchatPassword);
}

void KsirkJabberGameWidget::slotGroupChatJoined(const XMPP::Jid & jid)
{
  qCDebug(KSIRK_LOG) << jid.full();
  chatroomstateled->setState(KLed::On);

  qCDebug(KSIRK_LOG) << "Joined groupchat " << jid.full ();

  startnewgamebutton->setEnabled(true);
  joingamebutton->setEnabled(true);
  loadsavedgamebutton->setEnabled(true);
}

void KsirkJabberGameWidget::slotGroupChatLeft (const XMPP::Jid & jid)
{
  qCDebug(KSIRK_LOG);
  Q_UNUSED(jid);
  chatroomstateled->setState(KLed::Off);

  startnewgamebutton->setEnabled(false);
  joingamebutton->setEnabled(false);
  loadsavedgamebutton->setEnabled(false);
}

void KsirkJabberGameWidget::slotGroupChatPresence (const XMPP::Jid & jid, const XMPP::Status & status)
{
  qCDebug(KSIRK_LOG) << jid.full() << status.isAvailable();
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
  qCDebug(KSIRK_LOG) << jid.full() << error << reason;
}

void KsirkJabberGameWidget::slotNewJabberGame(const QString& nick,
                                        int nbPlayers,
                                        const QString& skin
                                        )
{
  qCDebug(KSIRK_LOG) << nick << nbPlayers << skin;
  for (int i = 0; i < jabberTable->rowCount(); i++)
  {
    if (jabberTable->itemAt(0,i)->text() == nick)
    {
      qCDebug(KSIRK_LOG) << "This game is already listed";
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
  qCDebug(KSIRK_LOG) << row;
  m_nick = jabberTable->item(row,0)->text();
  m_nbPlayers = jabberTable->item(row,2)->text().toInt();
  qCDebug(KSIRK_LOG) << m_nick << m_nbPlayers;
}

void KsirkJabberGameWidget::slotJoinJabberGame()
{
  qCDebug(KSIRK_LOG);
  m_automaton->joinJabberGame(m_nick);
}

void KsirkJabberGameWidget::slotCancel()
{
  qCDebug(KSIRK_LOG);
  emit cancelled(m_previousGuiIndex);
}
                                        

