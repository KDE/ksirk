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

// application specific includes
#include "mainwindow.h"
#include "ksirkSkinEditorConfigDialog.h"
#include "ksirkskineditorsettings.h"

//include files for QT
#include <QDockWidget>
#include <QTreeView>
#include <QPushButton>
#include <QGridLayout>
#include <QString>
#include <QVBoxLayout>
#include <QMovie>
// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandardgameaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <kmenubar.h>
#include <kdebug.h>
#include <ktextedit.h>
#include <phonon/mediaobject.h>
#include <KPushButton>
#include <kglobal.h>
#include <KStatusBar>
#include <KToolBar>
#include <KAction>
#include <KSvgRenderer>
#include <KDialog>
#include <KAboutApplicationDialog>


#include <assert.h>


namespace KsirkSkinEditor
{

MainWindow::MainWindow(QWidget* parent) :
  KXmlGuiWindow(parent)
{
  kDebug() << "MainWindow constructor begin";

  m_dirs = KGlobal::dirs();
//   m_accels.setEnabled(true);
  
  QString iconFileName = m_dirs-> findResource("appdata", "ksirkskineditor.png");
/*  if (iconFileName.isNull())
  {
      KMessageBox::error(0, i18n("Cannot load icon<br>Program cannot continue"), i18n("Error !"));
      exit(2);
  }*/
  QPixmap icon(iconFileName);

//    kDebug() << "Before initActions";
  initActions();

  kDebug() << "Setting up GUI";
  setupGUI();

//    kDebug() << "Before initStatusBar";
  initStatusBar();
  
  menuBar()-> show();
  
  setMouseTracking(true);

}

MainWindow::~MainWindow()
{
  kDebug();
  m_dirs = 0;
}

void MainWindow::initActions()
{
  QAction *action;

  // standard game actions
//   action = KStandardGameAction::gameNew(this, SLOT(slotNewGame()), this);
//   actionCollection()->addAction(action->objectName(), action);
//   action = KStandardGameAction::load(this, SLOT(slotOpenGame()), this);
//   actionCollection()->addAction(action->objectName(), action);
//   m_saveGameAction = KStandardGameAction::save(this, SLOT(slotSaveGame()), this);
//   actionCollection()->addAction(m_saveGameAction->objectName(), m_saveGameAction);
//   action = KStandardGameAction::quit(this, SLOT(close()), this);
//   actionCollection()->addAction(action->objectName(), action);

  action = KStandardAction::zoomIn(this, SLOT(slotZoomIn()), this);
  actionCollection()->addAction(action->objectName(), action);

  action = KStandardAction::zoomOut(this, SLOT(slotZoomOut()), this);
  actionCollection()->addAction(action->objectName(), action);

  KStandardAction::preferences( this, SLOT( optionsConfigure() ), actionCollection() );

}

void MainWindow::initStatusBar()
{
  statusBar()-> setSizeGripEnabled(true);
}

/**
  * Reimplementation of the inherited function called when a window close event arise
  */
bool MainWindow::queryClose()
{
    // TODO : Test si jeu en cours
  return true;
}

void MainWindow::saveXml(std::ostream& xmlStream)
{
  xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  xmlStream << "<ksirkskin formatVersion=\"2.0\" >" << std::endl;
  xmlStream << "</ksirkskin>" << std::endl;
}


void MainWindow::optionsConfigure()
{
  //An instance of your dialog could be already created and could be cached, 
  //in which case you want to display the cached dialog instead of creating 
  //another one 
  if ( KsirkSkinEditorConfigurationDialog::showDialog( "settings" ) ) 
    return; 
 
  //KConfigDialog didn't find an instance of this dialog, so lets create it : 
  KsirkSkinEditorConfigurationDialog* dialog = new KsirkSkinEditorConfigurationDialog(
      this, "settings",
      KsirkSkinEditorSettings::self() ); 

  dialog->show();
}

void MainWindow::slotZoomIn()
{
  kDebug();
}

void MainWindow::slotZoomOut()
{
  kDebug();
}

void MainWindow::slotShowAboutApplication()
{
  KAboutApplicationDialog dialog(KGlobal::mainComponent().aboutData(), this);
  dialog.exec();
}

bool MainWindow::queryExit()
{
//   kDebug() << "Writing skin m_config: " << m_automaton->skin();
  KGlobal::config()->sync();
  return true;
}


} // closing namespace

#include "mainwindow.moc"
