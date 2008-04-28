/* This file is part of KsirK.
   Copyright (C) 2008 Gael Clouet <pelouas@hotmail.fr>

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
*/

/* begin                : Thu Jan 17 2008 */


#ifndef KRIGHTDIALOG_H
#define KRIGHTDIALOG_H

#include "KsirkGlobalDefinitions.h"

#include "GameLogic/onu.h"
#include "GameLogic/country.h"
#include "GameLogic/player.h"
#include <stdlib.h>
#include <QGroupBox>
#include <QGridLayout>
#include <QPointF>
#include <QDockWidget>
#include <QList>
#include <QLabel>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <QPushButton>

#include "kgamewin.h"
namespace Ksirk
{
  class KGameWindow;
namespace GameLogic
{
  class ONU;
  class Country;
}
   /**
   * The KRightDialog class is the widget displayed on the right of
   * the application
   */
   class KRightDialog: public QWidget
   {
      Q_OBJECT
      
      public:
      /**
      * Creates the group box which will contains the needed widgets
      */
      KRightDialog(QDockWidget * parent,GameLogic::ONU * world,KGameWindow* m_game);
      
      /**
      * Destroy the widget
      */
      virtual ~KRightDialog();

      /**
      * Display the country information after a right click mouse
      * @param contryPoint which is the point clicked
      */
      void displayCountryDetails(const QPointF& countryPoint);

      /**
      * Display the fight details
      */
      void displayFightDetails(GameLogic::Country * attaker, GameLogic::Country * defender,int nb_A, int nb_D);

      /**
       * Display information about the recycling processus
       * @param player which we want to see information
       */
      void displayRecycleDetails(GameLogic::Player* player, int nbAvailArmies);

      /**
       * Update information about the recycling processus
       * @param country country which gives his information
       * @param recyclePhase shows if we are in recycle phase or not
       */
      void updateRecycleDetails(GameLogic::Country* country, bool recyclePhase, int nbAvailArmies);

      /**
      * Display the dice
      */
      void displayFightResult(int A1, int A2, int A3, int D1, int D2,int nbA,int nbD, bool win);

      void initListLabel(int i);

      void removeListLabel(); 
     
      void clearLabel();

      void clearLayout() ;

      inline void close() {m_parentWidget->hide();};

      inline void open() {m_parentWidget->show();};

      inline bool isOpen() {return m_parentWidget->isVisible();};

      private:
 
      QGridLayout * mainLayout;
      
      QLabel * loadingLabel;
      
      QLabel *infoProcess;
      
      QDockWidget * m_parentWidget;

      Ksirk::GameLogic::ONU * world;
    
      QList<QLabel*> * rightContents;

      QLabel * flag1;

      QLabel * flag2;

      QWidget * bas; 

      QWidget * haut;

      QWidget * milieu;

      QWidget * milieu2;

      // Widgets and buttons used in displayRecycleDetails
      QWidget * btRecycleWidget;

      QWidget * btValidWidget;

      // images for buttons
      QPixmap soldat;
      QPixmap stopAttackAuto;
      QPixmap recycleContinue;
      QPixmap recycleDone;
      QPixmap recycleNextPlayer;

      KGameWindow * game;

      QPushButton* buttonStopAttack;

      private slots:
      
      void slotStopAttackAuto();

   };
   
}

#endif // KRIGHTDIALOG_H
