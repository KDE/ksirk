/***************************************************************************
                          onu.cpp  -  description
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001-2006 by Gael de Chalendar (aka Kleag)
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
#define KDE_NO_COMPAT
#include <qfile.h>
#include <qdom.h>
#include <qpainter.h>
#include <QPixmap>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "onu.h"
#include "Sprites/skinSpritesData.h"
#include "goal.h"
#include "gameautomaton.h"

namespace Ksirk
{

namespace GameLogic
{

ONU::ONU(const QString& configFileName):
  m_configFileName(configFileName),
  countries(),
  nationalities(),
  m_continents(),
  m_zoom(1.0),
  m_renderer(0)
{
  kDebug() << "ONU constructor: " << m_configFileName << endl;
  m_font.family = "URW Chancery L";
  m_font.size = 13;
  m_font.weight = QFont::Bold;
  m_font.italic = true;
  m_font.foregroundColor = "black";
  m_font.backgroundColor = "white";
  
  
  Sprites::SkinSpritesData::changeable().init();
  unsigned int nationalityId = 0;
  unsigned int continentId = 0;
  // read the XML file and create DOM tree
  QFile configFile( configFileName );
  if ( !configFile.open( QIODevice::ReadOnly ) )
  {
      KMessageBox::error( 0,
                          i18n( "Cannot open file %1",configFileName ),
                          i18n( "Critical Error" ) );
      exit(2);
  }
  QDomDocument domTree;
  if ( !domTree.setContent( &configFile ) )
  {
      KMessageBox::error( 0,
                          i18n( "Parsing error for file %1",configFileName ),
                          i18n( "Critical Error" ) );
      configFile.close();
      exit(2);
  }
  configFile.close();

  // get the header information from the DOM
  QDomElement root = domTree.documentElement();

  // read the attributes values of the root element
  kDebug() << "ONU XML format version: " << root.attribute("format-version") << endl;
  QString formatVersion = root.attribute("format-version");
  if (formatVersion != ONU_FILE_FORMAT_VERSION)
  {
    KMessageBox::error(0,
            i18n("Error - Invalid skin definition file format. Expected %1 and got %2",QString(ONU_FILE_FORMAT_VERSION),formatVersion),
            i18n("Fatal Error"));
    exit(1);
  }

  m_name = root.attribute("name");
  m_skin = root.attribute("skinpath");
  kDebug() << "skin snapshot file: " << KGlobal::dirs()-> findResource("appdata", m_skin + "/Images/snapshot.jpg") << endl;
  m_snapshot = QPixmap(KGlobal::dirs()-> findResource("appdata", m_skin + "/Images/snapshot.jpg"));
  if (m_snapshot.isNull())
  {
    kError() << "Was not able to load the snapshot image: " 
      << KGlobal::dirs()-> findResource("appdata", m_skin + "/Images/snapshot.jpg") 
      << endl;
  }
  m_width  = root.attribute("width").toUInt();
  m_height  = root.attribute("height").toUInt();
  countries.resize(root.attribute("nb-countries").toUInt());
  nationalities.resize(root.attribute("nb-nationalities").toUInt());
  m_continents.resize(root.attribute("nb-continents").toUInt());
//    root.attribute("map");
  QString mapString = root.attribute("map");
  kDebug() << "Map path: " << mapString << endl;
  m_mapFileName = KGlobal::dirs()-> findResource("appdata", m_skin + '/' + mapString);
  kDebug() << "Map file name: " << m_mapFileName << endl;
  if (m_mapFileName.isEmpty())
  {
      KMessageBox::error(0, 
                         i18n("Map image filename not found\nProgram cannot continue"),
                         i18n("Error !"));
      exit(2);
  }
  m_map = QPixmap(m_mapFileName);

  QString mapMaskFileName = KGlobal::dirs()-> findResource("appdata", m_skin + '/' + root.attribute("map-mask"));
  kDebug() << "Map mask file name: " << mapMaskFileName << endl;
  if (mapMaskFileName.isNull())
  {
      KMessageBox::error(0, 
                         i18n("Map mask image not found\nProgram cannot continue"),
                         i18n("Error !"));
      exit(2);
  }
  kDebug() << "Loading map mask file: " << mapMaskFileName << endl;
  countriesMask = QImage(mapMaskFileName);
//   if (countriesMask.isNull())
//   {
//       KMessageBox::error(KApplication::kApplication()->mainWidget(),
//                          i18n("Cannot load the map mask image\nProgram cannot continue"),
//                          i18n("Error !"));
//       exit(2);
//   }

  QDomNode node = root.firstChild();
  while ( !node.isNull() )
  {
    if (node.isElement() && node.nodeName() == "skinSpritesData" )
    {
      QDomNode itemNode = node.firstChild();
      while ( !itemNode.isNull() )
      {
      
        if (itemNode.isElement() && itemNode.nodeName() == "intItem" )
        {
          QDomElement itemEl = itemNode.toElement();
          QString key = itemEl.attribute("key");
          int val = itemEl.attribute("value").toInt();
          Sprites::SkinSpritesData::changeable().intData(key, val);  
          kDebug() << "Loaded int item " << key << " = " << val << endl;
        }
        else if (itemNode.isElement() && itemNode.nodeName() == "strItem" )
        {
          QDomElement itemEl = itemNode.toElement();
          QString key(itemEl.attribute("key"));
          Sprites::SkinSpritesData::changeable().strData(key, itemEl.attribute("value"));  
          kDebug() << "Loaded str item " << key << " = " << itemEl.attribute("value") << endl;
        }
        itemNode = itemNode.nextSibling();
      }
    }
    else if ( node.isElement() && node.nodeName() == "desc" )
    {
        QDomElement descNode = node.toElement();
        m_description = descNode.text();
    }
    else if (node.isElement() && node.nodeName() == "font" )
    {
      kDebug() << "reading font data..." << endl;
      QDomNode itemNode = node.firstChild();
      while ( !itemNode.isNull() )
      {
      
        if (itemNode.isElement() && itemNode.nodeName() == "family" )
        {
          m_font.family = itemNode.toElement().text();
          kDebug() << "  got family: " << m_font.family << endl;
        }
        else if (itemNode.isElement() && itemNode.nodeName() == "size" )
        {
          QString fs = itemNode.toElement().text();
          QTextStream is(&fs);
          is >> m_font.size;
          kDebug() << "  got size: " << m_font.size << endl;
        }
        else if (itemNode.isElement() && itemNode.nodeName() == "weight" )
        {
          QString w = itemNode.toElement().text();
          if (w == "normal")
          {
            m_font.weight = QFont::Normal;
          }
          else if (w == "light")
          {
            m_font.weight = QFont::Light;
          }
          else if (w == "demibold")
          {
            m_font.weight = QFont::DemiBold;
          }
          else if (w == "bold")
          {
            m_font.weight = QFont::Bold;
          }
          else if (w == "black")
          {
            m_font.weight = QFont::Black;
          }
          kDebug() << "  got weight: " << m_font.weight << endl;
        }
        else if (itemNode.isElement() && itemNode.nodeName() == "italic" )
        {
          QString ital = itemNode.toElement().text();
          if (ital == "true")
          {
              m_font.italic = true;
          }
          else if (ital == "false")
          {
              m_font.italic = false;
          }
          kDebug() << "  got italic: " << m_font.italic << endl;
        }
        else if (itemNode.isElement() && itemNode.nodeName() == "foreground-color" )
        {
          m_font.foregroundColor = itemNode.toElement().text();
          kDebug() << "  got foreground color: " << m_font.foregroundColor << endl;
        }
        else if (itemNode.isElement() && itemNode.nodeName() == "background-color" )
        {
          m_font.backgroundColor = itemNode.toElement().text();
          kDebug() << "  got background color: " << m_font.backgroundColor << endl;
        }
        itemNode = itemNode.nextSibling();
      }
    }
    else  if ( node.isElement() && node.nodeName() == "country" )
    {
        QDomElement countryNode = node.toElement();
        unsigned int id = countryNode.attribute("id").toUInt();
        QString name = countryNode.attribute("name");
        QDomNode countryChild = countryNode.firstChild();
        QPoint centralPoint;
        QPoint flagPoint;
        QPoint cannonPoint;
        QPoint cavalryPoint;
        QPoint infantryPoint;
        while ( !countryChild.isNull() )
        {
            QDomElement countryChildEl = countryChild.toElement();
            unsigned int x = countryChildEl.attribute("x").toUInt();
            unsigned int y = countryChildEl.attribute("y").toUInt();
//                kDebug() << "Got attributes for " << countryChild.nodeName() << " x and y: " << countryChildEl.attribute("x") << " " << countryChildEl.attribute("y") << endl;
            if ( countryChild.isElement() && countryChild.nodeName() == "central-point" )
                centralPoint = QPoint(x, y);
            else if ( countryChild.isElement() && countryChild.nodeName() == "flag-point" )
                flagPoint = QPoint(x, y);
            else if ( countryChild.isElement() && countryChild.nodeName() == "cannons-point" )
                cannonPoint = QPoint(x, y);
            else if ( countryChild.isElement() && countryChild.nodeName() == "cavalry-point" )
                cavalryPoint = QPoint(x, y);
            else if ( countryChild.isElement() && countryChild.nodeName() == "infantry-point" )
                infantryPoint = QPoint(x, y);
            countryChild = countryChild.nextSibling();
        }
        kDebug() << "Creating country " << name << endl;
//            kDebug() << "\tflag point: " << flagPoint.x() << " " << flagPoint.y() << endl;
        countries[id] = new Country(name, centralPoint, flagPoint,
            cannonPoint, cavalryPoint, infantryPoint, id);
    }
    else if ( node.isElement() && node.nodeName() == "nationality" )
    {
        QDomElement nationalityNode = node.toElement();
        QString name = nationalityNode.attribute("name");
        QString leader = nationalityNode.attribute("leader");
        QString flag = nationalityNode.attribute("flag");
        kDebug() << "Creating nationality " << name << " ; flag: " << flag << endl;
        nationalities[nationalityId] = new Nationality(name, flag, leader);
        nationalityId++;
    }
    else if ( node.isElement() && node.nodeName() == "continent" )
    {
      QDomElement continentNode = node.toElement();
      unsigned int id = continentNode.attribute("id").toUInt();
      QString name = continentNode.attribute("name");
      unsigned int bonus = continentNode.attribute("bonus").toUInt();
      std::vector< Country* > continentList;
      QDomNode continentChild = continentNode.firstChild();
      while ( !continentChild.isNull() )
      {
        if ( continentChild.isElement() && continentChild.nodeName() == "continent-country" )
        {
          QDomElement continentChildEl = continentChild.toElement();
          unsigned int id = continentChildEl.attribute("id").toUInt();
          continentList.push_back(countries[id]);
        }
        continentChild = continentChild.nextSibling();
      }
      kDebug() << "Creating continent " << name << endl;
      m_continents[continentId++] = new Continent(name, continentList, bonus,id);
    }
    else if ( node.isElement() && node.nodeName() == "goals" )
    {
      QDomNode goalNode = node.firstChild();
      while ( !goalNode.isNull() )
      {
        if (goalNode.isElement() && goalNode.nodeName() == "goal" )
        {
          kDebug() << "Goal: creating goal" << endl;
          Goal* goal = new Goal();
          QDomNode subGoalNode = goalNode.firstChild();
          while ( !subGoalNode.isNull() )
          {
            if (subGoalNode.isElement() && subGoalNode.nodeName() == "countries" )
            {
              goal->type(Goal::Countries);
              QDomElement subGoalNodeEl = subGoalNode.toElement();
              unsigned int nb = subGoalNodeEl.attribute("nb").toUInt();
              goal->nbCountries(nb);
              kDebug() << "  nb countries: " << nb << endl;
            }
            else if (subGoalNode.isElement() && subGoalNode.nodeName() == "armiesByCountry" )
            {
              QDomElement subGoalNodeEl = subGoalNode.toElement();
              unsigned int nb = subGoalNodeEl.attribute("nb").toUInt();
              goal->nbArmiesByCountry(nb);
              kDebug() << "  nb armies by country: " << nb << endl;
            }
            else if (subGoalNode.isElement() && subGoalNode.nodeName() == "continent" )
            {
              goal->type(Goal::Continents);
              QDomElement subGoalNodeEl = subGoalNode.toElement();
              unsigned int id = subGoalNodeEl.attribute("id").toUInt();
              goal->continents().insert(id);
              kDebug() << "  continent: " << id << endl;
            }
            else if (subGoalNode.isElement() && subGoalNode.nodeName() == "player" )
            {
              goal->type(Goal::GoalPlayer);
              QDomElement subGoalNodeEl = subGoalNode.toElement();
              unsigned int nb = subGoalNodeEl.attribute("nbCountriesFallback").toUInt();
              goal->nbCountries(nb);
            kDebug() << "  player with fallback: " << nb << endl;
            }
            else if (subGoalNode.isElement() && subGoalNode.nodeName() == "desc" )
            {
              QDomElement subGoalNodeEl = subGoalNode.toElement();
              QString text = subGoalNodeEl.text();
              goal->description(text);
              kDebug() << "  description: '" << text << "'" << endl;
            }
            subGoalNode = subGoalNode.nextSibling();
          }
          kDebug() << "Inserting goal with type " << goal->type() << endl;
          GameLogic::GameAutomaton::changeable().goals().insert(goal);
          subGoalNode = subGoalNode.nextSibling();
        }
        goalNode = goalNode.nextSibling();
      }
    }
    node = node.nextSibling();
  }
  // create the neighbours lists
  node = root.firstChild();
  while ( !node.isNull() )
  {
      if ( node.isElement() && node.nodeName() == "country" )
      {
          std::vector< Country* > theNeighbours;
          QDomElement countryNode = node.toElement();
          unsigned int id = countryNode.attribute("id").toUInt();
          QDomNode countryChild = countryNode.firstChild();
          while ( !countryChild.isNull() )
          {
              QDomElement countryChildEl = countryNode.toElement();
              if ( countryChild.isElement() && countryChild.nodeName() == "neighbours" )
              {
                  QDomElement neighboursNode = countryChild.toElement();
                  QDomNode neighbour = neighboursNode.firstChild();
                  while ( !neighbour.isNull() )
                  {
                      QDomElement neighbourEl = neighbour.toElement();
                      unsigned int neighbourId = neighbourEl.attribute("id").toUInt();
//                        kDebug() << "Got attribute for " << neighbour.nodeName() << ": " << neighbourId << endl;
                      theNeighbours.resize(theNeighbours.size() + 1);
                      theNeighbours[theNeighbours.size() - 1] =  countries[neighbourId];
                      neighbour = neighbour.nextSibling();
                  }
              }
              countryChild = countryChild.nextSibling();
          }
          countries.at(id)-> neighbours(theNeighbours);
      }
      node = node.nextSibling();
  }
  buildMap();
//    kDebug() << "OUT ONU::ONU" << endl;
}

/** This method returns a pointer to the country that contains the point (x,y).
If there is no country at (x,y), the functions returns 0. */
Country* ONU::countryAt(QPointF point)
{
//    kDebug() << "ONU::countryAt x y " << x << " " << y << endl;
    if ( point.x() < 0 || point.x() >= countriesMask.width() 
        || point.y() < 0 || point.y() >= countriesMask.height() )
      return 0;

    unsigned int index = qBlue(countriesMask.pixel(point.toPoint()));
//    kDebug() << "OUT ONU::countryAt: " << index << endl;
    if (index >= countries.size()) return 0;
    return countries.at(index);
}

void ONU::reset()
{
  kDebug() << "ONU::reset" << endl;
  for (unsigned int i = 0; i < countries.size(); i++) 
  {
    countries.at(i)-> reset();
  }
}


std::vector<Country*>& ONU::getCountries()
{
  return countries;
}

std::vector<Nationality*>& ONU::getNationalities()
{
  return nationalities;
}

/** Read property of std::vector<Continent*> continents. */
const std::vector<Continent*>& ONU::getContinents() const
{
  return m_continents;
}

std::vector<Continent*>& ONU::getContinents()
{
  return m_continents;
}

/**
  * Returns the list of countries neighbours of this country that does not
  * belongs to the argument player.
  */
std::vector<Country*> ONU::neighboursBelongingTo(const Country& country, const Player* player)
{
    std::vector<Country*> list;
    for (unsigned int i = 0 ; i < country.neighbours().size(); i++)
    {
        Country *c = country.neighbours().at(i);
        if ((country.communicateWith(c)) && (c-> owner() == player))
            {list.push_back(c);}
    }
    return list;
}

/**
  * Returns the list of countries neighbours of this country that does not
  * belongs to the argument player.
  */
std::vector<Country*> ONU::neighboursNotBelongingTo(const Country& country, const Player* player)
{
    std::vector<Country*> list;
    for (unsigned int i = 0 ; i < country.neighbours().size(); i++)
    {
        Country *c = country.neighbours().at(i); 
        if ((country.communicateWith(c)) && (c-> owner() != player))
            {list.push_back(c);}
    }
    return list;
}

/**
  * Returns the country named "name" ; 0 in case there is no such country
  */
Country* ONU::countryNamed(const QString& name)
{
  for (unsigned int i = 0 ; i < countries.size(); i++)
  {
    Country *c = countries.at(i);
    if (c-> name() == name)
      return c;
  }
  return 0;
}

/** @return the number of countries in the world */
unsigned int ONU::getNbCountries() const
{
    return(countries.size());
}

void ONU::saveXml(std::ostream& xmlStream)
{
  xmlStream << "<ONU file=\"" << m_configFileName.toUtf8().data() << "\" >" << std::endl;

  xmlStream << "<countries>" << std::endl;
  for (unsigned int i = 0 ; i < countries.size(); i++)
  {
      Country *c = countries.at(i);
      c->saveXml(xmlStream);
  }
  xmlStream << "</countries>" << std::endl;
/*
  xmlStream << "<nationalities>" << std::endl;
  for (unsigned int i = 0 ; i < nationalities.size(); i++)
  {
      Nationality *n = nationalities.at(i);
      n->saveXml(xmlStream);
  }
  xmlStream << "</nationalities>" << std::endl;

  xmlStream << "<continents>" << std::endl;
  for (unsigned int i = 0 ; i < continents.size(); i++)
  {
      Continent *c = continents.at(i);
      c->saveXml(xmlStream);
  }
  xmlStream << "</continents>" << std::endl;
*/
  xmlStream << "</ONU>" << std::endl;  
}

const QString& ONU::mapFileName() const
{
  kDebug() << "ONU::mapFileName() " << m_mapFileName << endl;
  return m_mapFileName;
}

unsigned int ONU::width() const
{
  return m_width;
}

unsigned int ONU::height() const
{
  return m_height;
}

/** Returns the nation named "name" ; 0 in case there is no such nation */
Nationality* ONU::nationNamed(const QString& name)
{
  for (unsigned int i = 0 ; i < nationalities.size(); i++)
  {
    Nationality *n = nationalities.at(i);
    if (n->name() == name)
    {
      return n;
    }
  }
  return 0;
}

void ONU::sendCountries(QDataStream& stream)
{
  stream << quint32(countries.size());
  for (unsigned int i = 0; i < countries.size(); i++)
  {
    kDebug() << "Sending country n° " << i+1 << " on " << countries.size() << endl;
    countries[i]->send(stream);
  }
}

const Continent* ONU::continentWithId(const unsigned int id) const
{
  for (unsigned int i = 0; i < m_continents.size(); i++)
  {
    if ( m_continents.at(i)->id() == id)
    {
      return m_continents.at(i);
    }
  }
  return 0;
}

Continent* ONU::continentNamed(const QString& name)
{
  for (unsigned int i = 0 ; i < m_continents.size(); i++)
  {
    Continent *c = m_continents.at(i);
    if (c-> name() == name)
      return c;
  }
  return 0;
}

void ONU::buildMap()
{
  kDebug() << "ONU::buildMap" << endl;
  QSize size(m_renderer.defaultSize().width()*m_zoom,m_renderer.defaultSize().height()*m_zoom);
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  image.fill(0);
  QPainter p(&image);
  m_renderer.render(&p/*, svgid*/);
  QPixmap mapPixmap = QPixmap::fromImage(image);

  m_map.scaled(mapPixmap.size());
  QPainter painter(&m_map);
  QFont foregroundFont(m_font.family, m_font.size, m_font.weight, m_font.italic);
  QFont backgroundFont(m_font.family, m_font.size, QFont::Normal, m_font.italic);
  painter.drawPixmap(0,0,mapPixmap);
  
  for (uint i = 0; i < countries.size(); i++)
  {
    Country* country = countries[i];
    const QString& countryName = i18n(country->name().toUtf8().data());
    QRect countryNameRect = painter.fontMetrics().boundingRect(countryName);
    if (m_font.backgroundColor != "none")
    {
      painter.setPen(m_font.backgroundColor);
      painter.setFont(backgroundFont);
      painter.drawText(int(country->centralPoint().x()-countryNameRect.width()/2+1),
        int(country->centralPoint().y()+countryNameRect.height()/2 + 1),
        countryName);
    }
    painter.setPen(m_font.foregroundColor);
    painter.setFont(foregroundFont);
    painter.drawText(int(country->centralPoint().x()-countryNameRect.width()/2),
        int(country->centralPoint().y()+countryNameRect.height()/2),
        countryName);
  }
}

} // closing namespace GameLogic
} // closing namespace Ksirk
