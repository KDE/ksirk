/***************************************************************************
                          kdialogsetupjoueur.cpp  -  description
                             -------------------
    begin                : Thu Jul 19 2001
    copyright            : (C) 2001 by Gaël de Chalendar
    email                : Gael.de.Chalendar@free.fr
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
#define KDE_NO_COMPAT
#include "kwaitedplayersetupdialog.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/player.h"
#include "kgamewin.h"
#include "GameLogic/onu.h"
#include "Sprites/skinSpritesData.h"

#include <qstring.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <QPixmap>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kpassworddialog.h>
#include <kmessagebox.h>

#define _XOPEN_SOURCE_
#include <unistd.h>

namespace Ksirk
{
using namespace GameLogic;

KWaitedPlayerSetupDialog::KWaitedPlayerSetupDialog(
                          GameLogic::GameAutomaton* automaton,
                          QString& password,
                          int& result,
                          QWidget *parent) :
  QDialog(parent), QWaitedPlayerSetupDialog(), 
  m_automaton(automaton),
  m_password(password), m_result(result)
{
  setupUi(this);
  kDebug() << "KWaitedPlayerSetupDialog constructor" << endl;
  fillWaitedPlayersCombo();
  
  QObject::connect((const QObject *)PushButton1, SIGNAL(clicked()), this, SLOT(slotOK()) );
  
}

KWaitedPlayerSetupDialog::~KWaitedPlayerSetupDialog(){
}

void KWaitedPlayerSetupDialog::slotOK()
{
  kDebug() << "slotOk" << endl;
  // toport
//   m_password = QString(crypt(passwordEdit->password(),"T6"));
  m_result = waitedPlayersCombo->currentIndex();
  close();
}

void KWaitedPlayerSetupDialog::fillWaitedPlayersCombo()
{
  kDebug() << "Filling nations combo" << endl;
  KStandardDirs *m_dirs = KGlobal::dirs();

  std::vector<PlayerMatrix>::iterator it, it_end;
  it = m_automaton->game()->waitedPlayers().begin();
  it_end = m_automaton->game()->waitedPlayers().end();
  for (; it != it_end; it++)
  {
    kDebug() << "Adding waited player " << (*it).name << endl;
    QString imgName = m_automaton->game()->theWorld()->nationNamed((*it).nation)->flagFileName();
//     load image
    QPixmap flag;
    QSize size(flag.width()/Sprites::SkinSpritesData::single().intData("flag-frames"),flag.height());
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter p(&image);
    QSvgRenderer renderer;
    renderer.load(imgName);
    renderer.render(&p/*, svgid*/);
    QPixmap allpm = QPixmap::fromImage(image);
    flag = allpm.copy(0, 0, size.width(), size.height());

//     get name
    QString name = (*it).name;
    name += QString(" (");
    name += (i18n((*it).nation.toUtf8().data()));
    name += QString(")");
//     fill a combo entry
    waitedPlayersCombo->addItem(QIcon(flag),name);
  }
  
}

}

#include "kwaitedplayersetupdialog.moc"
