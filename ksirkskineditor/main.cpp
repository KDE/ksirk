/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : mer jui 11 22:27:28 EDT 2001
    copyright            : (C) 2001 by Gaël de Chalendar
    email                : Gael.de.Chalendar@free.fr

This is the standard main function of a KDE application 

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

***************************************************************************/

#include "mainwindow.h"

#include <KCmdLineArgs>
#include <K4AboutData>
#include <KLocale>
#include <KDebug>
#include <KApplication>
#include <KToolBar>

static const char *description =
    I18N_NOOP("KsirK Skin Editor");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE

int main(int argc, char *argv[])
{
  kDebug() << "Hello World!";
  K4AboutData aboutData(
    "ksirkskineditor",
    0,
    ki18n("KsirK Skin Editor"),
    /*KDE_VERSION_STRING*/"5.0.0",
    ki18n(description),
    K4AboutData::License_GPL,
    ki18n("(c) 2008, Gaël de Chalendar\n"),
    ki18n("For help and user manual, please see\nThe KsirK Web site"),
    "http://games.kde.org/game.php?game=ksirk");
  aboutData.addAuthor(ki18n("Gael de Chalendar aka Kleag"),KLocalizedString(), "kleag@free.fr");

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add("+[File]", ki18n("file to open"));
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;
  //KF5 port: remove this line and define TRANSLATION_DOMAIN in CMakeLists.txt instead
//KLocale::global()->insertCatalog( QLatin1String( "libkdegames" ));
  if (app.isSessionRestored())
  {
      RESTORE(KsirkSkinEditor::MainWindow);
  }
  else
  {
    kDebug() << "Creating main window";
    KsirkSkinEditor::MainWindow *ksirkskineditor = new KsirkSkinEditor::MainWindow();
    ksirkskineditor->show();
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    args->clear();
  }
  kDebug() << "Executing app";
  return app.exec();
}  
