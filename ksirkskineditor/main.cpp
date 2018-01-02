/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : mer jui 11 22:27:28 EDT 2001
    copyright            : (C) 2001 by Gaël de Chalendar
    email                : Gael.de.Chalendar@free.fr

This is the standard main function of a KDE application 

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.

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


#include <KAboutData>
#include <KLocalizedString>
#include "ksirkskineditor_debug.h"

#include <KToolBar>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

static const char *description =
    I18N_NOOP("KsirK Skin Editor");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE

int main(int argc, char *argv[])
{
  qCDebug(KSIRKSKINEDITOR_LOG) << "Hello World!";
  QApplication app(argc, argv);
  KLocalizedString::setApplicationDomain("ksirkskineditor");
  KAboutData aboutData(
    "ksirkskineditor",
    i18n("KsirK Skin Editor"),
    /*KDE_VERSION_STRING*/"5.0.0",
    i18n(description),
    KAboutLicense::GPL,
    i18n("(c) 2008, Gaël de Chalendar\n"),
    i18n("For help and user manual, please see\nThe KsirK Web site"),
    QStringLiteral("http://games.kde.org/game.php?game=ksirk"));
  aboutData.addAuthor(i18n("Gael de Chalendar aka Kleag"),QString(), "kleag@free.fr");
  QApplication::setWindowIcon(QIcon::fromTheme("preferences-desktop-locale"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("+[File]"), i18n("file to open")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

  if (app.isSessionRestored())
  {
      RESTORE(KsirkSkinEditor::MainWindow);
  }
  else
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Creating main window";
    KsirkSkinEditor::MainWindow *ksirkskineditor = new KsirkSkinEditor::MainWindow();
    ksirkskineditor->show();
  }
  qCDebug(KSIRKSKINEDITOR_LOG) << "Executing app";
  return app.exec();
}  
