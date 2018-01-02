/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef KSIRK_NEWPLAYERDATA_H
#define KSIRK_NEWPLAYERDATA_H

#include <QString>

namespace Ksirk {

class NewPlayerData
{
public:
  NewPlayerData(const QString& name, const QString& nation, const QString& password, bool computer,
                bool network);
  virtual ~NewPlayerData() {}
  
  inline const QString& name() const {return m_name;}
  inline void setName(const QString& name) {m_name = name;}
  
  inline const QString& nation() const {return m_nation;}
  inline void setNation(const QString& nation) {m_nation = nation;}
  
  inline const QString& password() const {return m_password;}
  inline void setPassword(const QString& password) {m_password = password;}
  
  inline bool computer() const {return m_computer;}
  inline void setComputer(bool computer) {m_computer = computer;}
  
  inline bool network() const {return m_network;}
  inline void setNetwork(bool network) {m_network = network;}
  
private:
  QString m_name;
  QString m_nation;
  QString m_password;
  bool m_computer;
  bool m_network;
};

}

#endif // KSIRK_NEWPLAYERDATA_H
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
