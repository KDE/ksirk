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
#include <KGameThemeSelector>
#include <knewstuff2/engine.h>

#include <qdir.h>
#include <qstringlist.h>
#include <qpushbutton.h>

namespace Ksirk
{

NewGameDialogImpl::NewGameDialogImpl(QWidget *parent) :
    QDialog(parent),
    Ui::NewGameDialog()
{
  kDebug() << "Skin got by NewGameDialog";
  setupUi(this);
  QObject::connect(buttonbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(slotCancel()) );
  QObject::connect(buttonbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(slotOK()) );
  QObject::connect(buttonbox->button(QDialogButtonBox::Help), SIGNAL(clicked()), this, SLOT(slotHelp()) );
  QObject::connect(skinCombo, SIGNAL(activated(int)), this, SLOT(slotSkinChanged(int)) );
  QObject::connect(ghnsbutton, SIGNAL(clicked()), this, SLOT(slotGHNS()) );
}

void NewGameDialogImpl::init(GameLogic::GameAutomaton* automaton, const QString& skin, bool networkGame)
{
  kDebug() << "Skin got by NewGameDialog: " << skin << " ; network=" << networkGame;
  if (networkGame)
  {
    localPlayersNumberLabel->show();
    localPlayersNumberEntry->show();
  }
  else
  {
    localPlayersNumberLabel->hide();
    localPlayersNumberEntry->hide();
  }
  
  m_automaton = automaton;
  m_skin = skin;

  fillSkinsCombo();
}

NewGameDialogImpl::~NewGameDialogImpl()
{
  QMap<QString, GameLogic::ONU*>::iterator it, it_end;
  it = m_worlds.begin(); it_end = m_worlds.end();
  for (; it != it_end; it++)
  {
//     delete(*it);
  }
}

void NewGameDialogImpl::slotOK()
{
  kDebug() << "KPlayerSetupDialog slotOk";
  kDebug() << "  skin is " << m_worlds[skinCombo->currentText()]->skin();
//   m_networkGame  = networkGameCheckBox->isChecked();
  emit newGameOK(
                 playersNumberEntry->value(),
                 m_worlds[skinCombo->currentText()]->skin(),
                 playersNumberEntry->value()-localPlayersNumberEntry->value(),
                 radioGoal->isChecked());
//   accept();
}

void NewGameDialogImpl::slotCancel()
{
  kDebug() << "KPlayerSetupDialog slotCancel";
  emit newGameKO();
//   reject();
}

/** @todo implements a help */
void NewGameDialogImpl::slotHelp()
{
  kDebug() << "KPlayerSetupDialog slotHelp not already implemented";
  KMessageBox::sorry(this, i18n("Help currently unavailable."),i18n("KsirK - No help !"));
}

void NewGameDialogImpl::fillSkinsCombo()
{
  kDebug() << "Filling skins combo";

  skinCombo->clear();
  foreach (GameLogic::ONU* onu,  m_worlds)
  {
    delete onu;
  }
  
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
                        i18n("Fatal Error !"));
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
        GameLogic::ONU* world = new GameLogic::ONU(m_automaton,skinsDirName + skinDir.dirName() + "/Data/world.desktop");
        if (!world->skin().isEmpty())
        {
          skinCombo->addItem(i18n(world->name().toUtf8().data()));
          m_worlds[i18n(world->name().toUtf8().data())] = world;
          if (QString("skins/")+skinDir.dirName() == m_skin)
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

void NewGameDialogImpl::slotSkinChanged(int skinNum)
{
    kDebug() << "NewGameDialogImpl::slotSkinChanged " 
              << skinNum << " ; " << skinCombo->currentText() 
              << " ; " << m_worlds[skinCombo->currentText()]->name() << " ; " 
              << m_worlds[skinCombo->currentText()]->description();
    skinDescLabel->setText(i18n(m_worlds[skinCombo->currentText()]->description().toUtf8().data()));
    skinSnapshotPixmap->setPixmap(m_worlds[skinCombo->currentText()]->snapshot());
    
    playersNumberEntry->setMinimum(2);
    playersNumberEntry->setMaximum(m_worlds[skinCombo->currentText()]->getNationalities().size());
    localPlayersNumberEntry->setMinimum(1);
    localPlayersNumberEntry->setMaximum(m_worlds[skinCombo->currentText()]->getNationalities().size()-1);
}

void NewGameDialogImpl::slotGHNS()
{
  if ( KConfigDialog::showDialog("settings") )
  {
    return;
  }
  kDebug();
  KNS::Entry::List entries = KNS::Engine::download();
  qDeleteAll(entries);
  fillSkinsCombo();
/*  KConfigDialog *dialog = new KConfigDialog(this, "settings", KsirkSettings::self());
  KGameThemeSelector* kgts = new KGameThemeSelector(dialog, KsirkSettings::self());
  dialog->addPage(kgts, i18n("Theme"), "games-config-theme");*/
/*  connect(dialog, SIGNAL(settingsChanged(const QString &)), view, SLOT(settingsChanged()));
  connect(dialog, SIGNAL(hidden()), view, SLOT(resumeFromConfigure()));*/
//   dialog->show();
  
}


}

#include "newGameDialogImpl.moc"
