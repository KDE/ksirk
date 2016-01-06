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
#include <KAboutData>
#include <QDebug>
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
  qDebug() << "Hello KsirK";

  KAboutData aboutData(QStringLiteral("ksirk"),
                       i18n("KsirK"),
                       QStringLiteral("5.0.0"),
                       i18n("KsirK - World Domination Strategy Game"),
                       KAboutLicense::GPL,
                       i18n("(c) 2002-2015, Gaël de Chalendar\n"),
                       i18n("For help and user manual, please see\nthe KsirK web site."),
                       QStringLiteral("http://games.kde.org/game.php?game=ksirk"));

  aboutData.addAuthor(i18n("Gael de Chalendar aka Kleag"),QStringLiteral(), QStringLiteral("kleag@free.fr"));
  aboutData.addAuthor(i18n("Nemanja Hirsl"),i18n("Current maintainer"), QStringLiteral("nemhirsl@gmail.com"));
  aboutData.addAuthor(i18n("Robin Doer"));
  aboutData.addAuthor(i18n("Albert Astals Cid"));
  aboutData.addAuthor(i18n("Michal Golunski (Polish translation)"),QStringLiteral(), QStringLiteral("michalgolunski@o2.pl"));
  aboutData.addAuthor(i18n("French students of the 'IUP ISI 2007-2008':"));
  aboutData.addAuthor(i18n("&nbsp;&nbsp;Anthony Rey<br/>&nbsp;&nbsp;Benjamin Lucas<br/>&nbsp;&nbsp;Benjamin Moreau<br/>&nbsp;&nbsp;Gaël Clouet<br/>&nbsp;&nbsp;Guillaume Pelouas<br/>&nbsp;&nbsp;Joël Marco<br/>&nbsp;&nbsp;Laurent Dang<br/>&nbsp;&nbsp;Nicolas Linard<br/>&nbsp;&nbsp;Vincent Sac"));

  aboutData.setOrganizationDomain(QByteArray("kde.org"));
  aboutData.setProductName(QByteArray("ksirk"));

  QApplication app(argc, argv);

  KLocalizedString::setApplicationDomain("ksirk");

  app.setWindowIcon(QIcon::fromTheme(QStringLiteral("ksirk")));
  KAboutData::setApplicationData(aboutData);

  QCommandLineParser parser;
  parser.addVersionOption();
  parser.addHelpOption();
  aboutData.setupCommandLine(&parser);
  parser.process(app);
  aboutData.processCommandLine(&parser);

  if (app.isSessionRestored())
  {
      RESTORE(Ksirk::KGameWindow);
  }
  else
  {
    qDebug() << "Creating main window";
    Ksirk::KGameWindow *ksirk = new Ksirk::KGameWindow();
    ksirk->show();
  }

  qDebug() << "Executing app";
  return app.exec();
}
