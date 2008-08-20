/***************************************************************************
                          skinSpritesData.h  -  description
                             -------------------
    begin                : 2005
    copyright            : (C) 2005-2007 by Gael de Chalendar (aka kleag)
    email                : kleag@free.fr
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

#ifndef KSIRKSKINEDITORSKINSPRITESDATA_H
#define KSIRKSKINEDITORSKINSPRITESDATA_H

#include <QPoint>
#include <QString>
#include <QStringList>
#include <QMap>

namespace KsirkSkinEditor
{
/**
 * This class holds named values related to the current skin. It is a singleton
 * to be easily accessible from any game object.
 * @author Gael de Chalendar (aka Kleag)
 */
class SkinSpritesData
{
public:

  /**
   * Initializes the sprites data by clearing its internal storage. Should be 
   * used each time a new skin is loaded
   */
  void init();
  
  /**
   * return the sole instance of this singleton class as const 
   */
  static const SkinSpritesData& single();

  /**
   * return the sole instance of this singleton class as changeable. Used only
   * at the time of skin loading for initialization purpose
   */
  static SkinSpritesData& changeable();

  /**
   * Gets the skin name
   */
  const QString& skin() const;
  
  /**
   * Sets the skin name
   */
  void skin(const QString& newSkin);
  

  /**
   * Gets the integer data named @ref name
   * @param name the name of the integer data to retrieve
   * @return the value of the integer data whose name is given
   */
  int intData(const QString& name) const;

  /**
   * Gets the string data named @ref name
   * @param name the name of the string data to retrieve
   * @return the value of the string data whose name is given
   */
  const QString& strData(const QString& name) const;
      
  /**
   * Sets the string data named @ref name with the value @ref data
   * @param name the name of the string data to initialize
   * @param data the value of the string data to initialize
   */
  void strData(const QString& name, const QString& data);

  /**
   * Sets the integer data named @ref name with the value @ref data
   * @param name the name of the integer data to initialize
   * @param data the value of the integer data to initialize
   */
  void intData(const QString& name, int data);
        
private:
  SkinSpritesData();
    
  SkinSpritesData(const SkinSpritesData& /*ga*/) {};

  virtual ~SkinSpritesData();

  static SkinSpritesData* m_singleton ;
  
  QString m_skin;
  
  QMap<QString, int> m_intDatas;
  QMap<QString, QString> m_strDatas;
};

}

#endif 
