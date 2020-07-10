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
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
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

#include <KLocalizedString>
#include "ksirk_debug.h"
#include <QStandardPaths>
#include <KMessageBox>
#include <KConfigDialog>
#include <downloaddialog.h>

#include <QDir>
#include <QStringList>
#include <QPushButton>

using namespace Ksirk::GameLogic;

namespace Ksirk
{

NewGameWidget::NewGameWidget(NewGameSetup* newGameSetup, QWidget *parent) :
    QWidget(parent),
    Ui::NewGameDialog(),
    m_newGameSetup(newGameSetup)
{
  qCDebug(KSIRK_LOG);
  setupUi(this);
  
  QObject::connect(nextButton, &QAbstractButton::clicked, this, &NewGameWidget::slotOK );
  QObject::connect(cancelButton, &QAbstractButton::clicked, this, &NewGameWidget::slotCancel );
  QObject::connect(skinCombo, SIGNAL(activated(int)), this, SLOT(slotSkinChanged(int)) );
  QObject::connect(ghnsbutton, &QAbstractButton::clicked, this, &NewGameWidget::slotGHNS );
  QObject::connect(tcpPortEntry,SIGNAL(valueChanged(int)),this, SLOT(slotTcpPortEdited(int)));
}

void NewGameWidget::init(const QString& skin, GameAutomaton::NetworkGameType netGameType)
{
  qCDebug(KSIRK_LOG) << "Skin got: " << skin << " ; network=" << netGameType;
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
  qCDebug(KSIRK_LOG) << "  skin is " << m_newGameSetup->worlds()[skinCombo->currentText()]->skin();
//   m_networkGame  = networkGameCheckBox->isChecked();
  m_newGameSetup->setSkin(m_newGameSetup->worlds()[skinCombo->currentText()]->skin());
  m_newGameSetup->setNbPlayers(playersNumberEntry->value());
  m_newGameSetup->setNbNetworkPlayers(m_newGameSetup->automaton()->networkGameType() == GameAutomaton::None?0:playersNumberEntry->value()-localPlayersNumberEntry->value());
  m_newGameSetup->setUseGoals(radioGoal->isChecked());
  emit newGameOK();
}

void NewGameWidget::slotCancel()
{
  qCDebug(KSIRK_LOG);
  emit newGameKO();
}

void NewGameWidget::fillSkinsCombo()
{
  qCDebug(KSIRK_LOG) << "Filling skins combo";

  skinCombo->clear();
  qDeleteAll(m_newGameSetup->worlds());
  
  QStringList skinsDirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, "skins", QStandardPaths::LocateDirectory);
  qCDebug(KSIRK_LOG) << skinsDirs;
  uint skinNum = 0;
  uint currentSkinNum = 0;
  foreach (const QString &skinsDirName, skinsDirs)
  {
    if (skinsDirName.isEmpty())
    {
      KMessageBox::error(0,
                        i18n("Skins directory not found - Verify your installation\nProgram cannot continue"),
                        i18n("Fatal Error!"));
      exit(2);
    }
    qCDebug(KSIRK_LOG) << "Got skins dir name: " << skinsDirName;
    QDir skinsDir(skinsDirName);
    QStringList skinsDirsNames = skinsDir.entryList(QStringList("[a-zA-Z]*"), QDir::Dirs);

    foreach (const QString& name, skinsDirsNames)
    {
      qCDebug(KSIRK_LOG) << "Got skin dir name: " << name;
      QDir skinDir(skinsDirName + '/' + name);
      if (skinDir.exists())
      {
        qCDebug(KSIRK_LOG) << "Got skin dir: " << skinDir.dirName();
        GameLogic::ONU* world = new GameLogic::ONU(m_newGameSetup->automaton(),skinsDirName + '/' + skinDir.dirName() + "/Data/world.desktop");
        if (!world->skin().isEmpty())
        {
          skinCombo->addItem(i18n(world->name().toUtf8().data()));
          m_newGameSetup->worlds()[i18n(world->name().toUtf8().data())] = world;
          if (QString(QLatin1String("skins/")+skinDir.dirName()) == m_newGameSetup->skin())
          {
            qCDebug(KSIRK_LOG) << "Setting currentSkinNum to " << skinNum;
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
    qCDebug(KSIRK_LOG) << "NewGameDialogImpl::slotSkinChanged " 
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
  qCDebug(KSIRK_LOG);
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


