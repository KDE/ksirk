/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : mer jui 11 22:27:28 EDT 2001
    copyright            : (C) 2001 by Gaël de Chalendar
    email                : Gael.de.Chalendar@free.fr

This is the standard main function of a KDE application simplified for KsirK

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kgamewin.h"
#include "GameLogic/gameautomaton.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kdebug.h>
#include <KApplication>
#include <KToolBar>

static const char *description =
    I18N_NOOP("KsirK");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
    

static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("file to open"), 0 },
  { 0, 0, 0 }
    // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{
  kDebug() << "Hello World!" << endl;
    KAboutData aboutData( "ksirk", I18N_NOOP("KsirK"),
        VERSION, description, KAboutData::License_GPL,
        "(c) 2002-2005, Gaël de Chalendar\nFor help and user manuel, please see \nThe KsirK Web site (http://home.gna.org/ksirk)", 0, 0, "kleag@free.fr");
    aboutData.addAuthor("Gael de Chalendar aka Kleag",0, "kleag@free.fr");
    aboutData.addAuthor("Robin Doer",0, "");
    aboutData.addAuthor("Albert Astals Cid",0, "");
    aboutData.addAuthor("Michal Golunski (Polish translation)",0, "michalgolunski@o2.pl");
    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication app;
 
    if (app.isSessionRestored())
    {
        RESTORE(Ksirk::KGameWindow);
    }
    else 
    {
      kDebug() << "Creating main window" << endl;
      Ksirk::KGameWindow *ksirk = new Ksirk::KGameWindow();
//       connect(app,SIGNAL(lastWindowClosed()),app,SLOT(quit()));
//         app.setMainWidget(ksirk);
        ksirk->show();
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        args->clear();
    }
    kDebug() << "Executing app" << endl;
    return app.exec();
}  
