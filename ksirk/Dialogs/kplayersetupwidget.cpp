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
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
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

#include <KLocalizedString>
#include "ksirk_debug.h"

#define _XOPEN_SOURCE_
// #include <unistd.h>

namespace Ksirk
{

KPlayerSetupWidget::KPlayerSetupWidget(QWidget *parent) :
  QWidget(parent), Ui::QPlayerSetupWidget()
{
  qCDebug(KSIRK_LOG);
  setupUi(this);

  connect(nationCombo, &QComboBox::activated, this, &KPlayerSetupWidget::slotNationChanged);
  connect(nameLineEdit, &QLineEdit::textEdited, this, &KPlayerSetupWidget::slotNameEdited);
  connect(nationCombo, &QComboBox::activated, this, &KPlayerSetupWidget::slotNationChanged);
  connect(nextButton, &QAbstractButton::pressed,this,&KPlayerSetupWidget::slotNext);
  connect(previousButton, &QAbstractButton::pressed,this,&KPlayerSetupWidget::slotPrevious);
  connect(cancelButton, &QAbstractButton::pressed,this,&KPlayerSetupWidget::slotCancel);
  
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
                              QString& password,
                              bool computerPlayer,
                              QMap< QString, QString >& nations,
                              QString & nationName,
                              NewGameSetup* newGameSetup)

{
  qCDebug(KSIRK_LOG) << playerName << nationName;

  m_automaton = automaton;
  m_name = playerName;
  m_computer = computerPlayer;
  m_nationName = nationName;
  m_nations = nations;
  m_onu = onu;
  m_number = playerNumber;
  m_password = password;
  m_newGameSetup = newGameSetup;

  qCDebug(KSIRK_LOG) << "connecting to playerJoinedGame";
  connect(automaton,&KGame::signalPlayerJoinedGame, this, &KPlayerSetupWidget::slotPlayerJoinedGame);
  init();
  qCDebug(KSIRK_LOG) << "constructor done";
}


void KPlayerSetupWidget::slotNext()
{
  qCDebug(KSIRK_LOG) << m_newGameSetup->players().size() << m_newGameSetup->nbPlayers();

  m_name = nameLineEdit-> text().trimmed();
//     qCDebug(KSIRK_LOG) << "Got name " << name;
  m_computer = (isComputerCheckBox-> checkState() == Qt::Checked);
//     qCDebug(KSIRK_LOG) << "computer? : " << computer;
  m_nationName = m_nationsNames[nationCombo->currentText()];
// @toport
//     m_password = QString(crypt(passwordEdit->password(),"T6"));

//     accept();
  if (m_newGameSetup->players().size() < m_newGameSetup->nbPlayers())
  {
    qCDebug(KSIRK_LOG) << "Add new player";
    NewPlayerData* newPlayer = new NewPlayerData(m_name, m_nationName, m_password, m_computer, false);
    m_newGameSetup->players().push_back(newPlayer);
    fillNationsCombo();
    slotNationChanged();
    isComputerCheckBox->setCheckState(Qt::Unchecked);
//     init(m_automaton,m_onu,m_newGameSetup->players().size()+1,"",false,"",false,m_nations,"", m_newGameSetup);
    setLabelText();
    emit next();
  }
  
}

void KPlayerSetupWidget::slotPrevious()
{
  qCDebug(KSIRK_LOG);
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
    setLabelText();
    delete player;
  }
}

void KPlayerSetupWidget::slotCancel()
{
  qCDebug(KSIRK_LOG);
  emit cancel();
}


void KPlayerSetupWidget::fillNationsCombo()
{
  qCDebug(KSIRK_LOG);
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
    qCDebug(KSIRK_LOG) << "Adding nation " << k << " / " << v;
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
  
  qCDebug(KSIRK_LOG) << "Nations combo filled";
}

void KPlayerSetupWidget::slotPlayerJoinedGame(KPlayer* player)
{
  qCDebug(KSIRK_LOG) << "KPlayerSetupWidget::slotPlayerJoinedGame: " << player->name()
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
  qCDebug(KSIRK_LOG) << "KPlayerSetupWidget::slotNationChanged " << nationCombo->currentText();
  if (nationCombo->currentText().isEmpty())
  {
    return;
  }
  GameLogic::Nationality* nation = m_onu->nationNamed(m_nationsNames[nationCombo->currentText()]);
//   qCDebug(KSIRK_LOG) << "nation = " << nation;
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
  qCDebug(KSIRK_LOG) << text.trimmed();
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
  qCDebug(KSIRK_LOG) << (void*)player;
  setLabelText();

  /// @TODO set the correct nation and password and computer state
  
  fillNationsCombo();
  if (m_newGameSetup->networkGameType() == GameLogic::GameAutomaton::None)
  {
    passwordLabel->hide();
    passwordEdit->hide();
  }
  if (player != nullptr)
  {
    qCDebug(KSIRK_LOG) << player->name();
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

void KPlayerSetupWidget::setLabelText()
{
    QString labelString = i18n("Player Number %1, please type in your name and choose your nation:", m_newGameSetup->players().size() + 1);
    TextLabel1->setText(labelString);
}

} // namespace Ksirk

#include "moc_kplayersetupwidget.cpp"
