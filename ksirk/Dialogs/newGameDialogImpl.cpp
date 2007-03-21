#include "newGameDialogImpl.h"

#include "GameLogic/onu.h"

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qradiobutton.h>
#include <qlabel.h>

namespace Ksirk
{

NewGameDialogImpl::NewGameDialogImpl(
      bool& ok,
      unsigned int& nbPlayers, 
      unsigned int maxPlayers,
      QString &skin,
      bool& networkGame,
      bool& useGoals,
      QWidget *parent) :
    QDialog(parent),
    Ui::NewGameDialog(), m_ok(ok), m_nbPlayers(nbPlayers),
  m_skin(skin), m_networkGame(networkGame), m_useGoals(useGoals)
{
  kDebug() << "Skin got by NewGameDialog: " << m_skin 
    << " ; maxPlayers=" << maxPlayers << endl;
  setupUi(this);
  playersNumberEntry->setMinimum(2);
  playersNumberEntry->setMaximum(maxPlayers);
  fillSkinsCombo();
  QObject::connect((const QObject *)buttonCancel, SIGNAL(clicked()), this, SLOT(slotCancel()) );
  QObject::connect((const QObject *)buttonOk, SIGNAL(clicked()), this, SLOT(slotOK()) );
  QObject::connect((const QObject *)buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()) );
  QObject::connect((const QObject *)skinCombo, SIGNAL(activated(int)), this, SLOT(slotSkinChanged(int)) );
}

NewGameDialogImpl::~NewGameDialogImpl()
{
  QMap<QString, GameLogic::ONU*>::iterator it, it_end;
  it = m_worlds.begin(); it_end = m_worlds.end();
  for (; it != it_end; it++)
  {
    delete(*it);
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
  close();
}

void NewGameDialogImpl::slotCancel()
{
  kDebug() << "KPlayerSetupDialog slotCancel" << endl;
  m_ok = false;
  close();
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
      GameLogic::ONU* world = new GameLogic::ONU(skinsDirName + skinDir.dirName() + "/Data/onu.xml");
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
