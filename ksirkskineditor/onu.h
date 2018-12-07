/***************************************************************************
                          onu.h  -  description
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001 by Gael de Chalendar
    email                : Gael.de.Chalendar@free.fr
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
#define KDE_NO_COMPAT

#ifndef KSIRKSKINEDITORONU_H
#define KSIRKSKINEDITORONU_H

#include "country.h"
#include "continent.h"
#include "nationality.h"
#include "spritetype.h"

#include <QPixmap>
#include <QFont>
#include <QSvgRenderer>
#include <QObject>
#include <QMap>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgamesvgdocument.h>

class QGraphicsItem;

namespace KsirkSkinEditor
{

class Country;
class Goal;
/**
  * Class ONU (Organisation des Nations Unies = UNO : United Nations 
  * Organization) is simply the list of the countries. The data definining 
  * each country is loaded from an XML configuration file located in the
  * current skin data directory
  */
class ONU: public QObject
{
  Q_OBJECT

public:

  /**
    * Constructor
    * @param configFileName The name of the XML file defining this world. Built
    * from the current skin dir and a default file name.
    */
  explicit ONU(const QString& configFileName, QObject *parent);

  /** Default destructor */
  virtual ~ONU();

  //{@
  /**
   * Accessors
   */
  inline const QString& skin() const {return m_skin;}
  inline const QString& name() const {return m_name;}
  inline void setName(const QString& n) { if (m_name != n) {m_name = n; m_dirty = true;} }
  inline const QString& description() const {return m_description;}
  inline void setDescription(const QString& d) { if (m_description != d) {m_description = d; m_dirty=true;} }
  inline const QString& configFileName() const {return m_configFileName;}
  inline const QPixmap& map() const {return m_map;}
  inline const QPixmap& snapshot() const {return m_snapshot;}
  inline unsigned int width() const {return m_width;}
  inline void setWidth(unsigned int w) { if (m_width != w) {m_width = w; m_dirty = true;} }
  inline unsigned int height() const {return m_height;}
  inline void setHeight(unsigned int h) { if (m_height != h) {m_height = h; m_dirty = true;} }
  inline const QStringList& poolIds() const {return m_poolIds;}
  inline bool dirty() const {return m_dirty;}
  inline void setDirty() {m_dirty = true;}
  //@}
  
  /**
    * This method returns a pointer to the country that contains the given 
    * point. If there is no country there, the functions returns 0.
    * @param point The point where to search for a country in the map mask
    * @return The country at the given point or 0 if there is no country there.
    */
  Country* countryAt(const QPointF& point);

  /**
    * Calls its reset method for each country
    */
  void reset();

  /**
    * Return the countries list
    */
  inline QList<Country*>& countries() {return m_countries;}
  
  /**
    * Returns the nationalities list
    */
  inline QList<Nationality*>& nationalities() {return m_nationalities;}

  //@{
  /** Read property of QList<Continent*> continents. */
  inline QList<Continent*>& continents() {return m_continents;}
  inline const QList<Continent*>& continents() const {return m_continents;}
  //@}

  inline QList<Goal*>& goals() {return m_goals;}

  /**
    * Retrieves the continent with the given id
    * @param id The id of the continent to retrieve
    * @return A pointer to the retrieved continent or 0 if there is no 
    * continent with the given id.
    */
  const Continent* continentWithId(const unsigned int id) const;
  
  /**
    * Returns the country named "name" ; 0 in case there is no such country.
    * @param name The name of the country to retrieve.
    * @return The country named name or 0 if there is no such country.
    */
  Country* countryNamed(const QString& name);

  /** 
    * Gets the number of countries in the world
    * @return The number of countries in the world 
    */
  unsigned int getNbCountries() const;

  /**
    * Saves a XML representation of the world for game saving purpose
    * @param xmlStream The stream to write on
    */
  void saveXml(std::ostream& xmlStream);
  
  /** 
    * Returns the nation named "name" ; 0 in case there is no such nation 
    * @param name The name of the nation to retrieve.
    * @return The nation named name or 0 if there is no such nation.
    */
  Nationality* nationNamed(const QString& name);
  
  /**
    * Returns the continent named "name" ; 0 in case there is no such continent 
    * @param name The name of the continent to retrieve.
    * @return The continent named name or 0 if there is no such continent.
    */
  Continent* continentNamed(const QString& name);

  Nationality* nationalityNamed(const QString& name);

  QSvgRenderer* renderer();

  inline const QImage& mask() const {return m_countriesMask;}

  KGameSvgDocument* svgDom();

  inline QMap<QGraphicsItem*, QPair<Country*, SpriteType> >& itemsMap() {return m_itemsMap;}

  QGraphicsItem* itemFor(const Country* country, SpriteType spriteType);

  QPixmap pixmapForId(const QString& id, int width, int height);

  inline const QPixmap& flagIcon() const {return m_flagIcon;}
  inline const QPixmap& infantryIcon() const {return m_infantryIcon;}
  inline const QPixmap& cavalryIcon() const {return m_cavalryIcon;}
  inline const QPixmap& cannonIcon() const {return m_cannonIcon;}

  void saveConfig(const QString& configFileName = QString());

  QFont foregroundFont();
  QFont backgroundFont();

  QColor foregroundColor() {return QColor(m_font.foregroundColor);}
  QColor backgroundColor() {return QColor(m_font.backgroundColor);}
  
  void setFont(const QFont& font);
  void setFontFgColor(const QColor& color);
  void setFontBgColor(const QColor& color);
  
  void createCountry(const QString& newCountryName);
  void deleteCountry(Country* country);
  
  void createContinent(const QString& newCountryName);
  void deleteContinent(Continent* country);

  void updateIcon(SpriteType type);

  void createGoal();
  void deleteGoal(int g);

  void createNationality(const QString& newNationalityName);
  void deleteNationality(Nationality* nationality);
  
  private:
  /**
    * All data that have to be stored about the font to display countries names
    * in this world's skin
    */
  struct FontDesc
  {
    QString family;
    uint size;
    QFont::Weight weight;
    bool italic;
    QString foregroundColor;
    QString backgroundColor;
  };

  /**
  * Build the map from it's stored image and the countries names
  */
  void buildMap();

  void loadPoolIds(const QString& fileName);
  
  QString m_configDir;

  /**
    * The name of the .desktop file containing the world's definition
    */
  QString m_configFileName;
  
  /**
    * The displayable name of the skin
    */
  QString m_name;
  
  /**
    * The displayable long description of the skin
    */
  QString m_description;
  
  /**
    * The map used by this skin, built at the proper size from its SVG source 
    * and decorated with countries names 
    */
  QPixmap m_map;
  
  /**
    * A snaphsot of a running game with this skin. Used at skin choice time.
    */
  QPixmap m_snapshot;
  
  //@{
  /**
    * The width and height of the map file (will be used as canvas size). These
    * measures do not take into account the zoom factor.
    */
  unsigned int m_width;
  unsigned int m_height;
  //@}

  /**
    * The list of countries
    */
  QList<Country*> m_countries;

  /**
    * The list of nationalities
    */
  QList<Nationality*> m_nationalities;

  /**
    * The continents of the world
    */
  QList<Continent*> m_continents;

  /**
    * This image stores the mask that defines the countries of the world.
    * The blue RGB component value of each pixel gives the index of the
    * country in the countries list.
    */
  QImage m_countriesMask;
  
  /**
    * The path to the skin ; relative to the ksirk data dir ; loaded from the 
    * XML file
    */
  QString m_skin;
  
  /** 
    * The description of the font used to draw countries names onto the map.
    */
  FontDesc m_font;

  /**
    * This SVG renderer stores the SVG file of the map, renders it at the
    * desired zoom factor and the result is used to build the map image.
    */
  QSvgRenderer m_renderer;

  KGameSvgDocument m_svgDom;
  
  QMap<QGraphicsItem*, QPair<Country*, SpriteType> > m_itemsMap;

  QPixmap m_flagIcon;
  QPixmap m_infantryIcon;
  QPixmap m_cavalryIcon;
  QPixmap m_cannonIcon;

  QString m_poolString;

  QList<Goal*> m_goals;

  QStringList m_poolIds;

  bool m_dirty;
};

}
#endif // ONU_H

