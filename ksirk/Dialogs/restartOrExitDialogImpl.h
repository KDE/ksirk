/***************************************************************************
                          restartOrExitDialogImpl.h  -  description
                             -------------------
    begin                : Sun Jul 16 2006
    copyright            : (C) 2006 by Gaël de Chalendar (aka Kleag)
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define KDE_NO_COMPAT


#include "ui_restartOrExitDialog.h"

#ifndef KSIRK_RESTARTOREXITDIALOGIMPL_H
#define KSIRK_RESTARTOREXITDIALOGIMPL_H

namespace Ksirk 
{

/**
  * This is the implementation of the "restart or exit" dialog made with
  * QT Designer
  * @author Gaël de Chalendar
  */
class RestartOrExitDialogImpl : public QDialog, public Ui::RestartOrExitDialog
{
  Q_OBJECT
public:
  RestartOrExitDialogImpl(
      const QString& label,
      QWidget *parent=0, 
      const char *name=0);
  virtual ~RestartOrExitDialogImpl();

private:
    
public slots:
    virtual void slotNewGame();
    virtual void slotDoNothing();
    virtual void slotExit();
  
};

} // closing namespace Ksirk

#endif // KSIRK_RESTARTOREXITDIALOGIMPL_H

