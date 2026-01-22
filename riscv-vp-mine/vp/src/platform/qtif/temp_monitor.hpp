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

#ifndef _TEMP_MONITOR_HPP_
#define _TEMP_MONITOR_HPP_

#include <qwidget.h>
#include <vector>
#include "atmi_wrapper.hpp"

class AtmiWrapper;
class QLabel;
class QCheckBox;
class QTimer;
class QMutex;
class QPushButton;
class QSpinBox;
class SimBrake;
class QwtPlot;
class QwtPlotCurve;
class TempMap;
class QTabWidget;
class QwtPlotZoomer;
class QwtPlotMarker;
class QSpacerItem;
class QRectF;

class TempMonitor: public QWidget, public TempMonitorIf
{
  Q_OBJECT
public:
  TempMonitor(AtmiWrapper *wrapper, QWidget *parent = NULL, const char *name = NULL);
  
  virtual void recordTemp(const double *T, const double *D); // must be thread-safe
  virtual void recordIRQ(unsigned module_id, double date); // must be thread-safe

public slots:
  void closing();

private slots:
  void togglePause();
  void setBrake(int);
  void updateCurves();
  void updateTempScale(const QRectF &);
  void updatePowerScale(const QRectF &);
  void updateDensityScale(const QRectF &);

private:
  friend class TempMap;

  unsigned nb_area;

  AtmiWrapper *m_wrapper;

  QLabel *curve_label;
  QLabel *irq_label;
  std::vector<QCheckBox *> label_curves;
  std::vector<QCheckBox *> label_irqs;
  std::vector<QLabel *> label_names;
  std::vector<QLabel *> label_values;
  std::vector<QLabel *> label_sum;
  QSpacerItem *label_spacer;
  QTimer *timer;

  QMutex *mutex;        // protect accesses to next member
  std::vector<double> dates; // reallocation must be protected by mutex
  std::vector<double> *temperatures; // reallocation must be protected by mutex
  std::vector<double> *powers_av; // reallocation must be protected by mutex
  std::vector<double> m_sum_power;
  std::vector<double> *powers; // reallocation must be protected by mutex
  std::vector<double> power_sum; // reallocation must be protected by mutex
  double sum_power;
  std::vector<double> *densities; // reallocation must be protected by mutex
  double *tmp_temperatures; // local copy of current temperatures
  double prev_date;

  TempMap *tmap;
  const char* *area_names;
  double *areas;

  bool paused;
  QPushButton *pause_button;
  QLabel *time_label;
  QLabel *Total_power;
  QPushButton *quit_button;

  SimBrake *brake;
  QLabel *brake_label;
  QSpinBox *brake_spinbox;

  QTabWidget *tabs;

  QwtPlot *TempPlot;
  std::vector<QwtPlotCurve*> temp_curves;
  QwtPlotZoomer *temp_zoomer;

  QwtPlot *PowerPlot;
  std::vector<QwtPlotCurve*> power_curves;
  QwtPlotZoomer *power_zoomer;

  QwtPlot *DensityPlot;
  std::vector<QwtPlotCurve*> density_curves;
  QwtPlotZoomer *density_zoomer;

  std::vector<QwtPlotMarker*> *irq_dates; // irq_dates[n] contains the dates (in seconds) when
                                         // the module 'n' has sent an IRQ.

  unsigned timer_count;

private slots:
  void updateTemp();
  void updateTab(int);
};

#endif // _TEMP_MONITOR_HPP_
