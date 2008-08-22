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
#include "skinSpritesData.h"
#include "goal.h"
#include "../ksirk/KsirkGlobalDefinitions.h"

#define KDE_NO_COMPAT
#include <QFile>
// #include <QDom>
#include <QPainter>
#include <QPixmap>
#include <QGraphicsPixmapItem>

#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kconfiggroup.h>

namespace KsirkSkinEditor
{

ONU::ONU(const QString& configDir):
  m_configDir(configDir),
  m_configFileName(configDir + "/Data/world.desktop"),
  m_countries(),
  m_nationalities(),
  m_continents(),
  m_renderer(0),
  m_itemsMap()
{

  kDebug() << "ONU constructor: " << m_configFileName;
  m_font.family = "URW Chancery L";
  m_font.size = (uint)(13);
  m_font.weight = QFont::Bold;
  m_font.italic = true;
  m_font.foregroundColor = "black";
  m_font.backgroundColor = "white";
  
  SkinSpritesData::changeable().init();
  unsigned int nationalityId = 0;
  unsigned int continentId = 0;
  KConfig config(m_configFileName);

  KConfigGroup onugroup = config.group("onu");

  kDebug() << "ONU XML format version: " << onugroup.readEntry("format-version");
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
  kDebug() << "skin snapshot file: " << m_configDir + "/Images/snapshot.jpg";
  m_snapshot = m_configDir + "/Images/snapshot.jpg";
  if (m_snapshot.isNull())
  {
    kError() << "Was not able to load the snapshot image: " 
    << m_configDir + "/Images/snapshot.jpg";
  }
  m_width  = onugroup.readEntry("width",0);
  m_height = onugroup.readEntry("height",0);
  m_description = onugroup.readEntry("desc");
//   countries.resize(onugroup.readEntry("nb-countries",0));
//   nationalities.resize(onugroup.readEntry("nb-nationalities",0));
//   m_continents.resize(onugroup.readEntry("nb-continents",0));
//    root.attribute("map");
  m_poolString = onugroup.readEntry("pool");
  kDebug() << "Pool path: " << m_poolString;
  kDebug() << "Searching resource: " << (m_configDir + '/' + m_poolString);
  QString poolFileName = m_configDir + '/' + m_poolString;
  kDebug() << "Pool file name: " << poolFileName;
  if (poolFileName.isEmpty())
  {
      KMessageBox::error(0, 
                         i18n("Pool filename not found\nProgram cannot continue"),
                         i18n("Error !"));
      exit(2);
  }
  m_map = QPixmap();
  m_renderer.load(poolFileName);

  QString mapMaskFileName = m_configDir + '/' + onugroup.readEntry("map-mask");
  kDebug() << "Map mask file name: " << mapMaskFileName;
  if (mapMaskFileName.isNull())
  {
      KMessageBox::error(0, 
                         i18n("Map mask image not found\nProgram cannot continue"),
                         i18n("Error !"));
      exit(2);
  }
  kDebug() << "Loading map mask file: " << mapMaskFileName;
  m_countriesMask = QImage(mapMaskFileName);

  SkinSpritesData::changeable().intData("fighters-flag-y-diff", onugroup.readEntry("fighters-flag-y-diff",0));
  SkinSpritesData::changeable().intData("width-between-flag-and-fighter", onugroup.readEntry("width-between-flag-and-fighter",0));


  kDebug() << "Loading flag data";
  SkinSpritesData::changeable().intData("flag-width", config.group("flag").readEntry("width",0));
  SkinSpritesData::changeable().intData("flag-height", config.group("flag").readEntry("height",0));
  SkinSpritesData::changeable().intData("flag-frames", config.group("flag").readEntry("frames",0));
  SkinSpritesData::changeable().intData("flag-versions", config.group("flag").readEntry("versions",0));

  kDebug() << "Loading infantry data";
  SkinSpritesData::changeable().strData("infantry-id", config.group("infantry").readEntry("id"));
  SkinSpritesData::changeable().intData("infantry-width", config.group("infantry").readEntry("width",0));
  SkinSpritesData::changeable().intData("infantry-height", config.group("infantry").readEntry("height",0));
  SkinSpritesData::changeable().intData("infantry-frames", config.group("infantry").readEntry("frames",0));
  SkinSpritesData::changeable().intData("infantry-versions", config.group("infantry").readEntry("versions",0));

  SkinSpritesData::changeable().strData("infantry1-id", config.group("infantry1").readEntry("id"));
  SkinSpritesData::changeable().intData("infantry1-width", config.group("infantry1").readEntry("width",0));
  SkinSpritesData::changeable().intData("infantry1-height", config.group("infantry1").readEntry("height",0));
  SkinSpritesData::changeable().intData("infantry1-frames", config.group("infantry1").readEntry("frames",0));
  SkinSpritesData::changeable().intData("infantry1-versions", config.group("infantry1").readEntry("versions",0));

  SkinSpritesData::changeable().strData("infantry2-id", config.group("infantry2").readEntry("id"));
  SkinSpritesData::changeable().intData("infantry2-width", config.group("infantry2").readEntry("width",0));
  SkinSpritesData::changeable().intData("infantry2-height", config.group("infantry2").readEntry("height",0));
  SkinSpritesData::changeable().intData("infantry2-frames", config.group("infantry2").readEntry("frames",0));
  SkinSpritesData::changeable().intData("infantry2-versions", config.group("infantry2").readEntry("versions",0));

  SkinSpritesData::changeable().strData("infantry3-id", config.group("infantry3").readEntry("id"));
  SkinSpritesData::changeable().intData("infantry3-width", config.group("infantry3").readEntry("width",0));
  SkinSpritesData::changeable().intData("infantry3-height", config.group("infantry3").readEntry("height",0));
  SkinSpritesData::changeable().intData("infantry3-frames", config.group("infantry3").readEntry("frames",0));
  SkinSpritesData::changeable().intData("infantry3-versions", config.group("infantry3").readEntry("versions",0));

  kDebug() << "Loading cavalry data";
  SkinSpritesData::changeable().strData("cavalry-id", config.group("cavalry").readEntry("id"));
  SkinSpritesData::changeable().intData("cavalry-width", config.group("cavalry").readEntry("width",0));
  SkinSpritesData::changeable().intData("cavalry-height", config.group("cavalry").readEntry("height",0));
  SkinSpritesData::changeable().intData("cavalry-frames", config.group("cavalry").readEntry("frames",0));
  SkinSpritesData::changeable().intData("cavalry-versions", config.group("cavalry").readEntry("versions",0));

  kDebug() << "Loading cannon data";
  SkinSpritesData::changeable().strData("cannon-id", config.group("cannon").readEntry("id"));
  SkinSpritesData::changeable().intData("cannon-width", config.group("cannon").readEntry("width",0));
  SkinSpritesData::changeable().intData("cannon-height", config.group("cannon").readEntry("height",0));
  SkinSpritesData::changeable().intData("cannon-frames", config.group("cannon").readEntry("frames",0));
  SkinSpritesData::changeable().intData("cannon-versions", config.group("cannon").readEntry("versions",0));

  kDebug() << "Loading firing data";
  SkinSpritesData::changeable().strData("firing-id", config.group("firing").readEntry("id"));
  SkinSpritesData::changeable().intData("firing-width", config.group("firing").readEntry("width",0));
  SkinSpritesData::changeable().intData("firing-height", config.group("firing").readEntry("height",0));
  SkinSpritesData::changeable().intData("firing-frames", config.group("firing").readEntry("frames",0));
  SkinSpritesData::changeable().intData("firing-versions", config.group("firing").readEntry("versions",0));

  kDebug() << "Loading exploding data";
  SkinSpritesData::changeable().strData("exploding-id", config.group("exploding").readEntry("id"));
  SkinSpritesData::changeable().intData("exploding-width", config.group("exploding").readEntry("width",0));
  SkinSpritesData::changeable().intData("exploding-height", config.group("cannon").readEntry("height",0));
  SkinSpritesData::changeable().intData("exploding-frames", config.group("exploding").readEntry("frames",0));
  SkinSpritesData::changeable().intData("exploding-versions", config.group("exploding").readEntry("versions",0));

  kDebug() << "Loading font data";
  KConfigGroup fontgroup = config.group("font");
  m_font.family = fontgroup.readEntry("family","URW Chancery L");
  m_font.size = fontgroup.readEntry("size",(uint)(13));
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

  kDebug() << "Loading countries data";
  QStringList countriesList = onugroup.readEntry("countries", QStringList());
  while (m_countries.size() < countriesList.size())
  {
    m_countries.push_back(0);
  }
  foreach (const QString &country, countriesList)
  { 
    kDebug() << "Loading"<<country<<"data";
    KConfigGroup countryGroup = config.group(country);
    unsigned int id = countryGroup.readEntry("id",0);
    QString name = country;
    QPointF anchorPoint = countryGroup.readEntry("anchor-point",QPoint());
    QPointF centralPoint = countryGroup.readEntry("central-point",QPoint());
    QPointF flagPoint = countryGroup.readEntry("flag-point",QPoint());
    QPointF cannonPoint = countryGroup.readEntry("cannon-point",QPoint());
    QPointF cavalryPoint = countryGroup.readEntry("cavalry-point",QPoint());
    QPointF infantryPoint = countryGroup.readEntry("infantry-point",QPoint());

    m_countries[id] = new Country(name, anchorPoint, centralPoint,
        flagPoint, cannonPoint, cavalryPoint, infantryPoint, id);
  }
  kDebug() << "Loading nationalities data";
  QStringList nationalitiesList = onugroup.readEntry("nationalities", QStringList());
  while (m_nationalities.size() < nationalitiesList.size())
  {
    m_nationalities.push_back(0);
  }
  foreach (const QString &nationality, nationalitiesList)
  {
    kDebug() << "Creating nationality " << nationality;
    KConfigGroup nationalityGroup = config.group(nationality);
    QString leader = nationalityGroup.readEntry("leader","");
    QString flag = nationalityGroup.readEntry("flag","");
//         kDebug() << "Creating nationality " << name << " ; flag: " << flag;
    m_nationalities[nationalityId] = new Nationality(nationality, flag, leader);
    nationalityId++;
  }


  kDebug() << "Loading continents data";
  QStringList continentsList = onugroup.readEntry("continents", QStringList());
  while (m_continents.size() < continentsList.size())
  {
    m_continents.push_back(0);
  }
  foreach (const QString &continent, continentsList)
  {
    kDebug() << "Loading"<<continent<<"data";
    KConfigGroup continentGroup = config.group(continent);

    unsigned int id = continentGroup.readEntry("id",0);
    unsigned int bonus = continentGroup.readEntry("bonus",0);
    QList<int> countryIdList = continentGroup.readEntry("continent-countries",QList<int>());
    int countryId;
    QList<Country*> continentList;
    foreach(countryId, countryIdList)
    {
      continentList.push_back(m_countries[countryId]);
    }
//       kDebug() << "Creating continent " << name;
    m_continents[continentId++] = new Continent(continent, continentList, bonus,id);
  }

  kDebug() << "Loading goals data";
  QStringList goalsList = onugroup.readEntry("goals", QStringList());
  foreach (const QString &_goal, goalsList)
  {
    kDebug() << "init goal " << _goal;
    KConfigGroup goalGroup = config.group(_goal);

    Goal* goal = new Goal();
    goal->description(goalGroup.readEntry("desc",""));
    QString goalType = goalGroup.readEntry("type","");
    if (goalType == "countries")
    {
      goal->type(Goal::Countries);
      goal->nbCountries(goalGroup.readEntry("nbCountries",0));
      goal->nbArmiesByCountry(goalGroup.readEntry("nbArmiesByCountry",0));
      kDebug() << "  nb countries: **********************************" << goal->nbCountries();
      kDebug() << "  nbarmies countries: **********************************" << goal->nbArmiesByCountry();
    }
    else if (goalType == "continents" )
    {
      goal->type(Goal::Continents);
      QList<int> continentsList = goalGroup.readEntry("continents",QList<int>());
      int continentId;
      foreach(continentId, continentsList)
      {
        goal->continents().push_back(continentId);
      }
    }
    else if (goalType == "player" )
    {
      goal->type(Goal::GoalPlayer);
      unsigned int nb = goalGroup.readEntry("nbCountriesFallback",0);
      goal->nbCountries(nb);
    }
    m_goals.push_back(goal);
  }

  foreach (const QString &country, countriesList)
  {
    kDebug() << "building neighbours list of " << country;
    QList< Country* > theNeighbours;
    KConfigGroup countryGroup = config.group(country);
    QList<int> theNeighboursIds = countryGroup.readEntry("neighbours",QList<int>());
    int neighbourId;
    foreach(neighbourId, theNeighboursIds)
    {

      theNeighbours.push_back(m_countries[neighbourId]);
    }
    m_countries.at(countryGroup.readEntry("id",0))-> neighbours(theNeighbours);
  }
  kDebug() << "Building map";
  buildMap();

  int flagWidth = SkinSpritesData::changeable().intData("flag-width");
  int flagHeight = SkinSpritesData::changeable().intData("flag-height");
  int flagFrames = SkinSpritesData::changeable().intData("flag-frames");
  int flagVersions = SkinSpritesData::changeable().intData("flag-versions");
  m_flagIcon = QPixmap(pixmapForId(nationalities()[0]->name().toLower(),flagWidth*flagFrames,flagHeight*flagVersions).copy(0,0,flagWidth,flagHeight));
                     
  int infantryWidth = SkinSpritesData::changeable().intData("infantry-width");
  int infantryHeight = SkinSpritesData::changeable().intData("infantry-height");
  int infantryFrames = SkinSpritesData::changeable().intData("infantry-frames");
  int infantryVersions = SkinSpritesData::changeable().intData("infantry-versions");
  m_infantryIcon = QPixmap(
  pixmapForId("infantry",infantryWidth*infantryFrames,infantryHeight*infantryVersions).copy(0,0,infantryWidth,infantryHeight));

  int cavalryWidth = SkinSpritesData::changeable().intData("cavalry-width");
  int cavalryHeight = SkinSpritesData::changeable().intData("cavalry-height");
  int cavalryFrames = SkinSpritesData::changeable().intData("cavalry-frames");
  int cavalryVersions = SkinSpritesData::changeable().intData("cavalry-versions");
  m_cavalryIcon = QPixmap(
  pixmapForId("cavalry",cavalryWidth*cavalryFrames,cavalryHeight*cavalryVersions).copy(0,0,cavalryWidth,cavalryHeight));
  
  int cannonWidth = SkinSpritesData::changeable().intData("cannon-width");
  int cannonHeight = SkinSpritesData::changeable().intData("cannon-height");
  int cannonFrames = SkinSpritesData::changeable().intData("cannon-frames");
  int cannonVersions = SkinSpritesData::changeable().intData("cannon-versions");
  m_cannonIcon = QPixmap(
  pixmapForId("cannon",cannonWidth*cannonFrames,cannonHeight*cannonVersions).copy(0,0,cannonWidth,cannonHeight));

  kDebug() << "OUT";
}

ONU::~ONU()
{
  QList<Country*>::iterator countriesIt, countriesIt_end;
  countriesIt = m_countries.begin(); countriesIt_end = m_countries.end();
  for (; countriesIt != countriesIt_end; countriesIt++)
  {
    delete *countriesIt;
  }

  QList<Nationality*>::iterator nationalitiesIt, nationalitiesIt_end;
  nationalitiesIt = m_nationalities.begin(); nationalitiesIt_end = m_nationalities.end();
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

void ONU::saveConfig(const QString& configFileName)
{
  if (!configFileName.isNull())
  {
    m_configFileName = configFileName;
  }
  kDebug() << m_configFileName;
  m_font.family = "URW Chancery L";
  m_font.size = (uint)(13);
  m_font.weight = QFont::Bold;
  m_font.italic = true;
  m_font.foregroundColor = "black";
  m_font.backgroundColor = "white";

  KConfig config(m_configFileName);

  KConfigGroup onugroup = config.group("onu");

  kDebug() << "ONU XML format version: ";
  onugroup.readEntry("format-version",ONU_FILE_FORMAT_VERSION);

  onugroup.writeEntry("name",m_name);
  onugroup.writeEntry("skinpath",m_skin);
  onugroup.writeEntry("width",m_width);
  onugroup.writeEntry("height",m_height);
  onugroup.writeEntry("desc",m_description);
  onugroup.writeEntry("pool",m_poolString);

  onugroup.writeEntry("fighters-flag-y-diff", SkinSpritesData::changeable().intData("fighters-flag-y-diff"));
  onugroup.writeEntry("width-between-flag-and-fighter",SkinSpritesData::changeable().intData("width-between-flag-and-fighter"));


  kDebug() << "Saving flag data";
  config.group("flag").writeEntry("width",SkinSpritesData::changeable().intData("flag-width"));
  config.group("flag").writeEntry("height",SkinSpritesData::changeable().intData("flag-height"));
  config.group("flag").writeEntry("frames",SkinSpritesData::changeable().intData("flag-frames"));
  config.group("flag").readEntry("versions",SkinSpritesData::changeable().intData("flag-versions"));

  kDebug() << "Saving infantry data";
  config.group("infantry").writeEntry("id",SkinSpritesData::changeable().strData("infantry-id"));
  config.group("infantry").writeEntry("width",SkinSpritesData::changeable().intData("infantry-width"));
  config.group("infantry").writeEntry("height",SkinSpritesData::changeable().intData("infantry-height"));
  config.group("infantry").writeEntry("frames",SkinSpritesData::changeable().intData("infantry-frames"));
  config.group("infantry").writeEntry("versions",SkinSpritesData::changeable().intData("infantry-versions"));

  kDebug() << "Saving cavalry data";
  config.group("cavalry").writeEntry("id",SkinSpritesData::changeable().strData("cavalry-id"));
  config.group("cavalry").writeEntry("width",SkinSpritesData::changeable().intData("cavalry-width"));
  config.group("cavalry").writeEntry("height",SkinSpritesData::changeable().intData("cavalry-height"));
  config.group("cavalry").writeEntry("frames",SkinSpritesData::changeable().intData("cavalry-frames"));
  config.group("cavalry").writeEntry("versions",SkinSpritesData::changeable().intData("cavalry-versions"));

  kDebug() << "Saving cannon data";
  config.group("cannon").writeEntry("id",SkinSpritesData::changeable().strData("cannon-id"));
  config.group("cannon").writeEntry("width",SkinSpritesData::changeable().intData("cannon-width"));
  config.group("cannon").writeEntry("height",SkinSpritesData::changeable().intData("cannon-height"));
  config.group("cannon").writeEntry("frames",SkinSpritesData::changeable().intData("cannon-frames"));
  config.group("cannon").writeEntry("versions",SkinSpritesData::changeable().intData("cannon-versions"));

  kDebug() << "Saving firing data";
  config.group("firing").writeEntry("id",SkinSpritesData::changeable().strData("firing-id"));
  config.group("firing").writeEntry("width",SkinSpritesData::changeable().intData("firing-width"));
  config.group("firing").writeEntry("height",SkinSpritesData::changeable().intData("firing-height"));
  config.group("firing").writeEntry("frames",SkinSpritesData::changeable().intData("firing-frames"));
  config.group("firing").writeEntry("versions",SkinSpritesData::changeable().intData("firing-versions"));

  kDebug() << "Saving exploding data";
  config.group("exploding").writeEntry("id",SkinSpritesData::changeable().strData("exploding-id"));
  config.group("exploding").writeEntry("width",SkinSpritesData::changeable().intData("exploding-width"));
  config.group("cannon").writeEntry("height",SkinSpritesData::changeable().intData("exploding-height"));
  config.group("exploding").writeEntry("frames",SkinSpritesData::changeable().intData("exploding-frames"));
  config.group("exploding").writeEntry("versions",SkinSpritesData::changeable().intData("exploding-versions"));

  kDebug() << "Saving font data";
  KConfigGroup fontgroup = config.group("font");
  fontgroup.writeEntry("family",m_font.family);
  fontgroup.writeEntry("size",m_font.size);
  switch (m_font.weight)
  {
    case QFont::Normal:
      fontgroup.writeEntry("weight", "normal");
      break;
    case QFont::Light:
      fontgroup.writeEntry("weight", "light");
      break;
    case QFont::DemiBold:
      fontgroup.writeEntry("weight", "demibold");
      break;
    case QFont::Bold:
      fontgroup.writeEntry("weight", "bold");
      break;
    case QFont::Black:
      fontgroup.writeEntry("weight", "black");
      break;
    default:
      fontgroup.writeEntry("weight", "normal");
  }
  fontgroup.writeEntry("italic", m_font.italic);
  fontgroup.writeEntry("foreground-color", m_font.foregroundColor);
  fontgroup.writeEntry("background-color", m_font.backgroundColor);

  kDebug() << "Saving countries data";
  QStringList countriesList;
  foreach (Country* country, m_countries)
  {
    countriesList.push_back(country->name());
  }
  onugroup.writeEntry("countries", countriesList);
  unsigned int countryNum = 0;
  foreach (Country* country, m_countries)
  {
    kDebug() << "Saving"<<country->name()<<"data";
    KConfigGroup countryGroup = config.group(country->name());
    countryGroup.writeEntry("id",++countryNum);
    countryGroup.writeEntry("anchor-point",country->anchorPoint());
    countryGroup.writeEntry("central-point",country->centralPoint());
    countryGroup.writeEntry("flag-point",country->pointFlag());
    countryGroup.writeEntry("cannon-point",country->pointCannon());
    countryGroup.writeEntry("cavalry-point",country->pointCavalry());
    countryGroup.writeEntry("infantry-point",country->pointInfantry());
  }

  kDebug() << "Saving nationalities data";
  QStringList nationalitiesList;
  foreach (Nationality* nationality, m_nationalities)
  {
    nationalitiesList.push_back(nationality->name());
  }
  onugroup.writeEntry("nationalities", nationalitiesList);
  foreach (Nationality* nationality, m_nationalities)
  {
    kDebug() << "Saving nationality " << nationality->name();
    KConfigGroup nationalityGroup = config.group(nationality->name());
    nationalityGroup.writeEntry("leader",nationality->leaderName());
    nationalityGroup.writeEntry("flag",nationality->flagFileName());
  }


  kDebug() << "Saving continents data";
  QStringList continentsList;
  foreach (Continent* continent, m_continents)
  {
    continentsList.push_back(continent->name());
  }
  onugroup.writeEntry("continents", continentsList);
  unsigned int continentNum = 0;
  foreach (Continent* continent, m_continents)
  {
    kDebug() << "Saving"<<continent->name()<<"data";
    KConfigGroup continentGroup = config.group(continent->name());

    continentGroup.writeEntry("id",++continentNum);
    continentGroup.writeEntry("bonus",continent->bonus());

    QList<int> countryIdList;
    foreach(Country* country, continent->members())
    {
      countryIdList.push_back(m_countries.indexOf(country));
    }
    continentGroup.writeEntry("continent-countries",countryIdList);
  }

  kDebug() << "Saving goals data";
  QStringList goalsList;
  int goalNum = 0;
  foreach (Goal* goal, m_goals)
  {
    QString name = QString("goal") + QString::number(++goalNum);
    KConfigGroup goalGroup = config.group(name);

    goalGroup.writeEntry("desc",goal->description());
    QList<int> continentsList;
    switch(goal->type())
    {
      case Goal::Countries:
        goalGroup.writeEntry("type","countries");
        goalGroup.writeEntry("nbCountries",goal->nbCountries());
        goalGroup.writeEntry("nbArmiesByCountry",goal->nbArmiesByCountry());
        break;
      case Goal::Continents:
        goalGroup.writeEntry("type","continents");
        foreach(unsigned int continent, goal->continents())
        {
          continentsList.push_back(continent);
        }
        goalGroup.writeEntry("continents",continentsList);
        break;
      case Goal::GoalPlayer:
        goalGroup.writeEntry("type","player");
        goalGroup.writeEntry("nbCountriesFallback",goal->nbCountries());
        break;
      default:
        goalGroup.writeEntry("type","");
    }
    goalsList.push_back(name);
  }
  onugroup.writeEntry("goals", goalsList);
  
  foreach (Country* country, m_countries)
  {
    QList<int> theNeighboursIds;
    KConfigGroup countryGroup = config.group(country->name());
    foreach(Country* theNeighbour, country->neighbours())
    {
      theNeighboursIds.push_back(m_countries.indexOf(theNeighbour));
    }
    countryGroup.writeEntry("neighbours",theNeighboursIds);
  }

  kDebug() << "OUT";
}


/** This method returns a pointer to the country that contains the point (x,y).
If there is no country at (x,y), the functions returns 0. */
Country* ONU::countryAt(const QPointF& point)
{
//    kDebug() << "ONU::countryAt x y " << x << " " << y;
    QPointF norm = point;
    if ( norm.x() < 0 || norm.x() >= m_countriesMask.width()
      || norm.y() < 0 || norm.y() >= m_countriesMask.height() )
      return 0;

    int index = qBlue(m_countriesMask.pixel(norm.toPoint()));
//    kDebug() << "OUT ONU::countryAt: " << index;
    if (index >= m_countries.size()) return 0;
    return m_countries.at(index);
}

void ONU::reset()
{
  kDebug();
  foreach (Country* country, m_countries)
  {
    country->reset();
  }
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
  foreach (Country *c, m_countries)
  {
    if (c-> name() == name)
      return c;
  }
  return 0;
}

/** @return the number of countries in the world */
unsigned int ONU::getNbCountries() const
{
    return(m_countries.size());
}

/** Returns the nation named "name" ; 0 in case there is no such nation */
Nationality* ONU::nationNamed(const QString& name)
{
  foreach (Nationality *n, m_nationalities)
  {
    if (n->name() == name)
    {
      return n;
    }
  }
  return 0;
}

const Continent* ONU::continentWithId(const unsigned int id) const
{
  foreach (const Continent* c, m_continents)
  {
    if (c->id() == id)
    {
      return c;
    }
  }
  return 0;
}

Continent* ONU::continentNamed(const QString& name)
{
  foreach (Continent *c, m_continents)
  {
    if (c-> name() == name)
      return c;
  }
  return 0;
}

void ONU::buildMap()
{
  kDebug();
  //QSize size((int)(m_renderer.defaultSize().width()),(int)(m_renderer.defaultSize().height()));
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
  
  for (int i = 0; i < m_countries.size(); i++)
  {
    Country* country = m_countries[i];
    const QString& countryName = i18n(country->name().toUtf8().data());
    if (m_font.backgroundColor != "none")
    {
      painter.setPen(m_font.backgroundColor);
      painter.setFont(backgroundFont);
      QRect countryNameRect = painter.fontMetrics().boundingRect(countryName);
      painter.drawText(
        int((country->centralPoint().x()-countryNameRect.width()/2+1)),
        int((country->centralPoint().y()+countryNameRect.height()/2 + 1)),
        countryName);
    }
    painter.setPen(m_font.foregroundColor);
    painter.setFont(foregroundFont);
    QRect countryNameRect = painter.fontMetrics().boundingRect(countryName);
    painter.drawText(
    int((country->centralPoint().x()-countryNameRect.width()/2)),
    int((country->centralPoint().y()+countryNameRect.height()/2)),
        countryName);
  }

}

QPixmap ONU::pixmapForId(const QString& id, int width, int height)
{
  kDebug() << id;
  //QSize size((int)(m_renderer.defaultSize().width()),(int)(m_renderer.defaultSize().height()));
  QSize size(width,height);
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  image.fill(0);
  QPainter p(&image);
  m_renderer.render(&p, id);
  QPixmap pixmap = QPixmap::fromImage(image);
  return pixmap;
}

QSvgRenderer* ONU::renderer()
{
  return &m_renderer;
}

KGameSvgDocument* ONU::svgDom()
{
  return &m_svgDom;
}

QGraphicsPixmapItem* ONU::itemFor(const Country* country, SpriteType spriteType)
{
  if (country==0 || spriteType == None) return 0;
  foreach (QGraphicsPixmapItem* item, m_itemsMap.keys())
  {
    if (m_itemsMap[item].first == country && m_itemsMap[item].second == spriteType)
    {
      kDebug() << item << (void*)country << spriteType;
      return item;
    }
  }
  kDebug() << 0;
  return 0;
}

}
