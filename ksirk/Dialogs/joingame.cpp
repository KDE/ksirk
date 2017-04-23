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

#include <QString>
#include <QLineEdit>

#include <KLocalizedString>
#include "ksirk_debug.h"

namespace Ksirk
{
  JoinGameDialog::JoinGameDialog(QString& host,
                                  int& port,
                                  QWidget *parent) :
  QDialog(parent), Ui::JoinGameDialog(),
    m_host(host),
    m_port(port)
{
  qCDebug(KSIRK_LOG);
  setupUi(this);
  hostEdit-> setText(m_host);
  portEdit-> setText(QString::number(m_port));

  hostEdit->setFocus();
}

JoinGameDialog::~JoinGameDialog(){
  hide();
}

void JoinGameDialog::accept()
{
  qCDebug(KSIRK_LOG);

  m_host = hostEdit->text();
  m_port = hostEdit->text().toInt();
  QDialog::accept();
}

void JoinGameDialog::reject() {
//   qCDebug(KSIRK_LOG) << "I not allow to close the dialog!" << endl;
  QDialog::reject();
}


}


