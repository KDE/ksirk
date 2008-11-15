/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 or later as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef DISTRIBUTIONDATA_H
#define DISTRIBUTIONDATA_H

#include <QList>

class DistributionData : public QList<int>
{
public:
  DistributionData() {}

  void init(int nb, int nbCountries);

  inline int nbToPlace() {return m_nbToPlace;}
  inline void setNbToPlace(int nb) {m_nbToPlace = nb;}

private:
  int m_nbToPlace;
};

#endif // DISTRIBUTIONDATA_H
