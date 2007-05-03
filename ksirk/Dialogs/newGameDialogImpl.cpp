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

#include "GameLogic/onu.h"

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <qdir.h>
#include <qstringlist.h>
#include <qpushbutton.h>

namespace Ksirk
{

NewGameDialogImpl::NewGameDialogImpl(
      GameLogic::GameAutomaton* automaton,
      bool& ok,
      unsigned int& nbPlayers, 
      unsigned int maxPlayers,
      QString &skin,
      bool& networkGame,
      bool& useGoals,
      QWidget *parent) :
    QDialog(parent),
    Ui::NewGameDialog(),
  m_automaton(automaton),
  m_ok(ok), m_nbPlayers(nbPlayers),
  m_skin(skin), m_networkGame(networkGame), m_useGoals(useGoals)
{
  kDebug() << "Skin got by NewGameDialog: " << m_skin 
    << " ; maxPlayers=" << maxPlayers << endl;
  setupUi(this);
  playersNumberEntry->setMinimum(2);
  playersNumberEntry->setMaximum(maxPlayers);
  fillSkinsCombo();
  QObject::connect(buttonbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(slotCancel()) );
  QObject::connect(buttonbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(slotOK()) );
  QObject::connect(buttonbox->button(QDialogButtonBox::Help), SIGNAL(clicked()), this, SLOT(slotHelp()) );
  QObject::connect(skinCombo, SIGNAL(activated(int)), this, SLOT(slotSkinChanged(int)) );
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
  kDebug() << "KPlayerSetupDialog slotOk" << endl;
  m_nbPlayers = playersNumberEntry->value();
  m_skin = m_worlds[skinCombo->currentText()]->skin();
  kDebug() << "  m_skin is " << m_skin << endl;
  m_networkGame  = networkGameCheckBox->isChecked();
  m_useGoals = (radioGoal->isChecked());
  m_ok = true;
  accept();
}

void NewGameDialogImpl::slotCancel()
{
  kDebug() << "KPlayerSetupDialog slotCancel" << endl;
  m_ok = false;
  reject();
}

/** @todo implements a help */
void NewGameDialogImpl::slotHelp()
{
  kDebug() << "KPlayerSetupDialog slotHelp not already implemented" << endl;
  KMessageBox::sorry(this, i18n("Help currently unavailable."),i18n("KsirK - No help !"));
}

/** @todo Add a thumbnail of the skin map for example */
void NewGameDialogImpl::fillSkinsCombo()
{
  kDebug() << "Filling skins combo" << endl;
  KStandardDirs *m_dirs = KGlobal::dirs();
  QString skinsDirName = m_dirs-> findResourceDir("appdata", "skins/skinsdir");
  if (skinsDirName.isEmpty())
  {
    KMessageBox::error(0,
                       i18n("Skins directory not found - Verify your installation\nProgram cannot continue"),
                       i18n("Fatal Error !"));
    exit(2);
  }
  skinsDirName += "skins/";
  kDebug() << "Got skins dir name: " << skinsDirName << endl;
  QDir skinsDir(skinsDirName);
  QStringList skinsDirsNames = skinsDir.entryList(QStringList("[a-zA-Z]*"), QDir::Dirs);
  
  uint skinNum = 0;
  uint currentSkinNum = 0;
  QStringList::iterator it, it_end;
  it = skinsDirsNames.begin(); it_end = skinsDirsNames.end();
  for (; it != it_end; it++, skinNum++)
  {
    kDebug() << "Got skin dir name: " << *it << endl;
    QDir skinDir(skinsDirName + *it);
    if (skinDir.exists())
    {
      kDebug() << "Got skin dir: " << skinDir.dirName() << endl;
      GameLogic::ONU* world = new GameLogic::ONU(m_automaton,skinsDirName + skinDir.dirName() + "/Data/onu.xml");
      skinCombo->addItem(i18n(world->name().toUtf8().data()));
      m_worlds[i18n(world->name().toUtf8().data())] = world;
      if (QString("skins/")+skinDir.dirName() == m_skin)
      {
        kDebug() << "Setting currentSkinNum to " << skinNum << endl;
        currentSkinNum = skinNum;
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
              << m_worlds[skinCombo->currentText()]->description() << endl;
    skinDescLabel->setText(i18n(m_worlds[skinCombo->currentText()]->description().toUtf8().data()));
    skinSnapshotPixmap->setPixmap(m_worlds[skinCombo->currentText()]->snapshot());
}

}

#include "newGameDialogImpl.moc"
