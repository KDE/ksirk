/***************************************************************************
                          kdialogsetupjoueur.cpp  -  description
                             -------------------
    begin                : Thu Jul 19 2001
    copyright            : (C) 2001 by Gaël de Chalendar
    email                : Gael.de.Chalendar@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/
#define KDE_NO_COMPAT
#include "kplayersetupwidget.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/player.h"
#include "GameLogic/nationality.h"
#include "GameLogic/onu.h"
#include "Sprites/skinSpritesData.h"
#include "Sprites/flagsprite.h"
#include "newplayerdata.h"
#include "newgamesetup.h"

#include <QString>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPixmap>
#include <QPainter>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kpassworddialog.h>
#include <kmessagebox.h>

#define _XOPEN_SOURCE_
// #include <unistd.h>

namespace Ksirk
{

KPlayerSetupWidget::KPlayerSetupWidget(QWidget *parent) :
  QWidget(parent), Ui::QPlayerSetupWidget()
{
  kDebug();
  setupUi(this);

  connect(nationCombo, SIGNAL(activated(int)), this, SLOT(slotNationChanged()));
  connect(nameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(slotNameEdited(QString)));
  connect(nationCombo, SIGNAL(activated(int)), this, SLOT(slotNationChanged()));
  connect(nextButton, SIGNAL(pressed()),this,SLOT(slotNext()));
  connect(previousButton, SIGNAL(pressed()),this,SLOT(slotPrevious()));
  connect(cancelButton, SIGNAL(pressed()),this,SLOT(slotCancel()));
  
  messageLabel->hide();
}

KPlayerSetupWidget::~KPlayerSetupWidget()
{
  hide();
}

void KPlayerSetupWidget::init(GameLogic::GameAutomaton* automaton,
                              GameLogic::ONU* onu,
                              unsigned int playerNumber,
                              QString &playerName,
                              bool network,
                              QString& password,
                              bool computerPlayer,
                              QMap< QString, QString >& nations,
                              QString & nationName,
                              NewGameSetup* newGameSetup)

{
  kDebug() << playerName << nationName;

  m_automaton = automaton;
  m_name = playerName;
  m_computer = computerPlayer;
  m_nationName = nationName;
  m_nations = nations;
  m_onu = onu;
  m_number = playerNumber;
  m_password = password;
  m_newGameSetup = newGameSetup;

  kDebug() << "connecting to playerJoinedGame";
  connect(automaton,SIGNAL(signalPlayerJoinedGame(KPlayer*)), this, SLOT(slotPlayerJoinedGame(KPlayer*)));
  init();
  kDebug() << "constructor done";
}


void KPlayerSetupWidget::slotNext()
{
  kDebug() << m_newGameSetup->players().size() << m_newGameSetup->nbPlayers();

  m_name = nameLineEdit-> text().trimmed();
//     kDebug() << "Got name " << name;
  m_computer = (isComputerCheckBox-> checkState() == Qt::Checked);
//     kDebug() << "computer? : " << computer;
  m_nationName = m_nationsNames[nationCombo->currentText()];
// @toport
//     m_password = QString(crypt(passwordEdit->password(),"T6"));

//     accept();
  if (m_newGameSetup->players().size() < m_newGameSetup->nbPlayers())
  {
    kDebug() << "Add new player";
    NewPlayerData* newPlayer = new NewPlayerData(m_name, m_nationName, m_password, m_computer, false);
    m_newGameSetup->players().push_back(newPlayer);
    fillNationsCombo();
    slotNationChanged();
    isComputerCheckBox->setCheckState(Qt::Unchecked);
//     init(m_automaton,m_onu,m_newGameSetup->players().size()+1,"",false,"",false,m_nations,"", m_newGameSetup);
    emit next();
  }
  
}

void KPlayerSetupWidget::slotPrevious()
{
  kDebug();
  if (m_newGameSetup->players().empty())
  {
    emit previous();
  }
  else
  {
    NewPlayerData* player = m_newGameSetup->players().back();
    m_newGameSetup->players().pop_back();
    fillNationsCombo();
    slotNationChanged();
    nameLineEdit->setText(player->name());
    passwordEdit->setText(player->password());
    isComputerCheckBox->setCheckState(player->computer()?Qt::Checked:Qt::Unchecked);
    delete player;
  }
}

void KPlayerSetupWidget::slotCancel()
{
  kDebug();
  emit cancel();
}


void KPlayerSetupWidget::fillNationsCombo()
{
  kDebug();
  nationCombo->clear();
  
  GameLogic::Nationality* nation = m_onu->nationNamed(*m_nations.keys().begin());
  nameLineEdit-> setText(nation->leaderName());

  foreach (const QString& k,m_nations.keys())
  {
    if (!isAvailable(k))
    {
      continue;
    }
    const QString& v =m_nations[k];
    kDebug() << "Adding nation " << I18N_NOOP(k) << " / " << v;
//     load image

    FlagSprite flagsprite(v,
                  Sprites::SkinSpritesData::single().intData("flag-width"),
                  Sprites::SkinSpritesData::single().intData("flag-height"),
                  Sprites::SkinSpritesData::single().intData("flag-frames"),
                  Sprites::SkinSpritesData::single().intData("flag-versions"),
                  1.0,m_automaton->game()->backGndWorld());
    QPixmap flag = flagsprite.image(0);

//     get name
    QString name = i18n(k.toUtf8().data());
    m_nationsNames.insert(name,k);
//     fill a combo entry
    nationCombo->addItem(QIcon(flag),name);
  }
  
  kDebug() << "Nations combo filled";
}

void KPlayerSetupWidget::slotPlayerJoinedGame(KPlayer* player)
{
  kDebug() << "KPlayerSetupWidget::slotPlayerJoinedGame: " << player->name()
      << " from " << ((GameLogic::Player*)player)->getNation()->name();
  for (int i = 0 ; i < nationCombo->count(); i++)
  {
    if (nationCombo->itemText(i) == m_nationsNames[((GameLogic::Player*)player)->getNation()->name()])
    {
      nationCombo->removeItem(i);
      break;
    }
  }
  slotNameEdited(nameLineEdit->text());
}

void KPlayerSetupWidget::slotNationChanged()
{
  kDebug() << "KPlayerSetupWidget::slotNationChanged " << nationCombo->currentText();
  if (nationCombo->currentText().isEmpty())
  {
    return;
  }
  GameLogic::Nationality* nation = m_onu->nationNamed(m_nationsNames[nationCombo->currentText()]);
//   kDebug() << "nation = " << nation;
  nameLineEdit-> setText(nation->leaderName());
  slotNameEdited(nameLineEdit->text());
}

bool KPlayerSetupWidget::isAvailable(QString nationName)
{
  foreach (NewPlayerData* player, m_newGameSetup->players())
  {
    if (player->nation() == nationName)
    {
      return false;
    }
  }
  return true;
}

void KPlayerSetupWidget::slotNameEdited(const QString& text)
{
  kDebug() << text.trimmed();
  bool found = false;
  foreach (NewPlayerData* player, m_newGameSetup->players())
  {
    if (player->name() == text.trimmed())
    {
      found = true;
      break;
    }
  }
  if (found || text.trimmed().isEmpty())
  {
    QString message = i18n("Name already in use. Please choose another one.");
    if (text.trimmed().isEmpty())
      message = i18n("Empty name. Please choose another one.");
    nextButton->setEnabled(false);
    nameLineEdit->setStyleSheet("background: red");
    messageLabel->setText(message);
    messageLabel->show();
  }
  else
  {
    nextButton->setEnabled(true);
    nameLineEdit->setStyleSheet("");
    messageLabel->hide();
  }
}

void KPlayerSetupWidget::init(NewPlayerData* player)
{
  kDebug() << (void*)player;
  QString labelString = i18n("Player Number %1, please type in your name and choose your nation:",m_newGameSetup->players().size());
  TextLabel1-> setText(labelString);

  /// @TODO set the correct nation and password and computer state
  
  fillNationsCombo();
  if (m_newGameSetup->networkGameType() == GameLogic::GameAutomaton::None)
  {
    passwordLabel->hide();
    passwordEdit->hide();
  }
  if (player != 0)
  {
    kDebug() << player->name();
    nationCombo->setCurrentIndex(nationCombo->findText(player->nation()));
    slotNationChanged();
    nameLineEdit->setText(player->name());
  }
  else
  {
    slotNationChanged();
  }
  nameLineEdit->setFocus();
}

} // namespace Ksirk

#include "kplayersetupwidget.moc"
