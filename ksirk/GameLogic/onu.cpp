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
#include "onu.h"
#include "Sprites/skinSpritesData.h"
#include "goal.h"
#include "gameautomaton.h"
#include "kgamewin.h"

#define KDE_NO_COMPAT
#include <QFile>
#include <qdom.h>
#include <QPainter>
#include <QPixmap>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kconfiggroup.h>

namespace Ksirk
{

namespace GameLogic
{

ONU::ONU(GameAutomaton* automaton,
  const QString& configFileName):
  m_automaton(automaton),
  m_configFileName(configFileName),
  countries(),
  nationalities(),
  m_continents(),
  m_zoom(1.0),
  m_zoomArena(1.0),
  m_nbZooms(0),
  m_zoomFactorFinal(1),
  m_renderer(0)
{

  kDebug() << "ONU constructor: " << m_configFileName << endl;
  m_font.family = "URW Chancery L";
  m_font.size = (uint)(13*m_zoom);
  m_font.weight = QFont::Bold;
  m_font.italic = true;
  m_font.foregroundColor = "black";
  m_font.backgroundColor = "white";
  
  m_timerFast=new QTimer();		//instanciation of the timer 
  QObject::connect(m_timerFast,SIGNAL(timeout()), this, SLOT(changingZoom()));	//connect the timer to the good slot
  
  Sprites::SkinSpritesData::changeable().init();
  unsigned int nationalityId = 0;
  unsigned int continentId = 0;
  KConfig config(configFileName);

  KConfigGroup onugroup = config.group("onu");

  kDebug() << "ONU XML format version: " << onugroup.readEntry("format-version") << endl;
  QString formatVersion = onugroup.readEntry("format-version");

  if (formatVersion != ONU_FILE_FORMAT_VERSION)
  {
    KMessageBox::error(0,
            i18n("Error - Invalid skin definition file format. Expected %1 and got %2",QString(ONU_FILE_FORMAT_VERSION),formatVersion),
            i18n("Fatal Error"));
    exit(1);
  }

  m_name = onugroup.readEntry("name");
  m_skin = onugroup.readEntry("skinpath");
  kDebug() << "skin snapshot file: " << KGlobal::dirs()-> findResource("appdata", m_skin + "/Images/snapshot.jpg") << endl;
  m_snapshot = QPixmap(KGlobal::dirs()-> findResource("appdata", m_skin + "/Images/snapshot.jpg"));
  if (m_snapshot.isNull())
  {
    kError() << "Was not able to load the snapshot image: " 
      << KGlobal::dirs()-> findResource("appdata", m_skin + "/Images/snapshot.jpg") 
      << endl;
  }
  m_width  = onugroup.readEntry("width",0);
  m_height  = onugroup.readEntry("height",0);
  m_description = onugroup.readEntry("desc");
  countries.resize(onugroup.readEntry("nb-countries",0));
  nationalities.resize(onugroup.readEntry("nb-nationalities",0));
  m_continents.resize(onugroup.readEntry("nb-continents",0));
//    root.attribute("map");
  QString poolString = onugroup.readEntry("pool");
  kDebug() << "Pool path: " << poolString << endl;
  kDebug() << "Searching resource: " << (m_skin + '/' + poolString) << endl;
  QString poolFileName = KGlobal::dirs()-> findResource("appdata", m_skin + '/' + poolString);
  kDebug() << "Pool file name: " << poolFileName << endl;
  if (poolFileName.isEmpty())
  {
      KMessageBox::error(0, 
                         i18n("Pool filename not found\nProgram cannot continue"),
                         i18n("Error !"));
      exit(2);
  }
  m_map = QPixmap();
  m_renderer.load(poolFileName);
  m_svgDom.load(poolFileName);

  QString mapMaskFileName = KGlobal::dirs()-> findResource("appdata", m_skin + '/' + onugroup.readEntry("map-mask"));
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

  Sprites::SkinSpritesData::changeable().intData("fighters-flag-y-diff", onugroup.readEntry("fighters-flag-y-diff",0));
  Sprites::SkinSpritesData::changeable().intData("width-between-flag-and-fighter", onugroup.readEntry("width-between-flag-and-fighter",0));


  Sprites::SkinSpritesData::changeable().intData("flag-width", config.group("flag").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("flag-height", config.group("flag").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("flag-frames", config.group("flag").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("flag-versions", config.group("flag").readEntry("versions",0));

  Sprites::SkinSpritesData::changeable().strData("infantry-id", config.group("infantry").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("infantry-width", config.group("infantry").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("infantry-height", config.group("infantry").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("infantry-frames", config.group("infantry").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("infantry-versions", config.group("infantry").readEntry("versions",0));

  Sprites::SkinSpritesData::changeable().strData("infantry1-id", config.group("infantry1").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("infantry1-width", config.group("infantry1").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("infantry1-height", config.group("infantry1").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("infantry1-frames", config.group("infantry1").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("infantry1-versions", config.group("infantry1").readEntry("versions",0));

  Sprites::SkinSpritesData::changeable().strData("infantry2-id", config.group("infantry2").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("infantry2-width", config.group("infantry2").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("infantry2-height", config.group("infantry2").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("infantry2-frames", config.group("infantry2").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("infantry2-versions", config.group("infantry2").readEntry("versions",0));

  Sprites::SkinSpritesData::changeable().strData("infantry3-id", config.group("infantry3").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("infantry3-width", config.group("infantry3").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("infantry3-height", config.group("infantry3").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("infantry3-frames", config.group("infantry3").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("infantry3-versions", config.group("infantry3").readEntry("versions",0));

  Sprites::SkinSpritesData::changeable().strData("cavalry-id", config.group("cavalry").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("cavalry-width", config.group("cavalry").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("cavalry-height", config.group("cavalry").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("cavalry-frames", config.group("cavalry").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("cavalry-versions", config.group("cavalry").readEntry("versions",0));

  Sprites::SkinSpritesData::changeable().strData("cannon-id", config.group("cannon").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("cannon-width", config.group("cannon").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("cannon-height", config.group("cannon").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("cannon-frames", config.group("cannon").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("cannon-versions", config.group("cannon").readEntry("versions",0));

  Sprites::SkinSpritesData::changeable().strData("firing-id", config.group("firing").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("firing-width", config.group("firing").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("firing-height", config.group("firing").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("firing-frames", config.group("firing").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("firing-versions", config.group("firing").readEntry("versions",0));

  Sprites::SkinSpritesData::changeable().strData("exploding-id", config.group("exploding").readEntry("id"));
  Sprites::SkinSpritesData::changeable().intData("exploding-width", config.group("exploding").readEntry("width",0));
  Sprites::SkinSpritesData::changeable().intData("exploding-height", config.group("cannon").readEntry("height",0));
  Sprites::SkinSpritesData::changeable().intData("exploding-frames", config.group("exploding").readEntry("frames",0));
  Sprites::SkinSpritesData::changeable().intData("exploding-versions", config.group("exploding").readEntry("versions",0));

  KConfigGroup fontgroup = config.group("font");
  m_font.family = fontgroup.readEntry("family","URW Chancery L");
  m_font.size = fontgroup.readEntry("size",(uint)(13*m_zoom));
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
  QString country;
  foreach (country, countriesList)
  { 
    KConfigGroup countryGroup = config.group(country);
    unsigned int id = countryGroup.readEntry("id",0);
    QString name = country;
    QPointF anchorPoint = countryGroup.readEntry("anchor-point",QPoint())*m_zoom;
    QPointF centralPoint = countryGroup.readEntry("central-point",QPoint())*m_zoom;
    QPointF flagPoint = countryGroup.readEntry("flag-point",QPoint())*m_zoom;
    QPointF cannonPoint = countryGroup.readEntry("cannon-point",QPoint())*m_zoom;
    QPointF cavalryPoint = countryGroup.readEntry("cavalry-point",QPoint())*m_zoom;
    QPointF infantryPoint = countryGroup.readEntry("infantry-point",QPoint())*m_zoom;

//     kDebug() << "Creating country " << name << endl;
//     kDebug() << "\tflag point: " << flagPoint << endl;
//     kDebug() << "\tcentral point: " << centralPoint << endl;
//     kDebug() << "\tcannon point: " << cannonPoint << endl;
//     kDebug() << "\tcavalry point: " << cavalryPoint << endl;
//     kDebug() << "\tinfantry point: " << infantryPoint << endl;
    countries[id] = new Country(automaton, name, anchorPoint, centralPoint,
        flagPoint, cannonPoint, cavalryPoint, infantryPoint, id);
  }
  QStringList nationalitiesList = onugroup.readEntry("nationalities", QStringList());
  QString nationality;
  foreach (nationality, nationalitiesList)
  {
    kDebug() << "Creating nationality " << nationality << endl;
    KConfigGroup nationalityGroup = config.group(nationality);
    QString leader = nationalityGroup.readEntry("leader","");
    QString flag = nationalityGroup.readEntry("flag","");
//         kDebug() << "Creating nationality " << name << " ; flag: " << flag << endl;
    nationalities[nationalityId] = new Nationality(nationality, flag, leader);
    nationalityId++;
  }


  QStringList continentsList = onugroup.readEntry("continents", QStringList());
  QString continent;
  foreach (continent, continentsList)
  {
    KConfigGroup continentGroup = config.group(continent);

    unsigned int id = continentGroup.readEntry("id",0);
    unsigned int bonus = continentGroup.readEntry("bonus",0);
    QList<int> countryIdList = continentGroup.readEntry("continent-countries",QList<int>());
    int countryId;
    std::vector<Country*> continentList;
    foreach(countryId, countryIdList)
    {
      continentList.push_back(countries[countryId]);
    }
//       kDebug() << "Creating continent " << name << endl;
    m_continents[continentId++] = new Continent(continent, continentList, bonus,id);
  }

  QStringList goalsList = onugroup.readEntry("goals", QStringList());
  QString goal;
  foreach (goal, goalsList)
  {
    KConfigGroup goalGroup = config.group(goal);

    Goal* goal = new Goal(automaton);
    goal->description(goalGroup.readEntry("desc",""));
    QString goalType = goalGroup.readEntry("type","");
    if (goalType == "countries")
    {
      goal->type(Goal::Countries);
      goal->nbCountries(goalGroup.readEntry("nbCountries",0));
      goal->nbArmiesByCountry(goalGroup.readEntry("nbArmiesByCountry",0));
      kDebug() << "  nb countries: **********************************" << goal->nbCountries() << endl;
      kDebug() << "  nbarmies countries: **********************************" << goal->nbArmiesByCountry() << endl;
    }
    else if (goalType == "continents" )
    {
      goal->type(Goal::Continents);
      QList<int> continentsList = goalGroup.readEntry("continents",QList<int>());
      int continentId;
      foreach(continentId, continentsList)
      {
        goal->continents().insert(continentId);
      }
    }
    else if (goalType == "player" )
    {
      goal->type(Goal::GoalPlayer);
      unsigned int nb = goalGroup.readEntry("nbCountriesFallback",0);
      goal->nbCountries(nb);
    }
    automaton->goals().insert(goal);
  }

  foreach (country, countriesList)
  {
    std::vector< Country* > theNeighbours;
    KConfigGroup countryGroup = config.group(country);
    QList<int> theNeighboursIds = countryGroup.readEntry("neighbours",QList<int>());
    int neighbourId;
    foreach(neighbourId, theNeighboursIds)
    {

      theNeighbours.push_back(countries[neighbourId]);
    }
    countries.at(countryGroup.readEntry("id",0))-> neighbours(theNeighbours);
  }
  buildMap();

//    kDebug() << "OUT ONU::ONU" << endl;
}

/** This method returns a pointer to the country that contains the point (x,y).
If there is no country at (x,y), the functions returns 0. */
Country* ONU::countryAt(const QPointF& point)
{
//    kDebug() << "ONU::countryAt x y " << x << " " << y << endl;
    QPointF norm = point;
    norm /= m_zoom;
    if ( norm.x() < 0 || norm.x() >= countriesMask.width()
      || norm.y() < 0 || norm.y() >= countriesMask.height() )
      return 0;

    unsigned int index = qBlue(countriesMask.pixel(norm.toPoint()));
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
  if (name.isEmpty())
  {
    return 0;
  }
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
    kDebug() << "Sending country number " << i+1 << " on " << countries.size() << endl;
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
  kDebug() << "with zoom="<< m_zoom << endl;
  //QSize size((int)(m_renderer.defaultSize().width()*m_zoom),(int)(m_renderer.defaultSize().height()*m_zoom));
  QSize size((int)(m_width),(int)(m_height));
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  image.fill(0);
  QPainter p(&image);
  m_renderer.render(&p, "map");
  QPixmap mapPixmap = QPixmap::fromImage(image);

  m_map = mapPixmap;

  QPainter painter(&m_map);
  QFont foregroundFont(m_font.family, m_font.size, m_font.weight, m_font.italic);
  QFont backgroundFont(m_font.family, m_font.size, QFont::Normal, m_font.italic);

  painter.drawPixmap(0,0,mapPixmap);
  
  for (uint i = 0; i < countries.size(); i++)
  {
    Country* country = countries[i];
    const QString& countryName = i18n(country->name().toUtf8().data());
    if (m_font.backgroundColor != "none")
    {
      painter.setPen(m_font.backgroundColor);
      painter.setFont(backgroundFont);
      QRect countryNameRect = painter.fontMetrics().boundingRect(countryName);
      painter.drawText(
        int((country->centralPoint().x()*m_zoom-countryNameRect.width()/2+1)),
        int((country->centralPoint().y()*m_zoom+countryNameRect.height()/2 + 1)),
        countryName);
    }
    painter.setPen(m_font.foregroundColor);
    painter.setFont(foregroundFont);
    QRect countryNameRect = painter.fontMetrics().boundingRect(countryName);
    painter.drawText(
    int((country->centralPoint().x()*m_zoom-countryNameRect.width()/2)),
    int((country->centralPoint().y()*m_zoom+countryNameRect.height()/2)),
        countryName);
  }

}

void ONU::applyZoomFactor(qreal zoomFactor)
{
/** Zoom 1: First method (take a long time to zoom) :
*/
  kDebug() << "zoomFactor=" << zoomFactor << "old zoom=" << m_zoom << endl;
  kDebug() << "new zoom=" << m_zoom << endl;

  m_zoom *= zoomFactor;

  //m_font.size = (unsigned int)(m_font.size*m_zoom);
  //m_width = (unsigned int)(m_width *m_zoom);
  //m_height = (unsigned int)(m_height *m_zoom);

  buildMap();

  std::vector<Country*>::iterator it, it_end;
  it = countries.begin(); it_end = countries.end();
  for ( ; it != it_end; it++ )
  {
    Country* country = *it;
    country->createArmiesSprites();
  }
 
}

void ONU::applyZoomFactorFast(qreal zoomFactor)		//benj
{

/** Zoom 2 : Second method , Very performent.  Carefull ! Can cause the game to lag. 
 To try this, comment all the first method and uncomment these lines
*/
   kDebug() << "zoomFactor FASTTTTTTT"<<endl;
   int nbLimitZooms = 6;

  //Application of zoom
    if (zoomFactor > 1 && m_nbZooms < nbLimitZooms)
    {  
	m_font.size = (unsigned int)(m_font.size*zoomFactor);
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
	m_font.size = (unsigned int)(m_font.size*zoomFactor);
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
  if (m_automaton->game()->currentWidgetType() == KGameWindow::mapType) {
    return m_zoom;
  }
  return m_zoomArena;
}


QSvgRenderer* ONU::renderer()
{
  return &m_renderer;
}


KGameSvgDocument* ONU::svgDom()
{
  return &m_svgDom;
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
