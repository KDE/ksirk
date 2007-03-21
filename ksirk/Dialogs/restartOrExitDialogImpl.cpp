#include "restartOrExitDialogImpl.h"

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qradiobutton.h>
#include <qlabel.h>

namespace Ksirk
{

RestartOrExitDialogImpl::RestartOrExitDialogImpl(
      const QString& label,
      QWidget *parent, 
      const char *name) :
  QDialog(parent),
  Ui::RestartOrExitDialog()
{
  setupUi(this);
  messageLabel->setText(label);
  messageLabel->adjustSize();
  QObject::connect((const QObject *)doNothingButton, SIGNAL(clicked()), this, SLOT(slotDoNothing()) );
  QObject::connect((const QObject *)exitButton, SIGNAL(clicked()), this, SLOT(slotExit()) );
  QObject::connect((const QObject *)newGameButton, SIGNAL(clicked()), this, SLOT(slotNewGame()) );
  adjustSize();
}

RestartOrExitDialogImpl::~RestartOrExitDialogImpl()
{
}

void RestartOrExitDialogImpl::slotNewGame()
{
  kDebug() << "KPlayerSetupDialog slotNewGame" << endl;
  close();
}

void RestartOrExitDialogImpl::slotExit()
{
  kDebug() << "KPlayerSetupDialog slotExit" << endl;
  close();
}

/** @todo implements a help */
void RestartOrExitDialogImpl::slotDoNothing()
{
  kDebug() << "KPlayerSetupDialog slotDoNothing" << endl;
  close();
}

}

#include "restartOrExitDialogImpl.moc"
