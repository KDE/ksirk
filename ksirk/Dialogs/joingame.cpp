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
#include "joingame.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/player.h"
#include "GameLogic/nationality.h"
#include "GameLogic/onu.h"
#include "Sprites/skinSpritesData.h"
#include "Sprites/flagsprite.h"

#include <QString>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPixmap>
#include <QPainter>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kpassworddialog.h>
#include <kmessagebox.h>

#define _XOPEN_SOURCE_
// #include <unistd.h>

namespace Ksirk
{

  JoinGameDialog::JoinGameDialog(GameLogic::GameAutomaton* automaton,
                                  QString& host,
                                  int& port,
                                  QWidget *parent) :
  QDialog(parent), Ui::JoinGameDialog(),
  m_automaton(automaton), m_host(host),
    m_port(port)
{
  kDebug();
  setupUi(this);
  hostEdit-> setText(m_host);
  portEdit-> setText(QString::number(m_port));
  QObject::connect(m_automaton, SIGNAL(clicked()), this, SLOT(slotOK()) );
  
  connect(automaton,SIGNAL(newJabberGame(const QString&, const QString&, int, const QString&)), this,SLOT(slotNewJabberGame(const QString&, const QString&, int, const QString&)));
  
  connect(jabberTable, SIGNAL(cellClicked(int,int)), this, SLOT(slotCellClicked(int,int)));

  hostEdit->setFocus();

  QStringList headers;
  headers.push_back("Nickname");
  headers.push_back("Host");
  headers.push_back("Port");
  headers.push_back("Skin");
  jabberTable->setHorizontalHeaderLabels(headers); 
}

JoinGameDialog::~JoinGameDialog(){
  hide();
}

void JoinGameDialog::slotNewJabberGame(const QString& nick,
                        const QString& host,
                        int port,
                        const QString& skin
                        )
{
  kDebug() << nick << host << port << skin;
  jabberTable->setSortingEnabled(false);
  jabberTable->setRowCount(jabberTable->rowCount()+1);
  
  QTableWidgetItem *newItem = new QTableWidgetItem(nick);
  jabberTable->setItem(jabberTable->rowCount()-1,0,newItem);
  newItem = new QTableWidgetItem(host);
  jabberTable->setItem(jabberTable->rowCount()-1,1,newItem);
  newItem = new QTableWidgetItem(QString::number(port));
  jabberTable->setItem(jabberTable->rowCount()-1,2,newItem);
  newItem = new QTableWidgetItem(skin);
  jabberTable->setItem(jabberTable->rowCount()-1,3,newItem);
  
  jabberTable->setSortingEnabled(false);
}

void JoinGameDialog::accept()
{
  kDebug();

  m_host = hostEdit->text();
  m_port = hostEdit->text().toInt();
  QDialog::accept();
}

void JoinGameDialog::reject() {
//   kDebug() << "I not allow to close the dialog!" << endl;
  QDialog::reject();
}

void JoinGameDialog::slotCellClicked(int row, int column)
{
  Q_UNUSED(column);
  kDebug() << row;
  m_host = jabberTable->item(row,1)->text();
  m_port = jabberTable->item(row,2)->text().toInt();
  kDebug() << m_host << m_port;
  
  hostEdit->setText(m_host);
  portEdit->setText(QString::number(m_port));
}

}

#include "joingame.moc"
