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

#include <cctype> // for toupper
#include <algorithm>
#include <math.h>
#include "locator.h"

namespace DSDcc
{

// LocatorInvalidException class

std::string LocatorInvalidException::getString()
{ return _locator_str; }


// Locator class

std::string const Locator::m_lon_array1 = "ABCDEFGHIJKLMNOPQR";
std::string const Locator::m_lat_array1 = "ABCDEFGHIJKLMNOPQR";
std::string const Locator::m_lon_array2 = "0123456789";
std::string const Locator::m_lat_array2 = "0123456789";
std::string const Locator::m_lon_array3 = "ABCDEFGHIJKLMNOPQRSTUVWX";
std::string const Locator::m_lat_array3 = "ABCDEFGHIJKLMNOPQRSTUVWX";

Locator::Locator() : m_lat(0.0), m_lon(0.0)
{
    setIndexes();
}

Locator::Locator(std::string loc_string)
{
  // locator must be exactly 6 characters
  if (loc_string.length() != 6)
    throw LocatorInvalidException(loc_string);
  // convert to upper case
  std::transform(loc_string.begin(), loc_string.end(), loc_string.begin(), toupper);
  // retrieve latitude and longitude indexes
  if ((m_lon_index1 = m_lon_array1.find(loc_string[0])) == std::string::npos)
    throw LocatorInvalidException(loc_string);
  if ((m_lat_index1 = m_lat_array1.find(loc_string[1])) == std::string::npos)
    throw LocatorInvalidException(loc_string);
  if ((m_lon_index2 = m_lon_array2.find(loc_string[2])) == std::string::npos)
    throw LocatorInvalidException(loc_string);
  if ((m_lat_index2 = m_lat_array2.find(loc_string[3])) == std::string::npos)
    throw LocatorInvalidException(loc_string);
  if ((m_lon_index3 = m_lon_array3.find(loc_string[4])) == std::string::npos)
    throw LocatorInvalidException(loc_string);
  if ((m_lat_index3 = m_lat_array3.find(loc_string[5])) == std::string::npos)
    throw LocatorInvalidException(loc_string);
  // convert to decimal degrees for lower corner of locator square
  m_lat  = (m_lat_index1 * 10.0) - 90.0;
  m_lon  = (m_lon_index1 * 20.0) - 180.0;
  m_lat += m_lat_index2;
  m_lon += m_lon_index2 * 2.0;
  m_lat += m_lat_index3 / 24.0;
  m_lon += m_lon_index3 / 12.0;
  // adjust to center of locator square
  m_lat += 1.25 / 60.0;
  m_lon += 2.5 / 60.0;

}

Locator::Locator(float lat, float lon) : m_lat(lat), m_lon(lon)
{
    setIndexes();
}

void Locator::setLatLon(float lat, float lon)
{
    m_lat = lat;
    m_lon = lon;
    setIndexes();
}

void Locator::setIndexes()
{
    float lat_rem, lon_rem;

    m_lat_index1 = int((m_lat + 90.0)  / 10.0);
    m_lon_index1 = int((m_lon + 180.0) / 20.0);
    lat_rem = m_lat + 90.0 - (m_lat_index1 * 10.0);
    lon_rem = m_lon + 180.0 - (m_lon_index1 * 20.0);
    m_lat_index2 = int(lat_rem);
    m_lon_index2 = int(lon_rem / 2.0);
    lat_rem = lat_rem - m_lat_index2;
    lon_rem = lon_rem - (m_lon_index2 * 2);
    m_lat_index3 = int(lat_rem * 24.0);
    m_lon_index3 = int(lon_rem * 12.0);
}

std::string Locator::toString() const
{
  std::string returned = "";

  returned.append(1,m_lon_array1[m_lon_index1]);
  returned.append(1,m_lat_array1[m_lat_index1]);
  returned.append(1,m_lon_array2[m_lon_index2]);
  returned.append(1,m_lat_array2[m_lat_index2]);
  returned.append(1,m_lon_array3[m_lon_index3]);
  returned.append(1,m_lat_array3[m_lat_index3]);

  return returned;
}

void Locator::toCSting(char *locator) const
{
    locator[0] = m_lon_array1[m_lon_index1];
    locator[1] = m_lat_array1[m_lat_index1];
    locator[2] = m_lon_array2[m_lon_index2];
    locator[3] = m_lat_array2[m_lat_index2];
    locator[4] = m_lon_array3[m_lon_index3];
    locator[5] = m_lat_array3[m_lat_index3];
}

// LocPoint class

float LocPoint::bearingTo(const LocPoint& distant_point)
{
  double lat1 = m_locator.latitude() * (M_PI / 180.0);
  double lon1 = m_locator.longitude() * (M_PI / 180.0);
  double lat2 = distant_point.latitude() * (M_PI / 180.0);
  double lon2 = distant_point.longitude() * (M_PI / 180.0);
  double dLon = lon2 - lon1;
  double y = sin(dLon) * cos(lat2);
  double x = (cos(lat1)*sin(lat2)) -
             (sin(lat1)*cos(lat2)*cos(dLon));
  double bear_rad = atan2(y,x);
  if (bear_rad > 0)
    return bear_rad * (180.0 / M_PI);
  else
    return 360.0 + (bear_rad * (180.0 / M_PI));
}

float LocPoint::distanceTo(const LocPoint& distant_point)
{
  double lat1 = m_locator.latitude() * (M_PI / 180.0);
  double lon1 = m_locator.longitude() * (M_PI / 180.0);
  double lat2 = distant_point.latitude() * (M_PI / 180.0);
  double lon2 = distant_point.longitude() * (M_PI / 180.0);
  return acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon2-lon1))*6371.0;
}

} // namespace DSDcc
