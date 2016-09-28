/*
    Copyright 2008 Edouard Griffiths, F4EXB.


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    See gpl.txt for the details of the licensing terms.

 */

#ifndef LOCATOR_H_
#define LOCATOR_H_

#include <string>

class LocatorInvalidException
{
  public:
    LocatorInvalidException(std::string locator_str) : _locator_str(locator_str) {};
    std::string getString();
  protected:
    std::string _locator_str;
};

class Locator
{
  public:
    Locator();
    Locator(std::string locator_str);
    Locator(float lat, float lon);
    std::string toString() const;
    void toCSting(char *locator) const;
    float latitude() const { return m_lat; };
    float longitude() const { return m_lon; };
    void setLatLon(float lat, float lon);

  protected:
    void setIndexes();

    int m_lat_index1;
    int m_lat_index2;
    int m_lat_index3;
    int m_lon_index1;
    int m_lon_index2;
    int m_lon_index3;
    static const std::string m_lon_array1;
    static const std::string m_lat_array1;
    static const std::string m_lon_array2;
    static const std::string m_lat_array2;
    static const std::string m_lon_array3;
    static const std::string m_lat_array3;
    float m_lat;
    float m_lon;
};

class LocPoint
{
  public:
    LocPoint() {};
    LocPoint(Locator& locator) : m_locator(locator) {};
    LocPoint(float lat, float lon) : m_locator(lat, lon) {};
    float latitude() const { return m_locator.latitude(); }
    float longitude() const { return m_locator.longitude(); }
    float bearingTo(const LocPoint& distant_point);
    float distanceTo(const LocPoint& distant_point);
    void setLatLon(float lat, float lon) { m_locator.setLatLon(lat, lon); }
    const Locator& getLocator() { return m_locator; }
  protected:
    Locator m_locator;
};

#endif // define LOCATOR_H_
