/*
 * Copyright (C) 2009, 2010, 2011, 2012, 2013  by CNRS and Grenoble-INP
 *
 * This file is part of LIBTLMPWT
 *
 * LIBTLMPWT is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Authors: Matthieu Moy <matthieu.moy@imag.fr>, Grenoble-INP
 *          Tayeb Bouhadiba, CNRS
 *          Claude Helmstetter, CNRS
 *
 * This software has been partially supported by the French ANR project HELP
 *                                                         (ANR-09-SEGI-006)
 */

#include "temp_map.hpp"
#include "temp_monitor.hpp"
#include <qpainter.h>

using namespace std;

const unsigned TMAP_WIDTH = 420;
const unsigned TMAP_TEMP_MAX = 60;

TempMap::TempMap(TempMonitor *monitor):
  m_monitor(monitor)
{
  setFixedWidth(TMAP_WIDTH);
  AtmiWrapper::rectangle_t main_rect;
  m_monitor->m_wrapper->get_bounding_rect(main_rect);
  dist_ratio = TMAP_WIDTH / (main_rect.x2-main_rect.x1);
  setFixedHeight(dist_ratio * (main_rect.y2-main_rect.y1));

  nb_area = m_monitor->nb_area;
  
  AtmiWrapper::rectangle_t rect;
  rectangles = new QRect*[nb_area];
  for (unsigned i = 0, ei = nb_area; i<ei; ++i) {
    m_monitor->m_wrapper->get_rect(rect, i);
    int x1 = rect.x1*dist_ratio;
    int y1 = rect.y1*dist_ratio;
    int x2 = rect.x2*dist_ratio;
    int y2 = rect.y2*dist_ratio;
    rectangles[i] = new QRect(x1,y1,x2-x1,y2-y1);
  }
}

// From temperature to color
QColor color_of(double temp) {
  const double low_temp = PwtModule::INITIAL_TEMPERATURE + 5;
  if (temp<=low_temp) return QColor(0, 80, 140);
  if (temp>=TMAP_TEMP_MAX) return QColor(255, 0, 0);
  const int r = 255 * (temp-low_temp) /
    (TMAP_TEMP_MAX - low_temp);
  const int g = 80 * (TMAP_TEMP_MAX-temp) /
    (TMAP_TEMP_MAX - low_temp);
  const int b = 140 * (TMAP_TEMP_MAX-temp) /
    (TMAP_TEMP_MAX - low_temp);
  return QColor(r, g, b);
}

void TempMap::paintEvent(QPaintEvent*) {
  QPainter p(this);
  for (unsigned i = 0, ei = m_monitor->nb_area; i<ei; ++i) {
    const double t = m_monitor->tmp_temperatures[i];
    p.setBrush(color_of(t));
    p.drawRect(*rectangles[i]);
    p.drawText(*rectangles[i], Qt::AlignCenter, m_monitor->area_names[i]);
  }
}
