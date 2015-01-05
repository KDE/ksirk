/***************************************************************************
 *                          newGameDialogImpl.cpp
 *                             -------------------
 *    begin                : Wed Feb 23 2005
 *    copyright            : (C) 2005 by Gael de Chalendar <kleag@free.fr>
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
#include "newGameDialogImpl.h"
#include "ksirksettings.h"

#include "GameLogic/onu.h"

#include <KLocale>
#include <KDebug>
#include <KStandardDirs>
#include <KMessageBox>
#include <KConfigDialog>
#include <knewstuff3/downloaddialog.h>

#include <qdir.h>
#include <qstringlist.h>
#include <qpushbutton.h>

using namespace Ksirk::GameLogic;

namespace Ksirk
{

NewGameWidget::NewGameWidget(NewGameSetup* newGameSetup, QWidget *parent) :
    QWidget(parent),
    Ui::NewGameDialog(),
    m_newGameSetup(newGameSetup)
{
  kDebug();
  setupUi(this);
  
  QObject::connect(nextButton, SIGNAL(clicked()), this, SLOT(slotOK()) );
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(slotCancel()) );
  QObject::connect(skinCombo, SIGNAL(activated(int)), this, SLOT(slotSkinChanged(int)) );
  QObject::connect(ghnsbutton, SIGNAL(clicked()), this, SLOT(slotGHNS()) );
  QObject::connect(tcpPortEntry,SIGNAL(valueChanged(int)),this, SLOT(slotTcpPortEdited(int)));
}

void NewGameWidget::init(const QString& skin, GameAutomaton::NetworkGameType netGameType)
{
  kDebug() << "Skin got: " << skin << " ; network=" << netGameType;
  m_newGameSetup->setSkin(skin);
  if (netGameType != GameAutomaton::None)
  {
    m_newGameSetup->setNetworkGameType(netGameType);
    localPlayersNumberLabel->show();
    localPlayersNumberEntry->show();
    if (netGameType == GameAutomaton::Socket)
    {
      tcpPortLabel->show();
      tcpPortEntry->show();
    }
    if (netGameType == GameAutomaton::Jabber)
    {
      tcpPortLabel->hide();
      tcpPortEntry->hide();
    }
    m_newGameSetup->setNbNetworkPlayers(1);
    localPlayersNumberEntry->setValue(m_newGameSetup->nbNetworkPlayers());
  }
  else
  {
    m_newGameSetup->setNetworkGameType(GameAutomaton::None);
    localPlayersNumberLabel->hide();
    localPlayersNumberEntry->hide();
    tcpPortLabel->hide();
    tcpPortEntry->hide();
    m_newGameSetup->setNbNetworkPlayers(0);
  }

  m_newGameSetup->setSkin(skin);

  fillSkinsCombo();
}

NewGameWidget::~NewGameWidget()
{
}

void NewGameWidget::slotOK()
{
  kDebug() << "  skin is " << m_newGameSetup->worlds()[skinCombo->currentText()]->skin();
//   m_networkGame  = networkGameCheckBox->isChecked();
  m_newGameSetup->setSkin(m_newGameSetup->worlds()[skinCombo->currentText()]->skin());
  m_newGameSetup->setNbPlayers(playersNumberEntry->value());
  m_newGameSetup->setNbNetworkPlayers(m_newGameSetup->automaton()->networkGameType() == GameAutomaton::None?0:playersNumberEntry->value()-localPlayersNumberEntry->value());
  m_newGameSetup->setUseGoals(radioGoal->isChecked());
  emit newGameOK();
}

void NewGameWidget::slotCancel()
{
  kDebug();
  emit newGameKO();
}

void NewGameWidget::fillSkinsCombo()
{
  kDebug() << "Filling skins combo";

  skinCombo->clear();
  qDeleteAll(m_newGameSetup->worlds());
  
  KStandardDirs *m_dirs = KGlobal::dirs();
  QStringList skinsDirs = m_dirs->findDirs("appdata","skins");
  kDebug() << skinsDirs;
  uint skinNum = 0;
  uint currentSkinNum = 0;
  foreach (const QString &skinsDirName, skinsDirs)
  {
  //   QString skinsDirName = m_dirs->findResourceDir("appdata", "skins/skinsdir");
    if (skinsDirName.isEmpty())
    {
      KMessageBox::error(0,
                        i18n("Skins directory not found - Verify your installation\nProgram cannot continue"),
                        i18n("Fatal Error!"));
      exit(2);
    }
    kDebug() << "Got skins dir name: " << skinsDirName;
    QDir skinsDir(skinsDirName);
    QStringList skinsDirsNames = skinsDir.entryList(QStringList("[a-zA-Z]*"), QDir::Dirs);

    foreach (const QString& name, skinsDirsNames)
    {
      kDebug() << "Got skin dir name: " << name;
      QDir skinDir(skinsDirName + name);
      if (skinDir.exists())
      {
        kDebug() << "Got skin dir: " << skinDir.dirName();
        GameLogic::ONU* world = new GameLogic::ONU(m_newGameSetup->automaton(),skinsDirName + skinDir.dirName() + "/Data/world.desktop");
        if (!world->skin().isEmpty())
        {
          skinCombo->addItem(i18n(world->name().toUtf8().data()));
          m_newGameSetup->worlds()[i18n(world->name().toUtf8().data())] = world;
          if (QString(QLatin1String("skins/")+skinDir.dirName()) == m_newGameSetup->skin())
          {
            kDebug() << "Setting currentSkinNum to " << skinNum;
            currentSkinNum = skinNum;
          }
        }
        else
        {
          delete world;
        }
      }
    }
  }
  skinCombo->setCurrentIndex(currentSkinNum);
  slotSkinChanged(currentSkinNum);
}

void NewGameWidget::slotSkinChanged(int skinNum)
{
    kDebug() << "NewGameDialogImpl::slotSkinChanged " 
              << skinNum << " ; " << skinCombo->currentText() 
              << " ; " << m_newGameSetup->worlds()[skinCombo->currentText()]->name() << " ; "
              << m_newGameSetup->worlds()[skinCombo->currentText()]->description();
              skinDescLabel->setText(i18n(m_newGameSetup->worlds()[skinCombo->currentText()]->description().toUtf8().data()));
              skinSnapshotPixmap->setPixmap(m_newGameSetup->worlds()[skinCombo->currentText()]->snapshot());
    
    playersNumberEntry->setMinimum(2);
    playersNumberEntry->setMaximum(m_newGameSetup->worlds()[skinCombo->currentText()]->getNationalities().size());
    localPlayersNumberEntry->setMinimum(1);
    localPlayersNumberEntry->setMaximum(m_newGameSetup->worlds()[skinCombo->currentText()]->getNationalities().size()-1);
}

void NewGameWidget::slotGHNS()
{
  // Bug 281294. No need to touch ConfigDialog.
  // Skins are handled by KNS3 dialog.
  // if ( KConfigDialog::showDialog("settings") )
  // {
  //   return;
  // }
  kDebug();
  KNS3::DownloadDialog dialog(this);
  dialog.exec();
  
  fillSkinsCombo();
/*  KConfigDialog *dialog = new KConfigDialog(this, "settings", KsirkSettings::self());
  KGameThemeSelector* kgts = new KGameThemeSelector(dialog, KsirkSettings::self());
  dialog->addPage(kgts, i18n("Theme"), "games-config-theme");*/
/*  connect(dialog, SIGNAL(settingsChanged(QString)), view, SLOT(settingsChanged()));
  connect(dialog, SIGNAL(hidden()), view, SLOT(resumeFromConfigure()));*/
//   dialog->show();
  
}

void NewGameWidget::slotTcpPortEdited(int port)
{
  m_newGameSetup->setTcpPort(port);
}

void NewGameWidget::slotNbNetworkPlayersEdited(int)
{
}

}


