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
#include "kplayersetupdialog.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/player.h"
#include "GameLogic/nationality.h"
#include "GameLogic/onu.h"
#include "Sprites/skinSpritesData.h"

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
#include <unistd.h>

namespace Ksirk
{

KPlayerSetupDialog::KPlayerSetupDialog( GameLogic::GameAutomaton* automaton,
                                        GameLogic::ONU* onu,
                                        unsigned int playerNumber,
                                        QString& playerName,
                                        bool network, QString& password,
                                        bool &computerPlayer,
                                        std::map< QString, QString >& nations, 
                                        QString& nationName,
                                        QWidget *parent) :
  QDialog(parent), Ui::QPlayerSetupDialog(),
  m_automaton(automaton), name(playerName),
    computer(computerPlayer), m_nationName(nationName), 
  m_nations(nations), m_onu(onu), m_nationsNames(), number(playerNumber), 
  m_password(password)
{
  kDebug() << "KPlayerSetupDialog constructor" << endl;
  setupUi(this);
  QString labelString = i18n("Player Number %1, please type in your name<BR>and choose your nation:",number);
  TextLabel1-> setText(labelString);
  fillNationsCombo();
  if (network)
    passwordEdit->setEnabled(true);
  QObject::connect((const QObject *)PushButton1, SIGNAL(clicked()), this, SLOT(slotOK()) );
  
  kDebug() << "KPlayerSetupDialog connecting to playerJoinedGame" << endl;
  connect(automaton,SIGNAL(signalPlayerJoinedGame(KPlayer*)),
          this,SLOT(slotPlayerJoinedGame(KPlayer*)));
  
  LineEdit2->setFocus();
  
  connect(nationCombo, SIGNAL(activated(int)), this, SLOT(slotNationChanged()));

  kDebug() << "KPlayerSetupDialog constructor done" << endl;
}

KPlayerSetupDialog::~KPlayerSetupDialog(){
  hide();
}

void KPlayerSetupDialog::slotOK()
{
  kDebug() << "KPlayerSetupDialog::slotOk" << endl;

  if (!testEmptyUserName(LineEdit2->text().trimmed())) {
    // Empty name, but required, do not close dialog
    KMessageBox::error(this,
      "<html><center><b>" + i18n("Error") + "</b><br>" +
      i18n("Player %1, you have to choose a name!",number) +
          "</center></html>", i18n("Error"));
  }
  else if (!testUniqueUserName(LineEdit2->text().trimmed())) {
    // Name is not unique
    KMessageBox::error(this,
      "<html><center><b>" + i18n("Error") + "</b><br>" +
      i18n("Player %1, you have to choose another name!<br>%2 is not unique.",number,LineEdit2->text()) +
          "</center></html>", i18n("Error"));
  }
  else {
    name = LineEdit2-> text();
//     kDebug() << "Got name " << name << endl;
    computer = (CheckBox1-> checkState() == Qt::Checked);
//     kDebug() << "computer? : " << computer << endl;
    m_nationName = m_nationsNames[nationCombo->currentText()];
// @toport
//     m_password = QString(crypt(passwordEdit->password(),"T6"));

    accept();
  }
}

void KPlayerSetupDialog::reject() {
//   kDebug() << "I not allow to close the dialog!" << endl;
}

void KPlayerSetupDialog::fillNationsCombo()
{
  kDebug() << "Filling nations combo" << endl;
  KStandardDirs *m_dirs = KGlobal::dirs();

  std::map< QString, QString >::const_iterator nationsIt, nationsIt_end;
  nationsIt = m_nations.begin(); nationsIt_end = m_nations.end();
  
  GameLogic::Nationality* nation = m_onu->nationNamed((*nationsIt).first);
  LineEdit2-> setText(nation->leaderName());

  for (; nationsIt != nationsIt_end; nationsIt++)
  {
//     kDebug() << "Adding nation " << i18n((*nationsIt).first) << " / " << (*nationsIt).second << endl;
//     load image
    QSize size(
        m_onu->renderer()->boundsOnElement((*nationsIt).second).width()/Sprites::SkinSpritesData::single().intData("flag-frames"),
        m_onu->renderer()->boundsOnElement((*nationsIt).second).height());
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter p(&image);
    m_onu->renderer()->render(&p, (*nationsIt).second);
    QPixmap allpm = QPixmap::fromImage(image);
    QPixmap flag = allpm.copy(0, 0, size.width(), size.height());


//     get name
    QString name = i18n((*nationsIt).first.toUtf8().data());
    m_nationsNames.insert(std::make_pair(name,(*nationsIt).first)); 
//     fill a combo entry
    nationCombo->addItem(QIcon(flag),name);
  }
  
  kDebug() << "Nations combo filled" << endl;
}

void KPlayerSetupDialog::slotPlayerJoinedGame(KPlayer* player)
{
//   kDebug() << "KPlayerSetupDialog::slotPlayerJoinedGame: " << player->name() 
//       << " from " << ((GameLogic::Player*)player)->getNation()->name() << endl;
  for (int i = 0 ; i < nationCombo->count(); i++)
  {
    if (nationCombo->itemText(i) == m_nationsNames[((GameLogic::Player*)player)->getNation()->name()])
    {
      nationCombo->removeItem(i);
      break;
    }
  }
}

inline bool KPlayerSetupDialog::testEmptyUserName(const QString& name) const {
  return (name != "");
}

inline bool KPlayerSetupDialog::testUniqueUserName(const QString& name) const {
  GameLogic::PlayersArray::const_iterator it =
    m_automaton->playerList()->constBegin();
  const GameLogic::PlayersArray::const_iterator it_end =
    m_automaton->playerList()->constEnd();

  for (; it != it_end; it++) {
    if ((*it)->name() == name) {
      // Player with "name" already registered
      return false;
    }
  }

  return true;
}

void KPlayerSetupDialog::slotNationChanged()
{
//   kDebug() << "KPlayerSetupDialog::slotNationChanged " << nationCombo->currentText() << endl;
  GameLogic::Nationality* nation = m_onu->nationNamed(m_nationsNames[nationCombo->currentText()]);
//   kDebug() << "nation = " << nation << endl;
  LineEdit2-> setText(nation->leaderName());
}


}

#include "kplayersetupdialog.moc"
