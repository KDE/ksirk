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
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/
#define KDE_NO_COMPAT

#ifndef ONU_H
#define ONU_H

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgamesvgdocument.h>

#include <qdatastream.h>
#include <QPixmap>
#include <QFont>
#include <QSvgRenderer>
#include <QTimer>
#include <QObject>
#include "country.h"
#include "continent.h"
#include "nationality.h"
#include "kgamewin.h"
#include "gameautomaton.h"
#include <QTextStream>

namespace Ksirk
{

namespace GameLogic
{

class Country;

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
  ONU(GameAutomaton* automaton, const QString& configFileName);

  /** Default destructor */
  virtual ~ONU();

  /** Zoom methods: */
    void applyZoomFactor(qreal zoomFactor);
    void applyZoomFactorFast(qreal zoomFactor);		//benj

  //{@
  /**
   * Accessors
   */
  const QString& skin() const {return m_skin;}
  const QString& name() const {return m_name;}
  const QString& description() const {return m_description;}
  const QString& mapFileName() const;
  const QString& getConfigFileName() const {return m_configFileName;}
  const QPixmap& map() const {return m_map;}
  const QPixmap& snapshot() const {return m_snapshot;}
  unsigned int width() const;
  unsigned int height() const;
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
  QList<Country*>& getCountries();
  
  /**
    * Returns the nationalities list
    */
  QList<Nationality*>& getNationalities();

  //@{
  /** Read property of QList<Continent*> continents. */
  QList<Continent*>& getContinents();
  const QList<Continent*>& getContinents() const;
  //@}

  /**
    * Retrieves the continent with the given id
    * @param id The id of the continent to retrieve
    * @return A pointer to the retrieved continent or 0 if there is no 
    * continent with the given id.
    */
//   const Continent* continentWithId(const unsigned int id) const;
  
  /** 
    * Returns the list of countries neighbours of the parameter country that 
    * belongs to the argument player.
    * @param country The country whose neighbours have to be tested
    * @param player The countries that belong to this player will be retrieved
    * @return A vector of pointers on countries neighbour to the given country 
    * and belonging to the given player.
    */
  QList<Country*> neighboursBelongingTo(const Country& country, const Player* player);

  /** Returns the list of countries neighbours of the parameter country that 
    * does not belong to the argument player.
    * @param country The country whose neighbours have to be tested
    * @param player The countries that do not belong to this player will be 
    * retrieved
    * @return A vector of pointers on countries neighbour to the given country 
    * and not belonging to the given player.
    */
    QList<Country*> neighboursNotBelongingTo(const Country& country, const Player* player);

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

  unsigned int indexOfCountry(Country*) const;
  
  /**
    * Saves a XML representation of the world for game saving purpose
    * @param xmlStream The stream to write on
    */
  void saveXml(QTextStream& xmlStream);
  
  /** 
    * Returns the nation named "name" ; 0 in case there is no such nation 
    * @param name The name of the nation to retrieve.
    * @return The nation named name or 0 if there is no such nation.
    */
  Nationality* nationNamed(const QString& name);
  
  /**
    * Transmit countries data to all network clients of the game. Called once
    * during finalization of network game start.
    * @param stream The stream to write countries data on
    */
  void sendCountries(QDataStream& stream);
  
  /** 
    * Returns the continent named "name" ; 0 in case there is no such continent 
    * @param name The name of the continent to retrieve.
    * @return The continent named name or 0 if there is no such continent.
    */
  Continent* continentNamed(const QString& name);

  double zoom() const;

  QSvgRenderer* renderer();

  KGameSvgDocument* svgDom();

  inline const QImage& mask() const {return countriesMask;}

  inline void setZoomArena(double newZoom) {m_zoomArena = newZoom;}

  inline GameAutomaton* automaton() {return m_automaton;}
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

  GameAutomaton* m_automaton;
  
  /**
    * The name of the XML file containing the world's definition
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
  QList<Country*> countries;

  /**
    * The list of nationalities
    */
  QList<Nationality*> nationalities;

  /**
    * The continents of the world
    */
  QList<Continent*> m_continents;

  /**
    * This image stores the mask that defines the countries of the world.
    * The blue RGB component value of each pixel gives the index of the
    * country in the countries list.
    */
  QImage countriesMask;
  
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
    * Zoom factor
    */
  double m_zoom;
  
  /** 
    * Zoom arena factor
    */
  double m_zoomArena;

  /**
    * Counter to know how many zooms have been asked by user 
    */
    int m_nbZooms;

  /**
    * Zoom used to change size of map
    */
    qreal m_zoomFactorFinal;

  /** 
    * Timer for the ZoomfactorFast
    */
    QTimer * m_timerFast;
    
  /**
    * This SVG renderer stores the SVG file of the map, renders it at the
    * desired zoom factor and the result is used to build the map image.
    */
//   QSvgRenderer m_renderer;

//   KGameSvgDocument m_svgDom;

  /**
   * Build the map from it's stored image and the countries names
   */
  void buildMap();

 /** SLOTS */
public Q_SLOTS:
  void changingZoom();

};

}
}
#endif // ONU_H

