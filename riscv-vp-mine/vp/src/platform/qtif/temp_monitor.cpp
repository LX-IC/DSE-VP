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

#include "temp_monitor.hpp"
#include "temp_map.hpp"
#include "printable.hpp"
#include "parameter.hpp"
#include "sim_brake.hpp"
#include <qlabel.h>
#include <qtimer.h>
#include <qmutex.h>
#include <qspinbox.h>
#include <cassert>
#include <iomanip>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <QTabWidget>
#include <qwt_plot_marker.h>
#include <QtGui/QSpacerItem>

#if QWT_VERSION < 0x060000
#define setSamples(A,B,C) setData(A,B,C)
#endif

using namespace std;

QColor COLORS[] = {
  QColor(0, 0, 139),
  QColor(0, 139, 0),
  QColor(0, 180, 180),
  QColor(139, 0, 139),
  QColor(211, 125, 0),
  QColor(145, 44, 238),
  QColor(139, 125, 107),
  QColor(205, 0, 0),
  QColor(150, 177, 42)};
const unsigned NB_COLOR = sizeof(COLORS)/sizeof(QColor);

QColor BackGround = QColor(210,230,210);

//------------------------------------------------------------------------------
TempMonitor::TempMonitor(AtmiWrapper *wrapper,
                         QWidget *parent, const char *name):
  m_wrapper(wrapper),
  label_names(), label_values(),
  prev_date(-0.1),
  paused(false),
  timer_count(0)
{
  assert(m_wrapper && "nothing to connect to");

  nb_area = m_wrapper->count_areas();

  label_curves.resize(nb_area, NULL);
  label_irqs.resize(nb_area, NULL);
  label_names.resize(nb_area, NULL);
  label_values.resize(nb_area, NULL);
  label_sum.resize(nb_area, NULL);

  QPalette mp = palette();
  mp.setColor(QPalette::Window, BackGround);
  setPalette(mp);
  setWindowTitle("riscv PWT Lite");

  // Temperatures as text
  QGridLayout *grid = new QGridLayout();

  curve_label = new QLabel(this);
  curve_label->setText("curve");
  grid->addWidget(curve_label, 0, 0);
  irq_label = new QLabel(this);
  irq_label->setText("IRQs");
  grid->addWidget(irq_label, 0, 1);

  ostringstream os;
  os << fixed << setprecision(2) << PwtModule::INITIAL_TEMPERATURE <<"°C";
  ostringstream osw;
  osw << fixed << setprecision(6) << 0 <<"W";
    
  for (unsigned i = 0, ei = nb_area; i<ei; ++i) {
    // Checkboxes
    label_curves[i] = new QCheckBox(this);
    label_irqs[i] = new QCheckBox(this);
    if (m_wrapper->should_be_traced(i))
      label_curves[i]->toggle();
    if (m_wrapper->should_irq_be_traced(i))
      label_irqs[i]->toggle();
    connect(label_curves[i], SIGNAL(clicked()), SLOT(updateCurves()));
    connect(label_irqs[i], SIGNAL(clicked()), SLOT(updateCurves()));

    label_names[i] = new QLabel(this);
    QPalette p = label_names[i]->palette();
    p.setColor(QPalette::WindowText, COLORS[i%NB_COLOR]);
    label_names[i]->setPalette(p);
    string tmp = m_wrapper->get_area_name(i)+string(": ");
    label_names[i]->setText(tmp.c_str());
    label_values[i] = new QLabel(this);
    label_values[i]->setPalette(p);
    label_values[i]->setText(os.str().c_str());
    //平均功耗部分
    
    label_sum[i] = new QLabel(this);
    
    label_sum[i]->setPalette(p);
    
    label_sum[i]->setText(osw.str().c_str());

    

    // Add to grid
    //对文本部分的主要输出
    grid->addWidget(label_curves[i], i+1, 0, Qt::AlignHCenter);
    grid->addWidget(label_irqs[i], i+1, 1, Qt::AlignHCenter);
    grid->addWidget(label_names[i], i+1, 2);
    grid->addWidget(label_values[i], i+1, 6);
    grid->addWidget(label_sum[i], i+1, 5);
    
  }

  //总功耗的输出
  //总功耗
    Total_power = new QLabel(this);
    
    grid->addWidget(Total_power, (nb_area + 2), 2);


  label_spacer = new QSpacerItem(42, 42, QSizePolicy::Minimum, QSizePolicy::Expanding);
  grid->addItem(label_spacer, nb_area+1,0);

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(updateTemp()));
  timer->start(100);//100

  mutex = new QMutex(QMutex::Recursive);
  temperatures = new std::vector<double>[nb_area];
  powers = new std::vector<double>[nb_area];
  powers_av = new std::vector<double>[nb_area];
  m_sum_power.resize(nb_area);
  //power_sum.resize(nb_area);
  densities = new std::vector<double>[nb_area];
  tmp_temperatures = new double[nb_area];

  // Connect ATMI wrapper to this module
  m_wrapper->set_monitor(this);

  // Temperatures as floorplan
  tmap = new TempMap(this);
  area_names = new const char*[nb_area];
  areas = new double[nb_area];
  for (unsigned i = 0, ei = nb_area; i<ei; ++i) {
    area_names[i] = m_wrapper->get_area_name(i);
    areas[i] = m_wrapper->get_area(i);
  }

  // Pause, Time, etc
  pause_button = new QPushButton("&Pause", this);
  connect(pause_button, SIGNAL(clicked()), SLOT(togglePause()));
  time_label = new QLabel(this);

  quit_button = new QPushButton("&Quit", this);
  connect(quit_button, SIGNAL(clicked()), qApp, SLOT(quit()));

  brake = new SimBrake("SIM_BRAKE");
  brake_label = new QLabel(this); brake_label->setText("brake: ");
  brake_spinbox = new QSpinBox(this);
  brake_spinbox->setRange(0, 100);
  brake_spinbox->setValue(0);
  connect(brake_spinbox, SIGNAL(valueChanged(int)), SLOT(setBrake(int)));

  QGridLayout * topbar = new QGridLayout();
  topbar->addWidget(quit_button, 0, 0);
  topbar->addWidget(pause_button, 0, 1);
  topbar->addWidget(brake_label, 0, 2);
  topbar->addWidget(brake_spinbox, 0, 3);
  topbar->addWidget(time_label, 0, 5);
  topbar->setColumnStretch(4, 10);

  // Curves: temperature
  TempPlot = new QwtPlot(this);
  TempPlot->setCanvasBackground(QColor("white"));
  TempPlot->setAxisTitle(QwtPlot::xBottom, "SystemC time (seconds)");
  TempPlot->setAxisTitle(QwtPlot::yLeft, "Temperature (°C)");
  TempPlot->setAxisScale(QwtPlot::yLeft, 20.0, 60.0, 10.0);
  // add curves
  temp_curves.resize(nb_area, NULL);
  for (unsigned i = 0; i<nb_area; ++i) {
    temp_curves[i] = new QwtPlotCurve(area_names[i]);
    temp_curves[i]->attach(TempPlot);
    temp_curves[i]->setPen(QPen(COLORS[i%NB_COLOR]));
  }
  // Curves: power
  PowerPlot = new QwtPlot(this);
  PowerPlot->setCanvasBackground(QColor("white"));
  PowerPlot->setAxisTitle(QwtPlot::xBottom, "SystemC time (seconds)");
  PowerPlot->setAxisTitle(QwtPlot::yLeft, "Power (mw)");
  PowerPlot->setAxisScale(QwtPlot::yLeft, 0.0, 500.0, 100.0);
  // add curves
  power_curves.resize(nb_area, NULL);
  for (unsigned i = 0; i<nb_area; ++i) {
    power_curves[i] = new QwtPlotCurve(area_names[i]);
    power_curves[i]->attach(PowerPlot);
    power_curves[i]->setPen(QPen(COLORS[i%NB_COLOR]));
  }
  // Curves: density
  DensityPlot = new QwtPlot(this);
  DensityPlot->setCanvasBackground(QColor("white"));
  DensityPlot->setAxisTitle(QwtPlot::xBottom, "SystemC time (seconds)");
  DensityPlot->setAxisTitle(QwtPlot::yLeft, "Power Density (mw/mm2)");
  DensityPlot->setAxisScale(QwtPlot::yLeft, 0.0, 1.0, 0.2);
  // add curves
  density_curves.resize(nb_area, NULL);
  for (unsigned i = 0; i<nb_area; ++i) {
    density_curves[i] = new QwtPlotCurve(area_names[i]);
    density_curves[i]->attach(DensityPlot);
    density_curves[i]->setPen(QPen(COLORS[i%NB_COLOR]));
  }

  tabs = new QTabWidget();
  tabs->addTab(TempPlot, "temperature");
  tabs->addTab(PowerPlot, "power");
  tabs->addTab(DensityPlot, "power density");
  tabs->addTab(tmap, "floorplan");

  connect(tabs, SIGNAL(currentChanged(int)), SLOT(updateTab(int)));

  // Main layout GUI界面的主要函数
  QGridLayout *mainBox = new QGridLayout(this);
  //表头部分
  mainBox->addLayout(topbar, 0, 0, 1, 2);
  //文字部分
  mainBox->addLayout(grid, 1, 0);
  //曲线部分
  mainBox->addWidget(tabs, 1, 1);
  mainBox->setRowStretch(1, 8);
  mainBox->setColumnStretch(1, 8);

  TempPlot->replot();
  TempPlot->setAxisAutoScale(QwtPlot::yLeft);
  TempPlot->setAxisAutoScale(QwtPlot::xBottom);
  PowerPlot->replot();
  PowerPlot->setAxisAutoScale(QwtPlot::yLeft);
  PowerPlot->setAxisAutoScale(QwtPlot::xBottom);
  DensityPlot->replot();
  DensityPlot->setAxisAutoScale(QwtPlot::yLeft);
  DensityPlot->setAxisAutoScale(QwtPlot::xBottom);

  temp_zoomer = new QwtPlotZoomer(TempPlot->canvas(), "Tzoomer");
  connect(temp_zoomer, SIGNAL(zoomed(const QRectF &)),
          SLOT(updateTempScale(const QRectF &)));

  power_zoomer = new QwtPlotZoomer(PowerPlot->canvas(), "Pzoomer");
  connect(power_zoomer, SIGNAL(zoomed(const QRectF &)),
          SLOT(updatePowerScale(const QRectF &)));

  density_zoomer = new QwtPlotZoomer(DensityPlot->canvas(), "Pzoomer");
  connect(density_zoomer, SIGNAL(zoomed(const QRectF &)),
          SLOT(updateDensityScale(const QRectF &)));

  irq_dates = new vector<QwtPlotMarker*>[nb_area];

  pause_button->setFocus();
}

//------------------------------------------------------------------------------
void TempMonitor::closing() {
  if (paused)
    mutex->unlock();
}

//------------------------------------------------------------------------------
void TempMonitor::updateTempScale(const QRectF &rect) {
  if (prev_date<=0) return;
  if (rect.right()>prev_date) {
    TempPlot->setAxisAutoScale(QwtPlot::xBottom);
    TempPlot->replot();
  }
}

void TempMonitor::updatePowerScale(const QRectF &rect) {
  if (prev_date<=0) return;
  if (rect.right()>prev_date) {
    PowerPlot->setAxisAutoScale(QwtPlot::xBottom);
    PowerPlot->replot();
  }
}

void TempMonitor::updateDensityScale(const QRectF &rect) {
  if (prev_date<=0) return;
  if (rect.right()>prev_date) {
    DensityPlot->setAxisAutoScale(QwtPlot::xBottom);
    DensityPlot->replot();
  }
}

//------------------------------------------------------------------------------
// Method called by Qt timer
void TempMonitor::updateTemp() {
  { QMutexLocker x(mutex);
    if (dates.empty()) return;
    const unsigned N = dates.size()-1;
    if (dates[N]==prev_date) return;
    for (unsigned i = 0; i<nb_area; ++i) {
      tmp_temperatures[i] = temperatures[i][N];
    }
    prev_date = dates[N];
  }
   
  for (unsigned i = 0, ei = nb_area; i<ei; ++i) {
    const double t = tmp_temperatures[i];
    ostringstream os; os << fixed << setprecision(2) << t <<"°C";
    label_values[i]->setText(os.str().c_str());
  }

  //新增部分
  for (unsigned i = 0, ei = nb_area; i<ei; ++i) {
    //dates.size随时间变化而变化,数据是实时更新的
    const double w = powers_av[i][dates.size()-1];
    ostringstream osw; osw << fixed << setprecision(3) << w*1e3 <<"mw";
    label_sum[i]->setText(osw.str().c_str());
  }
  

  ostringstream os;
  os << "SystemC time = " << fixed << setprecision(3) << prev_date <<"s";
  time_label->setText(os.str().c_str());

  ostringstream ost;
  ost << "Total power = " << fixed << setprecision(3) << power_sum[dates.size()-1]*1e3 <<"mw";
  Total_power->setText(ost.str().c_str());

  tmap->update();

  // update curves
  ++timer_count;
  if (timer_count==5) {
    updateCurves();
    timer_count = 0;
  }
}

//------------------------------------------------------------------------------
void TempMonitor::updateTab(int) {
  updateCurves();
}
//updata
void TempMonitor::updateCurves() {
  QMutexLocker x(mutex);
  for (unsigned i = 0, ei = nb_area; i<ei; ++i) {
    if (label_curves[i]->isChecked()) {
      temp_curves[i]->setSamples(&dates[0], &temperatures[i][0], dates.size());
      power_curves[i]->setSamples(&dates[0], &powers[i][0], dates.size());
      density_curves[i]->setSamples(&dates[0], &densities[i][0], dates.size());
    } else {
      temp_curves[i]->setSamples(NULL, NULL, 0);
      power_curves[i]->setSamples(NULL, NULL, 0);
      density_curves[i]->setSamples(NULL, NULL, 0);
    }
    QwtPlot *plot = NULL;
    if (label_irqs[i]->isChecked()) {
      switch (tabs->currentIndex()) {
      case 0: plot = TempPlot; break;
      case 1: plot = PowerPlot; break;
      case 2: plot = DensityPlot; break;
      default: break;
      }
    }
    //irq_dates[n]包含模块'n'发送IRQ的日期(以秒为单位)。
    for (unsigned j = 0, ej = irq_dates[i].size(); j!=ej; ++j)
      irq_dates[i][j]->attach(plot); // plot==NULL <==> detach
  }
  TempPlot->replot();
  PowerPlot->replot();
  DensityPlot->replot();
}

//------------------------------------------------------------------------------
// Pause manager
void TempMonitor::togglePause() {
  if (paused) {
    mutex->unlock();
    paused = false;
    pause_button->setText("&Pause");
  } else {
    mutex->lock();
    paused = true;
    pause_button->setText("&Run");
  }
}

//------------------------------------------------------------------------------
// Brake control
void TempMonitor::setBrake(int K) {
  brake->setK(K);
}

//------------------------------------------------------------------------------
// Method called by Atmi wrapper
//对温度，功耗，功耗密度数据的传递,应该在这个地方加一个总功耗的输出。
//
void TempMonitor::recordTemp(const double *T, const double *D) { // must be thread-safe!
  QMutexLocker x(mutex);
  dates.push_back(sc_core::sc_time_stamp().to_seconds());
  
  for (unsigned i = 0; i<nb_area; ++i) {
    temperatures[i].push_back(PwtModule::INITIAL_TEMPERATURE + T[i]);
    //在此处完成了功耗密度到功耗的转化
    // cout<<"i        = "<< i<<endl;
    // cout<<"D[i]     = "<< D[i]<<endl;
    // cout<<"areas[i] = "<< areas[i]<<endl;
    // powers[i].push_back(D[i]*areas[i]);
    //houjiade 1e3
    // if(i == 0)
    //   cout<<"D[i]               = "<< D[i]<<endl;
    powers[i].push_back(D[i]*areas[i]*1e3);
    m_sum_power[i] += (D[i]*areas[i]);
   
    if(powers[i].size() == 0 || m_sum_power[i]== 0.0)
    {
      powers_av[i].push_back(0.0);
    }      
    else
    {
      powers_av[i].push_back(m_sum_power[i]/(double)powers[i].size());  
    }
    densities[i].push_back(D[i]*1e-6);
    sum_power +=powers_av[i][powers_av[i].size()-1];
    // if(i==2)
    // {
    //   cout<<"powers[i].last            = "<< powers[i].back()<<endl;
    //   cout<<"m_sum_power[i]            = "<< m_sum_power[i]<<endl;
    //   cout<<"powers[i].size            = "<< powers[i].size()<<endl;
    //   cout<<"powers_av[i]              = "<< powers_av[i].back()<<endl;
    // }
      
  }
   power_sum.push_back(sum_power);
   sum_power  = 0;
}

// Method called by Atmi wrapper
void TempMonitor::recordIRQ(unsigned module_id, double date) { // must be thread-safe!
  QMutexLocker x(mutex);
  assert(module_id < nb_area && "invalid module ID");
  QwtPlotMarker *marker = new QwtPlotMarker();
  QPen pen;
  pen.setStyle(Qt::DashLine);
  pen.setBrush(COLORS[module_id%NB_COLOR]);
  marker->setLinePen(pen);
  marker->setLineStyle(QwtPlotMarker::VLine);
  marker->setXValue(date);
  irq_dates[module_id].push_back(marker);
}
