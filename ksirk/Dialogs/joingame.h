/***************************************************************************
                          kdialogsetupjoueur.h  -  description
                             -------------------
    begin                : Thu Jul 19 2001
    copyright            : (C) 2001 by Gaël de Chalendar
    email                : Gael.de.Chalendar@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/
#define KDE_NO_COMPAT

#ifndef JOINGAMEDIALOG_H
#define JOINGAMEDIALOG_H

#include "ui_joingame.h"

#include <QDialog>

class KPlayer;

namespace Ksirk
{

/**
  * This is the implementation of the player configuration dialog made with
  * QT Designer
  * @author Gael de Chalendar
  */
class JoinGameDialog : public QDialog, public Ui::JoinGameDialog
{
  Q_OBJECT
public:
  JoinGameDialog(QString& host,
                 int& port,
                 QWidget *parent=0);
  
  virtual ~JoinGameDialog();

protected:
  void accept();
  void reject();
  
private:
  QString &m_host;
  int& m_port;
};

}

#endif // KPLAYERSETUPDIALOG_H
