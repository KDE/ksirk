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
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/
#include "onu.h"
#include "Sprites/skinSpritesData.h"
#include "goal.h"

#include <QFile>
#include <qdom.h>
#include <QPainter>
#include <QPixmapCache>
#include <QMenuBar>
#include <KLocalizedString>
#include "ksirk_debug.h"
#include <KMessageBox>
#include <KConfig>
#include <KConfigGroup>

namespace Ksirk
{

namespace GameLogic
{

ONU::ONU(GameAutomaton* automaton,
  const QString& configFileName):
  QObject(automaton),
  m_automaton(automaton),
  m_configFileName(configFileName),
  countries(),
  nationalities(),
  m_continents(),
  m_skin(),
  m_zoom(1.0),
  m_zoomArena(1.0),
  m_nbZooms(0),
  m_zoomFactorFinal(1)
{
  qCDebug(KSIRK_LOG) << "ONU constructor: " << m_configFileName;
  m_font.family = "URW Chancery L";
  m_font.size = (int)(13*m_zoom);
  m_font.weight = QFont::Bold;
  m_font.italic = true;
  m_font.foregroundColor = "black";
  m_font.backgroundColor = "white";
  
  m_timerFast=new QTimer(this);		//instanciation of the timer
  QObject::connect(m_timerFast,&QTimer::timeout, this, &ONU::changingZoom);	//connect the timer to the good slot
  
  Sprites::SkinSpritesData::changeable().init();
//   unsigned int nationalityId = 0;
//   unsigned int continentId = 0;
  KConfig config(configFileName);

  KConfigGroup onugroup = config.group("onu");

  qCDebug(KSIRK_LOG) << "ONU XML format version: " << onugroup.readEntry("format-version");
  QString formatVersion = onugroup.readEntry("format-version");

  if (formatVersion != ONU_FILE_FORMAT_VERSION)
  {
    qCCritical(KSIRK_LOG) << "Error - Invalid skin definition file format. Expected "<<QString(ONU_FILE_FORMAT_VERSION)<<" and got " << formatVersion << "in" << m_configFileName << ". You should remove this skin folder";
//     KMessageBox::error(0,
//                         i18n("Error - Invalid skin definition file format. Expected %1 and got %2",QString(ONU_FILE_FORMAT_VERSION),formatVersion) + "<br>" + m_configFileName,
//                         i18n("Fatal Error"));
    return;
  }

  m_name = onugroup.readEntry("name");
  m_skin = onugroup.readEntry("skinpath");

  qCDebug(KSIRK_LOG) << "skin snapshot file: " << QStandardPaths::locate(QStandardPaths::AppDataLocation, m_skin + "/Images/snapshot.jpg");
  if (!QPixmapCache::find(m_skin+"snapshot", &m_snapshot))
  {
    // Pixmap isn't in the cache, create it and insert to cache
    m_snapshot = QPixmap(QStandardPaths::locate(QStandardPaths::AppDataLocation, m_skin + "/Images/snapshot.jpg"));
    if (m_snapshot.isNull())
    {
      qCCritical(KSIRK_LOG) << "Was not able to load the snapshot image: " << QStandardPaths::locate(QStandardPaths::AppDataLocation, m_skin + "/Images/snapshot.jpg");
    }
    QPixmapCache::insert(m_skin+"snapshot", m_snapshot);
  }
  m_width  = onugroup.readEntry("width",0);
  m_height  = onugroup.readEntry("height",0);
  m_description = onugroup.readEntry("desc");
//   countries.resize(onugroup.readEntry("nb-countries",0));
//   nationalities.resize(onugroup.readEntry("nb-nationalities",0));
//   m_continents.resize(onugroup.readEntry("nb-continents",0));
//    root.attribute("map");
  QString poolString = onugroup.readEntry("pool");
  qCDebug(KSIRK_LOG) << "Pool path: " << poolString;
  qCDebug(KSIRK_LOG) << "Searching resource: " << (m_skin + '/' + poolString);

  QString poolFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, m_skin + '/' + poolString);
  qCDebug(KSIRK_LOG) << "Pool file name: " << poolFileName;
  if (poolFileName.isEmpty())
  {
      KMessageBox::error(nullptr, 
                         i18n("Pool filename not found\nProgram cannot continue"),
                         i18n("Error!"));
      exit(2);
  }
  m_map = QPixmap();
  qCDebug(KSIRK_LOG) << m_skin << "before pool loading";
  if (!m_automaton->rendererFor(m_skin).isValid())
    m_automaton->rendererFor(m_skin).load(poolFileName);
  if (m_automaton->svgDomFor(m_skin).svgFilename().isNull())
    m_automaton->svgDomFor(m_skin).load(poolFileName);
  qCDebug(KSIRK_LOG) << m_skin << "after pool loading";
  
  QString mapMaskFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, m_skin + '/' + onugroup.readEntry("map-mask"));
  qCDebug(KSIRK_LOG) << "Map mask file name: " << mapMaskFileName;
  if (mapMaskFileName.isNull())
  {
      KMessageBox::error(nullptr, 
                         i18n("Map mask image not found\nProgram cannot continue"),
                         i18n("Error!"));
      exit(2);
  }
  qCDebug(KSIRK_LOG) << "Loading map mask file: " << mapMaskFileName;
  QPixmap countriesMaskPix;
  if (!QPixmapCache::find(m_skin+"mapmask", &countriesMaskPix))
  {
    // Pixmap isn't in the cache, create it and insert to cache
    countriesMaskPix = QPixmap(mapMaskFileName);
    if (countriesMaskPix.isNull())
    {
      qCCritical(KSIRK_LOG) << "Was not able to load the map mask image: " << mapMaskFileName ;
    }
    QPixmapCache::insert(m_skin+"mapmask", countriesMaskPix);
  }
//   countriesMask = QImage(mapMaskFileName);
  countriesMask = countriesMaskPix.toImage();
  
  Sprites::SkinSpritesData::changeable().intData("width-between-flag-and-fighter", onugroup.readEntry("width-between-flag-and-fighter",0));

  Sprites::SkinSpritesData::changeable().intData("flag-width", config.group("flag").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("flag-height", config.group("flag").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("flag-frames", config.group("flag").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("flag-versions", config.group("flag").readEntry("versions",0));

//   Sprites::SkinSpritesData::changeable().strData("infantry-id", config.group("infantry").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("infantry-width", config.group("infantry").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("infantry-height", config.group("infantry").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("infantry-frames", config.group("infantry").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("infantry-versions", config.group("infantry").readEntry("versions",0));

//   Sprites::SkinSpritesData::changeable().strData("infantry1-id", config.group("infantry1").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("infantry1-width", config.group("infantry1").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("infantry1-height", config.group("infantry1").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("infantry1-frames", config.group("infantry1").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("infantry1-versions", config.group("infantry1").readEntry("versions",0));

//   Sprites::SkinSpritesData::changeable().strData("infantry2-id", config.group("infantry2").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("infantry2-width", config.group("infantry2").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("infantry2-height", config.group("infantry2").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("infantry2-frames", config.group("infantry2").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("infantry2-versions", config.group("infantry2").readEntry("versions",0));

//   Sprites::SkinSpritesData::changeable().strData("infantry3-id", config.group("infantry3").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("infantry3-width", config.group("infantry3").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("infantry3-height", config.group("infantry3").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("infantry3-frames", config.group("infantry3").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("infantry3-versions", config.group("infantry3").readEntry("versions",0));

//   Sprites::SkinSpritesData::changeable().strData("cavalry-id", config.group("cavalry").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("cavalry-width", config.group("cavalry").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("cavalry-height", config.group("cavalry").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("cavalry-frames", config.group("cavalry").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("cavalry-versions", config.group("cavalry").readEntry("versions",0));

//   Sprites::SkinSpritesData::changeable().strData("cannon-id", config.group("cannon").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("cannon-width", config.group("cannon").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("cannon-height", config.group("cannon").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("cannon-frames", config.group("cannon").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("cannon-versions", config.group("cannon").readEntry("versions",0));

//   Sprites::SkinSpritesData::changeable().strData("firing-id", config.group("firing").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("firing-width", config.group("firing").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("firing-height", config.group("firing").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("firing-frames", config.group("firing").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("firing-versions", config.group("firing").readEntry("versions",0));

//   Sprites::SkinSpritesData::changeable().strData("exploding-id", config.group("exploding").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("exploding-width", config.group("exploding").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("exploding-height", config.group("cannon").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("exploding-frames", config.group("exploding").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("exploding-versions", config.group("exploding").readEntry("versions",0));

  KConfigGroup fontgroup = config.group("font");
  m_font.family = fontgroup.readEntry("family","URW Chancery L");
  m_font.size = fontgroup.readEntry("size",(int)(13*m_zoom));
  QString w = fontgroup.readEntry("weight", "bold");;
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
  m_font.italic = fontgroup.readEntry("italic", true);
  m_font.foregroundColor = fontgroup.readEntry("foreground-color", "black");
  m_font.backgroundColor = fontgroup.readEntry("background-color", "white");

  QStringList countriesList = onugroup.readEntry("countries", QStringList());
  foreach (const QString &country, countriesList)
  { 
    KConfigGroup countryGroup = config.group(country);
//     unsigned int id = countryGroup.readEntry("id",0);
    QString name = country;
    QPointF anchorPoint = countryGroup.readEntry("anchor-point",QPointF())*m_zoom;
    QPointF centralPoint = countryGroup.readEntry("central-point",QPointF())*m_zoom;
    QPointF flagPoint = countryGroup.readEntry("flag-point",QPointF())*m_zoom;
    QPointF cannonPoint = countryGroup.readEntry("cannon-point",QPointF())*m_zoom;
    QPointF cavalryPoint = countryGroup.readEntry("cavalry-point",QPointF())*m_zoom;
    QPointF infantryPoint = countryGroup.readEntry("infantry-point",QPointF())*m_zoom;

//     qCDebug(KSIRK_LOG) << "Creating country " << name;
//     qCDebug(KSIRK_LOG) << "\tflag point: " << flagPoint;
//     qCDebug(KSIRK_LOG) << "\tcentral point: " << centralPoint;
//     qCDebug(KSIRK_LOG) << "\tcannon point: " << cannonPoint;
//     qCDebug(KSIRK_LOG) << "\tcavalry point: " << cavalryPoint;
//     qCDebug(KSIRK_LOG) << "\tinfantry point: " << infantryPoint;
    countries.push_back(new Country(automaton, name, anchorPoint, centralPoint,
        flagPoint, cannonPoint, cavalryPoint, infantryPoint));
  }
  QStringList nationalitiesList = onugroup.readEntry("nationalities", QStringList());
  foreach (const QString &nationality, nationalitiesList)
  {
//     qCDebug(KSIRK_LOG) << "Creating nationality " << nationality;
    KConfigGroup nationalityGroup = config.group(nationality);
    QString leader = nationalityGroup.readEntry("leader","");
    QString flag = nationalityGroup.readEntry("flag","");
//         qCDebug(KSIRK_LOG) << "Creating nationality " << name << " ; flag: " << flag;
    nationalities.push_back(new Nationality(nationality, flag, leader));
//     nationalityId++;
  }


  QStringList continentsList = onugroup.readEntry("continents", QStringList());
  foreach (const QString &continent, continentsList)
  {
    KConfigGroup continentGroup = config.group(continent);

//     unsigned int id = continentGroup.readEntry("id",0);
    unsigned int bonus = continentGroup.readEntry("bonus",0);
    QList<QString> countryIdList = continentGroup.readEntry("continent-countries",QList<QString>());
//     int countryId;
    QList<Country*> continentList;
    foreach(const QString& countryId, countryIdList)
    {
//       qCDebug(KSIRK_LOG) << "Adding" << countryId << "to" << continent << "list";
      Country *c = countryNamed(countryId);
      if (c)
      {
        continentList.push_back(c);
      }
    }
//       qCDebug(KSIRK_LOG) << "Creating continent " << name;
    m_continents.push_back(new Continent(continent, continentList, bonus));
  }

  QStringList goalsList = onugroup.readEntry("goals", QStringList());
  foreach (const QString &_goal, goalsList)
  {
//     qCDebug(KSIRK_LOG) << "init goal " << _goal;
    KConfigGroup goalGroup = config.group(_goal);

    Goal* goal = new Goal(automaton);
    goal->description(goalGroup.readEntry("desc",""));
    QString goalType = goalGroup.readEntry("type","");
    if (goalType == "countries")
    {
      goal->type(Goal::Countries);
      goal->nbCountries(goalGroup.readEntry("nbCountries",0));
      goal->nbArmiesByCountry(goalGroup.readEntry("nbArmiesByCountry",0));
//       qCDebug(KSIRK_LOG) << "  nb countries: **********************************" << goal->nbCountries();
//       qCDebug(KSIRK_LOG) << "  nbarmies countries: **********************************" << goal->nbArmiesByCountry();
    }
    else if (goalType == "continents" )
    {
      goal->type(Goal::Continents);
      QList<QString> contList = goalGroup.readEntry("continents",QList<QString>());
      foreach(const QString& continentId, contList)
      {
        // Bug 308527. Use only known continents.
        if (continentsList.contains(continentId))
          goal->continents().push_back(continentId);
        else
        {
          qCDebug(KSIRK_LOG) << "Unknown continent " << continentId << " in skin " << m_skin ;
        }
      }
    }
    else if (goalType == "player" )
    {
      goal->type(Goal::GoalPlayer);
      unsigned int nb = goalGroup.readEntry("nbCountriesFallback",0);
      goal->nbCountries(nb);
    }
    automaton->goals().push_back(goal);
  }

  foreach (const QString &countryName, countriesList)
  {
    Country *country = countryNamed(countryName);

    if (!country)
    {
      continue;
    }

//     qCDebug(KSIRK_LOG) << "building neighbours list of " << countryName;
    QList< Country* > theNeighbours;
    KConfigGroup countryGroup = config.group(countryName);
    QList<QString> theNeighboursIds = countryGroup.readEntry("neighbours",QList<QString>());
//     int neighbourId;
    foreach(const QString& neighbourId, theNeighboursIds)
    {
      Country *c = countryNamed(neighbourId);
      if (c)
      {
        theNeighbours.push_back(c);
      }
    }

    country-> neighbours(theNeighbours);
  }
  buildMap();

  qCDebug(KSIRK_LOG) << "OUT";
}

ONU::~ONU()
{
  delete m_timerFast;

  QList<Country*>::iterator countriesIt, countriesIt_end;
  countriesIt = countries.begin(); countriesIt_end = countries.end();
  for (; countriesIt != countriesIt_end; countriesIt++)
  {
    delete *countriesIt;
  }

  QList<Nationality*>::iterator nationalitiesIt, nationalitiesIt_end;
  nationalitiesIt = nationalities.begin(); nationalitiesIt_end = nationalities.end();
  for (; nationalitiesIt != nationalitiesIt_end; nationalitiesIt++)
  {
    delete *nationalitiesIt;
  }
  
  QList<Continent*>::iterator continentsIt, continentsIt_end;
  continentsIt = m_continents.begin(); continentsIt_end = m_continents.end();
  for (; continentsIt != continentsIt_end; continentsIt++)
  {
    delete *continentsIt;
  }

}


/** This method returns a pointer to the country that contains the point (x,y).
If there is no country at (x,y), the functions returns 0. */
Country* ONU::countryAt(const QPointF& point)
{
//    qCDebug(KSIRK_LOG) << "ONU::countryAt x y " << x << " " << y;
    QPointF norm = point;
    norm /= m_zoom;
    if ( norm.x() < 0 || norm.x() >= countriesMask.width()
      || norm.y() < 0 || norm.y() >= countriesMask.height() )
      return nullptr;

    int index = qBlue(countriesMask.pixel(norm.toPoint()));
//    qCDebug(KSIRK_LOG) << "OUT ONU::countryAt: " << index;
    if (index >= countries.size()) return nullptr;
    return countries.at(index);
}

void ONU::reset()
{
  qCDebug(KSIRK_LOG);
  foreach (Country* country, countries)
  {
    country-> reset();
  }
}


QList<Country*>& ONU::getCountries()
{
  return countries;
}

QList<Nationality*>& ONU::getNationalities()
{
  return nationalities;
}

/** Read property of QList<Continent*> continents. */
const QList<Continent*>& ONU::getContinents() const
{
  return m_continents;
}

QList<Continent*>& ONU::getContinents()
{
  return m_continents;
}

/**
  * Returns the list of countries neighbours of this country that does not
  * belongs to the argument player.
  */
QList<Country*> ONU::neighboursBelongingTo(const Country& country, const Player* player)
{
  QList<Country*> list;
  foreach (Country *c,  country.neighbours())
  {
    if ((country.communicateWith(c)) && (c-> owner() == player))
      {list.push_back(c);}
  }
  return list;
}

/**
  * Returns the list of countries neighbours of this country that does not
  * belongs to the argument player.
  */
QList<Country*> ONU::neighboursNotBelongingTo(const Country& country, const Player* player)
{
  QList<Country*> list;
  foreach (Country *c,  country.neighbours())
  {
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
  if (name.isEmpty())
  {
//     qCDebug(KSIRK_LOG) << "request for country with empty name";
    return nullptr;
  }

  foreach (Country *c, countries)
  {
    if (c-> name() == name)
      return c;
  }

//   qCDebug(KSIRK_LOG) << "request for country" << name << "which doesn't seem to exist.";
  return nullptr;
}

/** @return the number of countries in the world */
unsigned int ONU::getNbCountries() const
{
    return(countries.size());
}

void ONU::saveXml(QTextStream& xmlStream)
{
  xmlStream << "<ONU file=\"" << m_configFileName << "\" >";

  xmlStream << "<countries>";
  foreach (Country *c, countries)
  {
      c->saveXml(xmlStream);
  }
  xmlStream << "</countries>";
  xmlStream << "</ONU>";  
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
  foreach (Nationality *n, nationalities)
  {
    if (n->name() == name)
    {
      return n;
    }
  }
  return nullptr;
}

void ONU::sendCountries(QDataStream& stream)
{
  stream << quint32(countries.size());
  foreach (Country* country, countries)
  {
//     qCDebug(KSIRK_LOG) << "Sending country " << country->name();
    country->send(stream);
  }
}

/*const Continent* ONU::continentWithId(const unsigned int id) const
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
*/
Continent* ONU::continentNamed(const QString& name)
{
  foreach (Continent *c, m_continents)
  {
    if (c-> name() == name)
      return c;
  }
  return nullptr;
}

void ONU::buildMap()
{
  qCDebug(KSIRK_LOG) << "with zoom="<< m_zoom;
  //QSize size((int)(m_automaton->rendererFor(m_skin).defaultSize().width()*m_zoom),(int)(m_automaton->rendererFor(m_skin).defaultSize().height()*m_zoom));
  if (!QPixmapCache::find(m_skin+"map"+QString::number(m_width)+QString::number(m_height), &m_map))
  {
    // Pixmap isn't in the cache, create it and insert to cache
    QSize size((int)(m_width),(int)(m_height));
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter p(&image);
    m_automaton->rendererFor(m_skin).render(&p, "map");
    m_map = QPixmap::fromImage(image);
    
    
    QPainter painter(&m_map);
    QFont foregroundFont(m_font.family, m_font.size, m_font.weight, m_font.italic);
    QFont backgroundFont(m_font.family, m_font.size, QFont::Normal, m_font.italic);

    foreach (Country* country, countries)
    {
      const QString& countryName = i18n(country->name().toUtf8().data());
      if (m_font.backgroundColor != "none")
      {
        painter.setPen(m_font.backgroundColor);
        painter.setFont(backgroundFont);
        QRect countryNameRect = painter.fontMetrics().boundingRect(countryName);
        painter.drawText(
          int( (country->centralPoint().x()*m_zoom) - (countryNameRect.width()/2) + 1 ),
         // HACK HACK see the same below
          int( (country->centralPoint().y()*m_zoom) + 4/*(countryNameRect.height()/2)*/ + 1 ),
          countryName);
      }
      painter.setPen(m_font.foregroundColor);
      painter.setFont(foregroundFont);
      QRect countryNameRect = painter.fontMetrics().boundingRect(countryName);
//       qCDebug(KSIRK_LOG) << countryName << "countryNameRect=" << countryNameRect;
//       qCDebug(KSIRK_LOG) << "draw at" << int( (country->centralPoint().x()*m_zoom) - (countryNameRect.width()/2) ) <<
//      int( (country->centralPoint().y()*m_zoom) - (countryNameRect.height()/2) );
      
      painter.drawText(
        int( (country->centralPoint().x()*m_zoom) - (countryNameRect.width()/2) ),
        // HACK HACK why is this 4 necessary below instead of the commented correction ???
        int( (country->centralPoint().y()*m_zoom) + 4/*- (countryNameRect.height()/2)*/ ),
        countryName);
    }

    QPixmapCache::insert(m_skin+"map"+QString::number(m_width)+QString::number(m_height), m_map);
  }
}

void ONU::applyZoomFactor(qreal zoomFactor)
{
/** Zoom 1: First method (take a long time to zoom) :
*/
//   qCDebug(KSIRK_LOG) << "zoomFactor=" << zoomFactor << "old zoom=" << m_zoom;
//   qCDebug(KSIRK_LOG) << "new zoom=" << m_zoom;

  m_zoom *= zoomFactor;

  //m_font.size = (int)(m_font.size*m_zoom);
  //m_width = (unsigned int)(m_width *m_zoom);
  //m_height = (unsigned int)(m_height *m_zoom);

  buildMap();

  foreach (Country* country, countries)
  {
    country->createArmiesSprites();
  }
 
}

void ONU::applyZoomFactorFast(qreal zoomFactor)		//benj
{

/** Zoom 2 : Second method , Very performent.  Carefull ! Can cause the game to lag. 
 To try this, comment all the first method and uncomment these lines
*/
  qCDebug(KSIRK_LOG) << "zoomFactor FASTTTTTTT";
  int nbLimitZooms = 6;

  //Application of zoom
  if (zoomFactor > 1 && m_nbZooms < nbLimitZooms)
  {
    m_font.size = (int)(m_font.size*zoomFactor);
    m_width = (unsigned int)(m_width *zoomFactor);
    m_height = (unsigned int)(m_height *zoomFactor);

      m_nbZooms++; // zoom forward
      m_automaton->game()->frame()->scale(zoomFactor, zoomFactor);
      m_zoomFactorFinal *= zoomFactor;

      //starting timer
      /*if  (m_timerFast->isActive())
      {
        m_timerFast->stop();
      }
      m_timerFast->start(4000);
      m_timerFast->setSingleShot(true);*/

  }
  else if (zoomFactor < 1 && m_nbZooms > -nbLimitZooms)
  {
    m_font.size = (int)(m_font.size*zoomFactor);
    m_width = (unsigned int)(m_width *zoomFactor);
    m_height = (unsigned int)(m_height *zoomFactor);

      m_nbZooms--; // zoom backward
      m_automaton->game()->frame()->scale(zoomFactor, zoomFactor);
      m_zoomFactorFinal *= zoomFactor;

      //starting timer
      /*if  (m_timerFast->isActive())
      {
        m_timerFast->stop();
      }
      m_timerFast->start(4000);
      m_timerFast->setSingleShot(true);*/
  }
}

double ONU::zoom() const
{
  if (m_automaton->game()->currentWidgetType() == KGameWindow::Map) {
    return m_zoom;
  }
  return m_zoomArena;
}


QSvgRenderer* ONU::renderer()
{
  return &m_automaton->rendererFor(m_skin);
}


KGameSvgDocument* ONU::svgDom()
{
  return &m_automaton->svgDomFor(m_skin);
}

unsigned int ONU::indexOfCountry(Country* c) const
{
  return countries.indexOf(c);
}


/** the SLOTS METHODS FOR THE ONU CLASS*/
void ONU::changingZoom()
{
/*
  m_automaton->game()->frame()->resetTransform();
  qreal m_zo=m_zoomFactorFinal;
  m_zoomFactorFinal=1;
  applyZoomFactor(m_zo);*/

 // m_automaton->game()->frame()->scale(1/m_zoomFactorFinal, 1/m_zoomFactorFinal);

  m_automaton->game()->frame()->resetTransform();
  applyZoomFactor(m_zoomFactorFinal);
  m_zoomFactorFinal=1.0;
}

/**END OF SLOTS METHODS FOR THE ONU*/
} // closing namespace GameLogic
} // closing namespace Ksirk

#include "moc_onu.cpp"
