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

#include "atmi_wrapper.hpp"
#include "platform/utilities/parameter.hpp"
#include "platform/utilities/trace.hpp"
// #include "temp_sensor.hpp"

extern "C" {
#include "platform/atmi/include/atmi.h"
}

// Config of the physical chip and the environement
#define CELSIUS_ZONE 60
#define HEATSINK_WIDTH 0.05
#define HEATSINK_THERMAL_RESISTANCE 1.
#define COPPER_THICKNESS 0.003
#define SILICON_THICKNESS 0.0005
#define INTERFACE_THICKNESS 0.0001
#define INTERFACE_THERMAL_CONDUCTIVITY 3

using namespace std;
using namespace sc_core;

namespace {
  ParameterInt atmi_step("atmi-step", "Step of the ATMI solver, in us", 1000);
  ParameterString temp_file("temp-file", "File name to store the temperatures",
                            "temperature.data");
}

struct AtmiWrapperInternalState {
  // rectangle coordinates (xl,yl,xr,yr) in meter
  atmi_rect *rectangles;

  // simulator
  atmi_simulator ts;
  atmi_param param;
  bool initialized;

  // Instantaneous power vector
  double* simu_q;

  // Temperature sensors
  atmi_rectset sensors;
};

AtmiWrapper::AtmiWrapper(sc_core::sc_module_name, unsigned nb_module):
  // m_step(sc_time(atmi_step.get(), SC_US)),
  m_step(sc_time(100, SC_NS)),
  m_nb_module(nb_module),
  m_modules(),
  m_atmi_state(new AtmiWrapperInternalState()),
  m_monitor(NULL)
{
  // pwt_step = sc_time(atmi_step.get(), SC_US);
  pwt_step = sc_time(100, SC_NS);
  pwt_step_duration = pwt_step.to_seconds();
  pwt_step_end = pwt_step;
  
  m_modules.reserve(nb_module);

  SC_THREAD(compute);
  m_atmi_state->rectangles = new atmi_rect[nb_module];
  m_atmi_state->simu_q = new double[nb_module];

  atmi_fill_param(&m_atmi_state->param,
                  CELSIUS_ZONE,
                  HEATSINK_THERMAL_RESISTANCE,
                  HEATSINK_WIDTH,
                  COPPER_THICKNESS,
                  SILICON_THICKNESS,
                  INTERFACE_THICKNESS,
                  INTERFACE_THERMAL_CONDUCTIVITY);
  m_atmi_state->initialized = false;
  
  // setting sensors at each place
  atmi_rectset_init(&m_atmi_state->sensors);

  for (unsigned i = 0; i <nb_module; ++i)
    atmi_rectset_add(&m_atmi_state->sensors, i);

  m_ofs.open(temp_file.get().c_str());
  if (!m_ofs) {
    ERROR("failed to open \"" << temp_file.get() <<"\"");
  }
}

AtmiWrapper::~AtmiWrapper() {
  m_ofs.close();
  if (m_atmi_state->initialized) atmi_simulator_freemem(&m_atmi_state->ts);
  delete[] m_atmi_state->rectangles;
  delete[] m_atmi_state->simu_q;
  delete m_atmi_state;
}

void AtmiWrapper::config_rectangle(PwtModule *module,
                                   double x1, double y1, double x2, double y2)
{
  const unsigned id = m_modules.size();
  assert(id<m_nb_module && "more modules than declared in constructor");
  module->set_coordinates(x1*1e-2, y1*1e-2, x2*1e-2, y2*1e-2); // convert cm to m (meter)
  m_atmi_state->rectangles[id].x1 = module->m_x1; 
  m_atmi_state->rectangles[id].y1 = module->m_y1;
  m_atmi_state->rectangles[id].x2 = module->m_x2;
  m_atmi_state->rectangles[id].y2 = module->m_y2;
  m_modules.push_back(module);
}

void AtmiWrapper::end_of_elaboration() {
  assert(m_nb_module==m_modules.size()
         && "less rectangles than declared in constructor");
}

void AtmiWrapper::compute() {
  const unsigned N = m_nb_module;
  unsigned t_us = 0;
  const unsigned step_us = floor(pwt_step_duration*1e6);
  while (true) {
    wait(pwt_step);
    for (unsigned i = 0, ei = N; i!=ei; ++i) {
      m_atmi_state->simu_q[i] = m_modules[i]->compute_power_density();
    }
    if (!m_atmi_state->initialized) {
      double *q_init = new double[N];
      for (unsigned i = 0, ei = N; i!=ei; ++i) {
        q_init[i] = m_atmi_state->simu_q[i]/3;
      }
      // INFO(1, "Initialize ATMI");
      atmi_simulator_init(&m_atmi_state->ts,
                          "atmi_solver_file.atmi",
                          &m_atmi_state->param,
                          pwt_step_duration,
                          N, m_atmi_state->rectangles,
                          &m_atmi_state->sensors, q_init);
      m_atmi_state->initialized = true;
      delete[] q_init;
    }
    // INFO(1, "ATMI step start (ts.t = " << m_atmi_state->ts.t * 1000.0 << " ms )");
    // INFO(1, "power densities: " << string_of(m_atmi_state->simu_q, m_atmi_state->simu_q+N));
    atmi_simulator_step(&m_atmi_state->ts, m_atmi_state->simu_q);
    // INFO(1, "ATMI step done (ts.t = " << m_atmi_state->ts.t * 1000.0 << " ms )");
    // INFO(1, "temperatures: " << string_of(m_atmi_state->ts.temperature,
                                          // m_atmi_state->ts.temperature+N));
    if (m_monitor) {
      m_monitor->recordTemp(m_atmi_state->ts.temperature,
                            m_atmi_state->simu_q);
      // cerr << "move IRQ dates\n";
      for (unsigned n = 0, en = m_modules.size(); n!=en; ++n) {
        for (unsigned i = 0, ei = m_modules[n]->m_irq_dates.size(); i!=ei; ++i) {
          // cerr << "move IRQ date: " << m_modules[n]->m_irq_dates[i].to_seconds() << "\n";
          m_monitor->recordIRQ(n, m_modules[n]->m_irq_dates[i].to_seconds());
          // cerr << "move IRQ date: done\n";
        }
        m_modules[n]->m_irq_dates.clear();
      }
      // cerr << "move IRQ dates: done\n";
    }
    for (unsigned i = 0, ei = N; i!=ei; ++i) {
      m_modules[i]->set_temperature(m_atmi_state->ts.temperature[i]);
    }
    t_us += step_us;
    m_ofs << t_us << '\t'
          << string_of(m_atmi_state->ts.temperature, m_atmi_state->ts.temperature+N, "\t")
          << '\n';
    pwt_step_begin = pwt_step_end; pwt_step_end += pwt_step;
  }
}

unsigned AtmiWrapper::count_areas() const {
  return m_nb_module;
}

const char *AtmiWrapper::get_area_name(unsigned id) const {
  assert(id<m_modules.size() && "Out of bound access");
  return m_modules[id]->m_module->name();
}

double AtmiWrapper::get_area(unsigned id) const {
  assert(id<m_modules.size() && "Out of bound access");
  atmi_rect &r = m_atmi_state->rectangles[id];
  return (r.x2-r.x1) * (r.y2-r.y1);
}

void AtmiWrapper::set_monitor(TempMonitorIf *tm) {
  m_monitor = tm;
}

void AtmiWrapper::rectangle_t::set(double x1_, double y1_, double x2_, double y2_) {
  assert(x2_>x1_ && y2_>y1_ && "please swap values");
  x1 = x1_; y1 = y1_; x2 = x2_; y2 = y2_;
}

AtmiWrapper::rectangle_t& assign(AtmiWrapper::rectangle_t &dst,
                                 const atmi_rect &src)
{
  dst.x1 = src.x1; dst.y1 = src.y1;
  dst.x2 = src.x2; dst.y2 = src.y2;
  return dst;
}

void AtmiWrapper::get_bounding_rect(rectangle_t &rect) const {
  const unsigned N = m_nb_module;
  if (N==0) {rect.set(0, 0, 1e-3, 1e-3); return;}
  assign(rect, m_atmi_state->rectangles[0]);
  for (unsigned i = 1; i<N; ++i) {
    if (m_atmi_state->rectangles[i].x1<rect.x1)
      rect.x1 = m_atmi_state->rectangles[i].x1;
    if (m_atmi_state->rectangles[i].y1<rect.y1)
      rect.y1 = m_atmi_state->rectangles[i].y1;
    if (m_atmi_state->rectangles[i].x2>rect.x2)
      rect.x2 = m_atmi_state->rectangles[i].x2;
    if (m_atmi_state->rectangles[i].y2>rect.y2)
      rect.y2 = m_atmi_state->rectangles[i].y2;
  }
}

void AtmiWrapper::get_rect(rectangle_t &rect, unsigned id) const {
  assert(id<m_nb_module && "out of bound");
  assign(rect, m_atmi_state->rectangles[id]);
}

bool AtmiWrapper::should_be_traced(unsigned id) const {
  assert(id<m_nb_module && "out of bound");
  return m_modules[id]->should_be_traced();
}

bool AtmiWrapper::should_irq_be_traced(unsigned id) const {
  assert(id<m_nb_module && "out of bound");
  return m_modules[id]->should_irq_be_traced();
}
