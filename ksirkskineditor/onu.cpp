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
#include "skinSpritesData.h"
#include "goal.h"
#include "../ksirk/KsirkGlobalDefinitions.h"

#include <QApplication>
#include <QFile>
// #include <QDom>
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QXmlStreamReader>

#include <KLocalizedString>
#include "ksirkskineditor_debug.h"
#include <KMessageBox>
#include <KConfig>
#include <KConfigGroup>
#include <QRegExp>

namespace KsirkSkinEditor
{

ONU::ONU(const QString& configDir, QObject *parent)
    : QObject(parent),
  m_configDir(configDir),
  m_configFileName(configDir + "/Data/world.desktop"),
  m_countries(),
  m_nationalities(),
  m_continents(),
  m_renderer(),
  m_itemsMap(),
  m_dirty(false)
{

  qCDebug(KSIRKSKINEDITOR_LOG) << "ONU constructor: " << m_configFileName;
  m_font.family = QStringLiteral("URW Chancery L");
  m_font.size = 13;
  m_font.weight = QFont::Bold;
  m_font.italic = true;
  m_font.foregroundColor = QStringLiteral("black");
  m_font.backgroundColor = QStringLiteral("white");
  
  SkinSpritesData::changeable().init();
//   unsigned int nationalityId = 0;
//   unsigned int continentId = 0;
  KConfig config(m_configFileName);

  KConfigGroup onugroup = config.group(QStringLiteral("onu"));

  qCDebug(KSIRKSKINEDITOR_LOG) << "ONU XML format version: " << onugroup.readEntry("format-version");
  QString formatVersion = onugroup.readEntry("format-version");

  if (!formatVersion.isEmpty() && formatVersion != ONU_FILE_FORMAT_VERSION)
  {
    KMessageBox::error(nullptr,
                        i18n("Invalid skin definition file format. Expected %1 and got %2",QStringLiteral(ONU_FILE_FORMAT_VERSION),formatVersion) + "<br>" + m_configFileName);
//     exit(1);
  }

  m_name = onugroup.readEntry("name");
  m_skin = onugroup.readEntry("skinpath");
  qCDebug(KSIRKSKINEDITOR_LOG) << "skin snapshot file: " << m_configDir + "/Images/snapshot.jpg";
  m_snapshot = QString(m_configDir + "/Images/snapshot.jpg");
  if (m_snapshot.isNull())
  {
    qCCritical(KSIRKSKINEDITOR_LOG) << "Was not able to load the snapshot image: " 
    << m_configDir + "/Images/snapshot.jpg";
  }
  m_width  = onugroup.readEntry("width",1024);
  m_height = onugroup.readEntry("height",768);
  m_description = onugroup.readEntry("desc");
//   countries.resize(onugroup.readEntry("nb-countries",0));
//   nationalities.resize(onugroup.readEntry("nb-nationalities",0));
//   m_continents.resize(onugroup.readEntry("nb-continents",0));
//    root.attribute("map");
  m_poolString = onugroup.readEntry("pool");
  if (m_poolString.isEmpty())
  {
    m_poolString = QStringLiteral("Images/pool.svg");
  }
  qCDebug(KSIRKSKINEDITOR_LOG) << "Pool path: " << m_poolString;
  qCDebug(KSIRKSKINEDITOR_LOG) << "Searching resource: " << (m_configDir + '/' + m_poolString);
  QString poolFileName = m_configDir + '/' + m_poolString;
  qCDebug(KSIRKSKINEDITOR_LOG) << "Pool file name: " << poolFileName;
  if (poolFileName.isEmpty())
  {
      KMessageBox::error(nullptr, 
                         i18n("Pool filename not found\nProgram cannot continue"));
      exit(2);
  }
  m_map = QPixmap();
  m_renderer.load(poolFileName);
  m_svgDom.load(poolFileName);
  loadPoolIds(poolFileName);
  
  QString mapMaskFileName = m_configDir + '/' + onugroup.readEntry("map-mask");
  qCDebug(KSIRKSKINEDITOR_LOG) << "Map mask file name init: " << mapMaskFileName;
  if (mapMaskFileName == (m_configDir + '/'))
  {
    mapMaskFileName = m_configDir + '/' + "Images/map-mask.png";
  }
  qCDebug(KSIRKSKINEDITOR_LOG) << "Map mask file name: " << mapMaskFileName;
  if (mapMaskFileName.isNull())
  {
      KMessageBox::error(nullptr, 
                         i18n("Map mask image not found\nProgram cannot continue"));
      exit(2);
  }
  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading map mask file: " << mapMaskFileName;
  m_countriesMask = QImage(mapMaskFileName);

  SkinSpritesData::changeable().intData(QStringLiteral("width-between-flag-and-fighter"), onugroup.readEntry("width-between-flag-and-fighter",0));


  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading flag data";
  SkinSpritesData::changeable().intData(QStringLiteral("flag-width"), config.group(QStringLiteral("flag")).readEntry("width",20));
  SkinSpritesData::changeable().intData(QStringLiteral("flag-height"), config.group(QStringLiteral("flag")).readEntry("height",20));
  SkinSpritesData::changeable().intData(QStringLiteral("flag-frames"), config.group(QStringLiteral("flag")).readEntry("frames",4));
  SkinSpritesData::changeable().intData(QStringLiteral("flag-versions"), config.group(QStringLiteral("flag")).readEntry("versions",1));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading infantry data";
//   SkinSpritesData::changeable().strData("infantry-id", config.group(QStringLiteral("infantry")).readEntry("id"));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry-width"), config.group(QStringLiteral("infantry")).readEntry("width",23));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry-height"), config.group(QStringLiteral("infantry")).readEntry("height",32));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry-frames"), config.group(QStringLiteral("infantry")).readEntry("frames",1));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry-versions"), config.group(QStringLiteral("infantry")).readEntry("versions",3));

//   SkinSpritesData::changeable().strData("infantry1-id", config.group(QStringLiteral("infantry1")).readEntry("id"));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry1-width"), config.group(QStringLiteral("infantry1")).readEntry("width",0));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry1-height"), config.group(QStringLiteral("infantry1")).readEntry("height",0));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry1-frames"), config.group(QStringLiteral("infantry1")).readEntry("frames",0));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry1-versions"), config.group(QStringLiteral("infantry1")).readEntry("versions",0));

//   SkinSpritesData::changeable().strData("infantry2-id", config.group(QStringLiteral("infantry2")).readEntry("id"));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry2-width"), config.group(QStringLiteral("infantry2")).readEntry("width",0));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry2-height"), config.group(QStringLiteral("infantry2")).readEntry("height",0));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry2-frames"), config.group(QStringLiteral("infantry2")).readEntry("frames",0));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry2-versions"), config.group(QStringLiteral("infantry2")).readEntry("versions",0));

//   SkinSpritesData::changeable().strData("infantry3-id", config.group(QStringLiteral("infantry3")).readEntry("id"));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry3-width"), config.group(QStringLiteral("infantry3")).readEntry("width",0));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry3-height"), config.group(QStringLiteral("infantry3")).readEntry("height",0));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry3-frames"), config.group(QStringLiteral("infantry3")).readEntry("frames",0));
  SkinSpritesData::changeable().intData(QStringLiteral("infantry3-versions"), config.group(QStringLiteral("infantry3")).readEntry("versions",0));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading cavalry data";
//   SkinSpritesData::changeable().strData("cavalry-id", config.group(QStringLiteral("cavalry")).readEntry("id"));
  SkinSpritesData::changeable().intData(QStringLiteral("cavalry-width"), config.group(QStringLiteral("cavalry")).readEntry("width",32));
  SkinSpritesData::changeable().intData(QStringLiteral("cavalry-height"), config.group(QStringLiteral("cavalry")).readEntry("height",32));
  SkinSpritesData::changeable().intData(QStringLiteral("cavalry-frames"), config.group(QStringLiteral("cavalry")).readEntry("frames",1));
  SkinSpritesData::changeable().intData(QStringLiteral("cavalry-versions"), config.group(QStringLiteral("cavalry")).readEntry("versions",3));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading cannon data";
//   SkinSpritesData::changeable().strData("cannon-id", config.group(QStringLiteral("cannon")).readEntry("id"));
  SkinSpritesData::changeable().intData(QStringLiteral("cannon-width"), config.group(QStringLiteral("cannon")).readEntry("width",32));
  SkinSpritesData::changeable().intData(QStringLiteral("cannon-height"), config.group(QStringLiteral("cannon")).readEntry("height",32));
  SkinSpritesData::changeable().intData(QStringLiteral("cannon-frames"), config.group(QStringLiteral("cannon")).readEntry("frames",2));
  SkinSpritesData::changeable().intData(QStringLiteral("cannon-versions"), config.group(QStringLiteral("cannon")).readEntry("versions",3));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading firing data";
//   SkinSpritesData::changeable().strData("firing-id", config.group(QStringLiteral("firing")).readEntry("id"));
  SkinSpritesData::changeable().intData(QStringLiteral("firing-width"), config.group(QStringLiteral("firing")).readEntry("width",64));
  SkinSpritesData::changeable().intData(QStringLiteral("firing-height"), config.group(QStringLiteral("firing")).readEntry("height",32));
  SkinSpritesData::changeable().intData(QStringLiteral("firing-frames"), config.group(QStringLiteral("firing")).readEntry("frames",4));
  SkinSpritesData::changeable().intData(QStringLiteral("firing-versions"), config.group(QStringLiteral("firing")).readEntry("versions",3));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading exploding data";
//   SkinSpritesData::changeable().strData("exploding-id", config.group(QStringLiteral("exploding")).readEntry("id"));
  SkinSpritesData::changeable().intData(QStringLiteral("exploding-width"), config.group(QStringLiteral("exploding")).readEntry("width",32));
  SkinSpritesData::changeable().intData(QStringLiteral("exploding-height"), config.group(QStringLiteral("exploding")).readEntry("height",32));
  SkinSpritesData::changeable().intData(QStringLiteral("exploding-frames"), config.group(QStringLiteral("exploding")).readEntry("frames",4));
  SkinSpritesData::changeable().intData(QStringLiteral("exploding-versions"), config.group(QStringLiteral("exploding")).readEntry("versions",3));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading font data";
  KConfigGroup fontgroup = config.group(QStringLiteral("font"));
  m_font.family = fontgroup.readEntry("family","URW Chancery L");
  m_font.size = fontgroup.readEntry("size", 13);
  QString w = fontgroup.readEntry("weight", "bold");;
  if (w == QLatin1String("normal"))
  {
    m_font.weight = QFont::Normal;
  }
  else if (w == QLatin1String("light"))
  {
    m_font.weight = QFont::Light;
  }
  else if (w == QLatin1String("demibold"))
  {
    m_font.weight = QFont::DemiBold;
  }
  else if (w == QLatin1String("bold"))
  {
    m_font.weight = QFont::Bold;
  }
  else if (w == QLatin1String("black"))
  {
    m_font.weight = QFont::Black;
  }
  m_font.italic = fontgroup.readEntry("italic", true);
  m_font.foregroundColor = fontgroup.readEntry("foreground-color", "black");
  m_font.backgroundColor = fontgroup.readEntry("background-color", "white");

  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading countries data";
  QStringList countriesList = onugroup.readEntry("countries", QStringList());
/*  while (m_countries.size() < countriesList.size())
  {
    m_countries.push_back(0);
  }*/
  for (const QString &country: countriesList)
  { 
    qCDebug(KSIRKSKINEDITOR_LOG) << "Loading"<<country<<"data";
    KConfigGroup countryGroup = config.group(country);
//     unsigned int id = countryGroup.readEntry("id",0);
    QString name = country;
    QPointF anchorPoint = countryGroup.readEntry("anchor-point",QPointF());
    QPointF centralPoint = countryGroup.readEntry("central-point",QPointF());
    QPointF flagPoint = countryGroup.readEntry("flag-point",QPointF());
    QPointF cannonPoint = countryGroup.readEntry("cannon-point",QPointF());
    QPointF cavalryPoint = countryGroup.readEntry("cavalry-point",QPointF());
    QPointF infantryPoint = countryGroup.readEntry("infantry-point",QPointF());

    m_countries.push_back(new Country(name, anchorPoint, centralPoint,
        flagPoint, cannonPoint, cavalryPoint, infantryPoint));
  }
  
  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading nationalities data";
  QStringList nationalitiesList = onugroup.readEntry("nationalities", QStringList());
/*  while (m_nationalities.size() < nationalitiesList.size())
  {
    m_nationalities.push_back(0);
  }*/
  for (const QString &nationality: nationalitiesList)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Creating nationality " << nationality;
    KConfigGroup nationalityGroup = config.group(nationality);
    QString leader = nationalityGroup.readEntry("leader","");
    QString flag = nationalityGroup.readEntry("flag","");
//         qCDebug(KSIRKSKINEDITOR_LOG) << "Creating nationality " << name << " ; flag: " << flag;
    m_nationalities.push_back(new Nationality(nationality, flag, leader));
//     nationalityId++;
  }


  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading continents data";
  QStringList continentsList = onugroup.readEntry("continents", QStringList());
/*  while (m_continents.size() < continentsList.size())
  {
    m_continents.push_back(0);
  }*/
  for (const QString &continent: continentsList)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Loading"<<continent<<"data";
    KConfigGroup continentGroup = config.group(continent);

//     unsigned int id = continentGroup.readEntry("id",0);
    unsigned int bonus = continentGroup.readEntry("bonus",0);
    QList<QString> countryIdList = continentGroup.readEntry("continent-countries",QList<QString>());
//     int countryId;
    QList<Country*> continentList;
    for(const QString& countryId: countryIdList)
    {
      continentList.push_back(countryNamed(countryId));
    }
//       qCDebug(KSIRKSKINEDITOR_LOG) << "Creating continent " << name;
    m_continents.push_back(new Continent(continent, continentList, bonus));
  }

  qCDebug(KSIRKSKINEDITOR_LOG) << "Loading goals data";
  QStringList goalsList = onugroup.readEntry("goals", QStringList());
  for (const QString &_goal: goalsList)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "init goal " << _goal;
    KConfigGroup goalGroup = config.group(_goal);

    Goal* goal = new Goal();
    goal->setDescription(goalGroup.readEntry("desc",""));
    QString goalType = goalGroup.readEntry("type","");
    if (goalType == QLatin1String("countries"))
    {
      goal->setType(Goal::Countries);
      goal->setNbCountries(goalGroup.readEntry("nbCountries",0));
      goal->setNbArmiesByCountry(goalGroup.readEntry("nbArmiesByCountry",0));
      qCDebug(KSIRKSKINEDITOR_LOG) << "  nb countries: **********************************" << goal->nbCountries();
      qCDebug(KSIRKSKINEDITOR_LOG) << "  nbarmies countries: **********************************" << goal->nbArmiesByCountry();
    }
    else if (goalType == QLatin1String("continents") )
    {
      goal->setType(Goal::Continents);
      QList<QString> continentsList = goalGroup.readEntry("continents",QList<QString>());
      for(const QString& continentId: continentsList)
        goal->continents().push_back(continentId);
    }
    else if (goalType == QLatin1String("player") )
    {
      goal->setType(Goal::GoalPlayer);
      unsigned int nb = goalGroup.readEntry("nbCountriesFallback",0);
      goal->setNbCountries(nb);
    }
    m_goals.push_back(goal);
  }

  for (const QString &country: countriesList)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "building neighbours list of " << country;
    QList< Country* > theNeighbours;
    KConfigGroup countryGroup = config.group(country);
    QList<QString> theNeighboursIds = countryGroup.readEntry("neighbours",QList<QString>());
//     int neighbourId;
    for(const QString& neighbourId: theNeighboursIds)
    {

      theNeighbours.push_back(countryNamed(neighbourId));
    }
    countryNamed(country)-> neighbours(theNeighbours);
  }
  qCDebug(KSIRKSKINEDITOR_LOG) << "Building map";
  buildMap();
  qCDebug(KSIRKSKINEDITOR_LOG) << "Map built";
  
  qCDebug(KSIRKSKINEDITOR_LOG) << "Building flag icon";
  int flagWidth = SkinSpritesData::changeable().intData(QStringLiteral("flag-width"));
  int flagHeight = SkinSpritesData::changeable().intData(QStringLiteral("flag-height"));
  int flagFrames = SkinSpritesData::changeable().intData(QStringLiteral("flag-frames"));
  int flagVersions = SkinSpritesData::changeable().intData(QStringLiteral("flag-versions"));
  if (m_nationalities.empty())
  {
    m_flagIcon = QPixmap(flagWidth,flagHeight);
  }
  else
  {
    m_flagIcon = QPixmap(pixmapForId(m_nationalities[0]->name().toLower(),flagWidth*flagFrames,flagHeight*flagVersions).copy(0,0,flagWidth,flagHeight));
  }
  qCDebug(KSIRKSKINEDITOR_LOG) << "Building infantry icon";
  int infantryWidth = SkinSpritesData::changeable().intData(QStringLiteral("infantry-width"));
  int infantryHeight = SkinSpritesData::changeable().intData(QStringLiteral("infantry-height"));
  int infantryFrames = SkinSpritesData::changeable().intData(QStringLiteral("infantry-frames"));
  int infantryVersions = SkinSpritesData::changeable().intData(QStringLiteral("infantry-versions"));
  m_infantryIcon = QPixmap(
  pixmapForId(QStringLiteral("infantry"),infantryWidth*infantryFrames,infantryHeight*infantryVersions).copy(0,0,infantryWidth,infantryHeight));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Building cavalry icon";
  int cavalryWidth = SkinSpritesData::changeable().intData(QStringLiteral("cavalry-width"));
  int cavalryHeight = SkinSpritesData::changeable().intData(QStringLiteral("cavalry-height"));
  int cavalryFrames = SkinSpritesData::changeable().intData(QStringLiteral("cavalry-frames"));
  int cavalryVersions = SkinSpritesData::changeable().intData(QStringLiteral("cavalry-versions"));
  m_cavalryIcon = QPixmap(
  pixmapForId(QStringLiteral("cavalry"),cavalryWidth*cavalryFrames,cavalryHeight*cavalryVersions).copy(0,0,cavalryWidth,cavalryHeight));
  
  qCDebug(KSIRKSKINEDITOR_LOG) << "Building cannon icon";
  int cannonWidth = SkinSpritesData::changeable().intData(QStringLiteral("cannon-width"));
  int cannonHeight = SkinSpritesData::changeable().intData(QStringLiteral("cannon-height"));
  int cannonFrames = SkinSpritesData::changeable().intData(QStringLiteral("cannon-frames"));
  int cannonVersions = SkinSpritesData::changeable().intData(QStringLiteral("cannon-versions"));
  m_cannonIcon = QPixmap(
  pixmapForId(QStringLiteral("cannon"),cannonWidth*cannonFrames,cannonHeight*cannonVersions).copy(0,0,cannonWidth,cannonHeight));

  qCDebug(KSIRKSKINEDITOR_LOG) << "OUT";
}

ONU::~ONU()
{
  qDeleteAll(m_countries);
  qDeleteAll(m_nationalities);
  qDeleteAll(m_continents);
  qDeleteAll(m_goals);
}

void ONU::saveConfig(const QString& configFileName)
{
  if (!configFileName.isNull())
  {
    m_configFileName = configFileName;
  }
  qCDebug(KSIRKSKINEDITOR_LOG) << m_configFileName;

  KConfig config(m_configFileName);

  KConfigGroup onugroup = config.group(QStringLiteral("onu"));

  qCDebug(KSIRKSKINEDITOR_LOG) << "ONU XML format version: ";
  onugroup.writeEntry("format-version",ONU_FILE_FORMAT_VERSION);

  onugroup.writeEntry("name",m_name);
  onugroup.writeEntry("skinpath",m_skin);
  onugroup.writeEntry("width",m_width);
  onugroup.writeEntry("height",m_height);
  onugroup.writeEntry("desc",m_description);
  onugroup.writeEntry("pool",m_poolString);

  onugroup.writeEntry("width-between-flag-and-fighter",SkinSpritesData::changeable().intData(QStringLiteral("width-between-flag-and-fighter")));


  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving flag data";
  config.group(QStringLiteral("flag")).writeEntry("width",SkinSpritesData::changeable().intData(QStringLiteral("flag-width")));
  config.group(QStringLiteral("flag")).writeEntry("height",SkinSpritesData::changeable().intData(QStringLiteral("flag-height")));
  config.group(QStringLiteral("flag")).writeEntry("frames",SkinSpritesData::changeable().intData(QStringLiteral("flag-frames")));
  config.group(QStringLiteral("flag")).writeEntry("versions",SkinSpritesData::changeable().intData(QStringLiteral("flag-versions")));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving infantry data";
//   config.group(QStringLiteral("infantry")).writeEntry("id",SkinSpritesData::changeable().strData("infantry-id"));
  config.group(QStringLiteral("infantry")).writeEntry("width",SkinSpritesData::changeable().intData(QStringLiteral("infantry-width")));
  config.group(QStringLiteral("infantry")).writeEntry("height",SkinSpritesData::changeable().intData(QStringLiteral("infantry-height")));
  config.group(QStringLiteral("infantry")).writeEntry("frames",SkinSpritesData::changeable().intData(QStringLiteral("infantry-frames")));
  config.group(QStringLiteral("infantry")).writeEntry("versions",SkinSpritesData::changeable().intData(QStringLiteral("infantry-versions")));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving cavalry data";
//   config.group(QStringLiteral("cavalry")).writeEntry("id",SkinSpritesData::changeable().strData("cavalry-id"));
  config.group(QStringLiteral("cavalry")).writeEntry("width",SkinSpritesData::changeable().intData(QStringLiteral("cavalry-width")));
  config.group(QStringLiteral("cavalry")).writeEntry("height",SkinSpritesData::changeable().intData(QStringLiteral("cavalry-height")));
  config.group(QStringLiteral("cavalry")).writeEntry("frames",SkinSpritesData::changeable().intData(QStringLiteral("cavalry-frames")));
  config.group(QStringLiteral("cavalry")).writeEntry("versions",SkinSpritesData::changeable().intData(QStringLiteral("cavalry-versions")));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving cannon data";
//   config.group(QStringLiteral("cannon")).writeEntry("id",SkinSpritesData::changeable().strData("cannon-id"));
  config.group(QStringLiteral("cannon")).writeEntry("width",SkinSpritesData::changeable().intData(QStringLiteral("cannon-width")));
  config.group(QStringLiteral("cannon")).writeEntry("height",SkinSpritesData::changeable().intData(QStringLiteral("cannon-height")));
  config.group(QStringLiteral("cannon")).writeEntry("frames",SkinSpritesData::changeable().intData(QStringLiteral("cannon-frames")));
  config.group(QStringLiteral("cannon")).writeEntry("versions",SkinSpritesData::changeable().intData(QStringLiteral("cannon-versions")));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving firing data";
//   config.group(QStringLiteral("firing")).writeEntry("id",SkinSpritesData::changeable().strData("firing-id"));
  config.group(QStringLiteral("firing")).writeEntry("width",SkinSpritesData::changeable().intData(QStringLiteral("firing-width")));
  config.group(QStringLiteral("firing")).writeEntry("height",SkinSpritesData::changeable().intData(QStringLiteral("firing-height")));
  config.group(QStringLiteral("firing")).writeEntry("frames",SkinSpritesData::changeable().intData(QStringLiteral("firing-frames")));
  config.group(QStringLiteral("firing")).writeEntry("versions",SkinSpritesData::changeable().intData(QStringLiteral("firing-versions")));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving exploding data";
//   config.group(QStringLiteral("exploding")).writeEntry("id",SkinSpritesData::changeable().strData("exploding-id"));
  config.group(QStringLiteral("exploding")).writeEntry("width",SkinSpritesData::changeable().intData(QStringLiteral("exploding-width")));
  config.group(QStringLiteral("cannon")).writeEntry("height",SkinSpritesData::changeable().intData(QStringLiteral("exploding-height")));
  config.group(QStringLiteral("exploding")).writeEntry("frames",SkinSpritesData::changeable().intData(QStringLiteral("exploding-frames")));
  config.group(QStringLiteral("exploding")).writeEntry("versions",SkinSpritesData::changeable().intData(QStringLiteral("exploding-versions")));

  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving font data";
  KConfigGroup fontgroup = config.group(QStringLiteral("font"));
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

  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving countries data";
  QStringList countriesList;
  for (Country* country: m_countries)
  {
    countriesList.push_back(country->name());
  }
  onugroup.writeEntry("countries", countriesList);
  unsigned int countryNum = 0;
  for (Country* country: m_countries)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Saving"<<country->name()<<"data" << countryNum;
    KConfigGroup countryGroup = config.group(country->name());
//     countryGroup.writeEntry("id",countryNum);
    countryNum++;
    countryGroup.writeEntry("anchor-point",country->anchorPoint());
    countryGroup.writeEntry("central-point",country->centralPoint());
    countryGroup.writeEntry("flag-point",country->pointFlag());
    countryGroup.writeEntry("cannon-point",country->pointCannon());
    countryGroup.writeEntry("cavalry-point",country->pointCavalry());
    countryGroup.writeEntry("infantry-point",country->pointInfantry());
  }

  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving nationalities data";
  QStringList nationalitiesList;
  for (Nationality* nationality: m_nationalities)
  {
    nationalitiesList.push_back(nationality->name());
  }
  onugroup.writeEntry("nationalities", nationalitiesList);
  for (Nationality* nationality: m_nationalities)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Saving nationality " << nationality->name();
    KConfigGroup nationalityGroup = config.group(nationality->name());
    nationalityGroup.writeEntry("leader",nationality->leaderName());
    nationalityGroup.writeEntry("flag",nationality->flagFileName());
  }


  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving continents data";
  QStringList continentsList;
  for (Continent* continent: m_continents)
  {
    continentsList.push_back(continent->name());
  }
  onugroup.writeEntry("continents", continentsList);
//   unsigned int continentNum = 0;
  for (Continent* continent: m_continents)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Saving"<<continent->name()<<"data";
    KConfigGroup continentGroup = config.group(continent->name());

//     continentGroup.writeEntry("id",++continentNum);
    continentGroup.writeEntry("bonus",continent->bonus());

    QList<QString> countryIdList;
    for(Country* country: continent->members())
    {
      countryIdList.push_back(country->name());
    }
    continentGroup.writeEntry("continent-countries",countryIdList);
  }

  qCDebug(KSIRKSKINEDITOR_LOG) << "Saving goals data";
  QStringList goalsList;
  int goalNum = 0;
  for (Goal* goal: m_goals)
  {
    QString name = QStringLiteral("goal") + QString::number(++goalNum);
    KConfigGroup goalGroup = config.group(name);

    goalGroup.writeEntry("desc",goal->description());
    QList<QString> continentsList;
    switch(goal->type())
    {
      case Goal::Countries:
        goalGroup.writeEntry("type","countries");
        goalGroup.writeEntry("nbCountries",goal->nbCountries());
        goalGroup.writeEntry("nbArmiesByCountry",goal->nbArmiesByCountry());
        break;
      case Goal::Continents:
        goalGroup.writeEntry("type","continents");
        for(const QString& continent: goal->continents())
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
  
  for (Country* country: m_countries)
  {
    QList<QString> theNeighboursIds;
    KConfigGroup countryGroup = config.group(country->name());
    for(Country* theNeighbour: country->neighbours())
    {
      theNeighboursIds.push_back(theNeighbour->name());
    }
    countryGroup.writeEntry("neighbours",theNeighboursIds);
  }

  m_dirty = false;
  qCDebug(KSIRKSKINEDITOR_LOG) << "OUT";
}


/** This method returns a pointer to the country that contains the point (x,y).
If there is no country at (x,y), the functions returns 0. */
Country* ONU::countryAt(const QPointF& point)
{
//    qCDebug(KSIRKSKINEDITOR_LOG) << "ONU::countryAt x y " << x << " " << y;
    QPointF norm = point;
    if ( norm.x() < 0 || norm.x() >= m_countriesMask.width()
      || norm.y() < 0 || norm.y() >= m_countriesMask.height() )
      return nullptr;

    int index = qBlue(m_countriesMask.pixel(norm.toPoint()));
//    qCDebug(KSIRKSKINEDITOR_LOG) << "OUT ONU::countryAt: " << index;
    if (index >= m_countries.size()) return nullptr;
    return m_countries.at(index);
}

void ONU::reset()
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  for (Country* country: m_countries)
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
    return nullptr;
  }
  for (Country *c: m_countries)
  {
    if (c-> name() == name)
      return c;
  }
  return nullptr;
}

/** @return the number of countries in the world */
unsigned int ONU::getNbCountries() const
{
    return(m_countries.size());
}

/** Returns the nation named "name" ; 0 in case there is no such nation */
Nationality* ONU::nationNamed(const QString& name)
{
  for (Nationality *n: m_nationalities)
  {
    if (n->name() == name)
    {
      return n;
    }
  }
  return nullptr;
}

// const Continent* ONU::continentWithId(const unsigned int id) const
// {
//   for (const Continent* c: m_continents)
//   {
//     if (c->id() == id)
//     {
//       return c;
//     }
//   }
//   return 0;
// }

Continent* ONU::continentNamed(const QString& name)
{
  for (Continent *c: m_continents)
  {
    if (c-> name() == name)
      return c;
  }
  return nullptr;
}

void ONU::buildMap()
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  //QSize size((int)(m_renderer.defaultSize().width()),(int)(m_renderer.defaultSize().height()));
  const qreal dpr = qApp->devicePixelRatio();
  const QSize size((int)(m_width * dpr), (int)(m_height * dpr));
  m_map = QPixmap(size);
  m_map.fill(Qt::transparent);
  QPainter painter(&m_map);
  m_renderer.render(&painter, QStringLiteral("map"));

  QFont foregroundFont(m_font.family, m_font.size, m_font.weight, m_font.italic);
  QFont backgroundFont(m_font.family, m_font.size, QFont::Normal, m_font.italic);

  painter.scale(dpr, dpr);
  for (int i = 0; i < m_countries.size(); i++)
  {
    Country* country = m_countries[i];
    const QString& countryName = i18n(country->name().toUtf8().data());
    if (m_font.backgroundColor != QLatin1String("none"))
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
  m_map.setDevicePixelRatio(dpr);
}

QPixmap ONU::pixmapForId(const QString& id, int width, int height)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << id << width << height;
  //QSize size((int)(m_renderer.defaultSize().width()),(int)(m_renderer.defaultSize().height()));
  const qreal dpr = qApp->devicePixelRatio();
  const QSize size(width * dpr, height * dpr);
  QPixmap pixmap(size);
  pixmap.fill(Qt::transparent);
  QPainter p(&pixmap);
  m_renderer.render(&p, id);
  pixmap.setDevicePixelRatio(dpr);
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

QGraphicsItem* ONU::itemFor(const Country* country, SpriteType spriteType)
{
  if (country==nullptr || spriteType == None) return nullptr;
  for (QGraphicsItem* item: m_itemsMap.keys())
  {
    if (m_itemsMap[item].first == country && m_itemsMap[item].second == spriteType)
    {
//       qCDebug(KSIRKSKINEDITOR_LOG) << item << (void*)country << spriteType;
      return item;
    }
  }
  qCDebug(KSIRKSKINEDITOR_LOG) << 0;
  return nullptr;
}

QFont ONU::foregroundFont()
{
  QFont foregroundFont(m_font.family, m_font.size, m_font.weight, m_font.italic);
  return foregroundFont;
}

QFont ONU::backgroundFont()
{
  QFont backgroundFont(m_font.family, m_font.size, QFont::Normal, m_font.italic);
  return backgroundFont;
}

void ONU::setFont(const QFont& font)
{
  if (m_font.family == font.family()
      && m_font.size == font.pointSize()
      && m_font.weight == (QFont::Weight)font.weight()
      && m_font.italic == font.italic())
    return;
  m_font.family = font.family();
  m_font.size = font.pointSize();
  m_font.weight = (QFont::Weight)font.weight();
  m_font.italic = font.italic();
  m_dirty = true;
}

void ONU::setFontFgColor(const QColor& color)
{
  if (m_font.foregroundColor == color.name())
    return;
  m_font.foregroundColor = color.name();
  m_dirty = true;
}

void ONU::setFontBgColor(const QColor& color)
{
  if (m_font.backgroundColor == color.name())
    return;
  m_font.backgroundColor = color.name();
  m_dirty = true;
}


void ONU::createCountry(const QString& newCountryName)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  Country* newCountry = new Country(newCountryName, QPointF(), QPointF(), QPointF(), QPointF(), QPointF(), QPointF()/*, m_countries.size()*/);
  m_countries.push_back(newCountry);
  m_dirty = true;
}

void ONU::deleteCountry(Country* country)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << country->name();
  QList<QGraphicsItem*> itemsToRemove;
  for (QGraphicsItem* item: m_itemsMap.keys())
  {
    if (m_itemsMap[item].first == country)
    {
      itemsToRemove.push_back(item);
    }
  }
  for (QGraphicsItem* item: itemsToRemove)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "remove an item";
    item->hide();
    item->scene()->removeItem(item);
    m_itemsMap.remove(item);
    delete item;
  }
  qCDebug(KSIRKSKINEDITOR_LOG) << "remove and delete the country";
  KConfig config(m_configFileName);
  config.deleteGroup(country->name());

  m_countries.removeAll(country);
  delete country;
  m_dirty = true;
}

void ONU::createContinent(const QString& newContinentName)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  Continent* newContinent = new Continent(newContinentName, QList<Country*>(), 0);
  m_continents.push_back(newContinent);
  m_dirty = true;
}

void ONU::deleteContinent(Continent* continent)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << continent->name();
  qCDebug(KSIRKSKINEDITOR_LOG) << "remove and delete the continent";
  KConfig config(m_configFileName);
  config.deleteGroup(continent->name());
  
  m_continents.removeAll(continent);
  delete continent;
  m_dirty = true;
}

void ONU::updateIcon(SpriteType type)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << type;
  int flagWidth;
  int flagHeight;
  int flagFrames;
  int flagVersions;
  int infantryWidth;
  int infantryHeight;
  int infantryFrames;
  int infantryVersions;
  int cavalryWidth;
  int cavalryHeight;
  int cavalryFrames;
  int cavalryVersions;
  int cannonWidth;
  int cannonHeight;
  int cannonFrames;
  int cannonVersions;

  switch (type)
  {
    case Flag:
      flagWidth = SkinSpritesData::changeable().intData(QStringLiteral("flag-width"));
      flagHeight = SkinSpritesData::changeable().intData(QStringLiteral("flag-height"));
      flagFrames = SkinSpritesData::changeable().intData(QStringLiteral("flag-frames"));
      flagVersions = SkinSpritesData::changeable().intData(QStringLiteral("flag-versions"));
      if (m_nationalities.empty())
      {
        m_flagIcon = QPixmap(flagWidth,flagHeight);
      }
      else
      {
        m_flagIcon = QPixmap(pixmapForId(nationalities()[0]->name().toLower(),flagWidth*flagFrames,flagHeight*flagVersions).copy(0,0,flagWidth,flagHeight));
      }
      break;
    case Infantry:
      infantryWidth = SkinSpritesData::changeable().intData(QStringLiteral("infantry-width"));
      infantryHeight = SkinSpritesData::changeable().intData(QStringLiteral("infantry-height"));
      infantryFrames = SkinSpritesData::changeable().intData(QStringLiteral("infantry-frames"));
      infantryVersions = SkinSpritesData::changeable().intData(QStringLiteral("infantry-versions"));
      m_infantryIcon = QPixmap(
      pixmapForId(QStringLiteral("infantry"),infantryWidth*infantryFrames,infantryHeight*infantryVersions).copy(0,0,infantryWidth,infantryHeight));
      break;
    case Cavalry:
      cavalryWidth = SkinSpritesData::changeable().intData(QStringLiteral("cavalry-width"));
      cavalryHeight = SkinSpritesData::changeable().intData(QStringLiteral("cavalry-height"));
      cavalryFrames = SkinSpritesData::changeable().intData(QStringLiteral("cavalry-frames"));
      cavalryVersions = SkinSpritesData::changeable().intData(QStringLiteral("cavalry-versions"));
      m_cavalryIcon = QPixmap(
      pixmapForId(QStringLiteral("cavalry"),cavalryWidth*cavalryFrames,cavalryHeight*cavalryVersions).copy(0,0,cavalryWidth,cavalryHeight));
      break;
    case Cannon:
      cannonWidth = SkinSpritesData::changeable().intData(QStringLiteral("cannon-width"));
      cannonHeight = SkinSpritesData::changeable().intData(QStringLiteral("cannon-height"));
      cannonFrames = SkinSpritesData::changeable().intData(QStringLiteral("cannon-frames"));
      cannonVersions = SkinSpritesData::changeable().intData(QStringLiteral("cannon-versions"));
      m_cannonIcon = QPixmap(
      pixmapForId(QStringLiteral("cannon"),cannonWidth*cannonFrames,cannonHeight*cannonVersions).copy(0,0,cannonWidth,cannonHeight));
      break;
    default:;
  }
}

void ONU::createGoal()
{
  Goal* goal = new Goal();
  m_goals.push_back(goal);
  m_dirty = true;
}

void ONU::deleteGoal(int g)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << m_goals.size() << g;

  KConfig config(m_configFileName);
  QString groupName = QStringLiteral("goal")+QString::number(g+1);
  qCDebug(KSIRKSKINEDITOR_LOG) << "delete group" << groupName;
  config.deleteGroup(groupName);
  
  Goal* goal = m_goals.takeAt(g);
  delete goal;
  m_dirty = true;
}

Nationality* ONU::nationalityNamed(const QString& name)
{
  for (Nationality* nationality: m_nationalities)
  {
    if (nationality->name() == name)
    {
      return nationality;
    }
  }
  return nullptr;
}

void ONU::createNationality(const QString& newNationalityName)
{
  Nationality* nationality = new Nationality(newNationalityName, QLatin1String(""), QLatin1String(""));
  
  m_nationalities.push_back(nationality);
  m_dirty = true;
}

void ONU::deleteNationality(Nationality* nationality)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  
  KConfig config(m_configFileName);
  config.deleteGroup(nationality->name());
  
  m_nationalities.removeAll(nationality);
  delete nationality;
  m_dirty = true;
}

void ONU::loadPoolIds(const QString& fileName)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << fileName;
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text))
  {
    KMessageBox::error(nullptr,
                        i18n("Cannot read file %1:\n%2.",fileName,file.errorString()),
                        i18nc("@title:window", "PoolLoader"));
                        return;
  }
  
  QXmlStreamReader xml(&file);
  QRegExp reg("\\D+\\d+");
  while (!xml.atEnd())
  {
    QXmlStreamReader::TokenType type = xml.readNext();
    if (type == QXmlStreamReader::StartElement)
    {
      qCDebug(KSIRKSKINEDITOR_LOG) << xml.text().toString();
      QXmlStreamAttributes attributes = xml.attributes ();
      auto id = attributes.value(QLatin1String(""), QStringLiteral("id") );

      if (!id.isEmpty() && !reg.exactMatch(id.toString()))
      {
        m_poolIds.push_back(id.toString());
      }
    }
  }
  if (xml.hasError())
  {
    qCCritical(KSIRKSKINEDITOR_LOG) << "Error: " << xml.errorString();
    // do error handling
  }
  m_poolIds.sort();
}

}

#include "moc_onu.cpp"
