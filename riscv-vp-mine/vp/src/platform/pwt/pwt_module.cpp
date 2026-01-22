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

#include "pwt_module.hpp"
#include "atmi_wrapper.hpp"

using namespace std;
using namespace sc_core;

namespace {
  PwtModule *find_pwt_parent(sc_object *obj) {
    assert(obj!=NULL);
    sc_object *p = obj->get_parent();
    if (p==NULL) return NULL;
    PwtModule *m = dynamic_cast<PwtModule*>(p);
    if (m) return m;
    else return find_pwt_parent(p);
  }

  ParameterDouble capacitance("pwt-capacitance",
                              "average capacitance for 1 mm2 of silicon (for dynamic power)",
                              3.1e-9);
  ParameterDouble initacti("pwt-default-activity-ratio",
                           "average ratio of gates that are active during one cycle"
                           " (in [0,1], used for dynamic power)",
                           0.1);
  ParameterDouble leakage("pwt-leakage",
                          "average leakage intensity for 1 mm2 of silicion (for static power)",
                          2.5e-3);
  ParameterDouble leaktemp("pwt-leakage-temp",
                           "additionnal percentage of leakage for each additional degree",
                           5.0);
  ParameterDouble initfreq("pwt-frequency", "initial frequency (in MHz)", 50.0);
  ParameterDouble initvolt("pwt-voltage", "initial voltage (in Volt)", 3.0);
}

sc_time pwt_step = SC_ZERO_TIME;
double pwt_step_duration = 0.0;
sc_time pwt_step_begin = SC_ZERO_TIME;
sc_time pwt_step_end = SC_ZERO_TIME;

PwtModule::PwtModule(sc_module *module):
  m_module(module),
  m_parent(find_pwt_parent(module)),
  m_to_be_traced(false), m_irq_to_be_traced(false),
  m_activity_ratio(initacti.get()), m_activity_date(sc_core::SC_ZERO_TIME),
  m_counters(1, 0), m_counter0(&m_counters[0]),
  m_has_coordinates(false),
  m_temp(INITIAL_TEMPERATURE), m_max_temp(INITIAL_TEMPERATURE),
  m_capacitance(capacitance.get()*1e6),
  m_leakage(leakage.get()*1e6),
  m_leaktemp(leaktemp.get()/100)
{
  if (m_parent) {
    m_parent->register_child(this);
    set_voltage(m_parent->get_voltage());
    set_frequency(m_parent->get_frequency());
  } else {
    set_voltage(initvolt.get());
    set_frequency(initfreq.get()*1e6);
  }
}

PwtModule::~PwtModule() {
  // std::cout << m_module->name() <<": max temp: " << m_max_temp <<std::endl;
}

void PwtModule::set_coordinates(double x1, double y1, double x2, double y2) {
  assert(x1!=x2 && y1!=y2 && "Area must not be zero");
  if (x1<x2) {m_x1 = x1; m_x2 = x2;}
  else {m_x1 = x2; m_x2 = x1;}
  if (y1<y2) {m_y1 = y1; m_y2 = y2;}
  else {m_y1 = y2; m_y2 = y1;}
  m_has_coordinates = true;
}

void PwtModule::set_voltage(double volt) {
  set_voltage_hook(volt);
  m_volt = volt;
  for (unsigned i = 0, ei = m_children.size(); i!=ei; ++i)
    m_children[i]->set_voltage(volt);
}

void PwtModule::set_frequency(unsigned hertz) {
  set_frequency_hook(hertz);
  m_freq = hertz;
  m_period = sc_time(1.0/(double)hertz, SC_SEC);
  for (unsigned i = 0, ei = m_children.size(); i!=ei; ++i)
    m_children[i]->set_frequency(hertz);
}

void PwtModule::set_voltage_hook(double volt) {}
void PwtModule::set_frequency_hook(unsigned hertz) {}
void PwtModule::set_temperature_hook(double temp) {}

float PwtModule::dynamic_power_density(float activity) const {
  const double V = get_voltage();
  return .5 * m_capacitance * V * V * activity;
}

float PwtModule::static_power_density() const {
  const double V = get_voltage();
  const double T = get_temperature() - INITIAL_TEMPERATURE;
  return V * m_leakage * (1 + m_leaktemp * T);
}

void PwtModule::set_activity(float ratio, const sc_time &date)
{
  assert(date>=m_activity_date && "time going backward");
  if (date>m_activity_date) {
    add_activity_long(m_activity_ratio, m_activity_date, date);
  }
  m_activity_date = date;
  m_activity_ratio = ratio;
}

void PwtModule::add_activity_long(float activity_per_cycle,
                                  const sc_time &from, const sc_time &to)
{
  assert(pwt_step_begin<=from && "'from' date in past");
  assert(from<to && "invalid period");
  const float activity_per_second = activity_per_cycle * get_frequency();
  const sc_time from_offset = from - pwt_step_begin;
  const sc_time to_offset = to - pwt_step_begin;
  const unsigned first = floor(from_offset/pwt_step);
  const unsigned last = floor(to_offset/pwt_step);
  if (last>=m_counters.size()) {
    m_counters.resize(last+1, 0);
    m_counter0 = &m_counters[0];
  }
  if (first==last) {
    m_counters[first] += activity_per_second * (to-from).to_seconds();
  } else {
    m_counters[first] += activity_per_second * ((first+1)*pwt_step-from_offset).to_seconds();
    const double x = activity_per_second * pwt_step_duration;
    for (unsigned i = first+1; i<last; ++i)
      m_counters[i] += x;
    m_counters[last] += activity_per_second * (to_offset-last*pwt_step).to_seconds();
  }
}

double PwtModule::compute_power_density() {
  if (pwt_step_end>m_activity_date) {
    add_activity_long(m_activity_ratio, m_activity_date, pwt_step_end);
    m_activity_date = pwt_step_end;
  }
  const float activity = *m_counter0 / pwt_step_duration;
  m_counters.pop_front(); if (m_counters.empty()) m_counters.push_back(0);
  m_counter0 = &m_counters[0];
  return static_power_density() + dynamic_power_density(activity);
}

const sc_time &pwt_time_stamp = sc_time_stamp();
PwtTargetModule::PwtTargetModule(sc_core::sc_module *m,
                                 float read_acti_base, float write_acti_base,
                                 float read_acti_per_word, float write_acti_per_word):
  PwtModule(m),
  READ_ACTIVITY_BASE(read_acti_base),
  WRITE_ACTIVITY_BASE(write_acti_base),
  READ_ACTIVITY_PER_WORD(read_acti_per_word),
  WRITE_ACTIVITY_PER_WORD(write_acti_per_word),
  m_delayed_activity(0.0), m_begin(sc_core::SC_ZERO_TIME)
{}

void PwtModule::trace_irq() {
  m_irq_dates.push_back(sc_time_stamp());
}

// Delayed activity --------

std::vector<PwtTargetModule*> DelayedActivityManager::targets =
  std::vector<PwtTargetModule*>();

void DelayedActivityManager::commit(const sc_core::sc_time &local_offset) {
  for (unsigned i = 0, ei = targets.size(); i!=ei; ++i)
    targets[i]->commit_delayed_activity(sc_time_stamp() + local_offset);
  targets.clear();
}

void PwtTargetModule::commit_delayed_activity(const sc_core::sc_time &end) {
  size_t nb_cycle = (end-m_begin) / m_period;
  if (nb_cycle==0) nb_cycle = 1;
  add_activity_long(m_delayed_activity / nb_cycle, m_begin, end);
  m_delayed_activity = 0.0;
}

void PwtTargetModule::add_delayed_side_effects(unsigned size,
                                               tlm::tlm_command command,
                                               const sc_core::sc_time &local_offset)
{
  if (m_delayed_activity == 0.0) {
    m_begin = sc_core::sc_time_stamp() + local_offset;
    DelayedActivityManager::register_target(this);
  } else {
    assert(sc_core::sc_time_stamp() + local_offset==m_begin && "missing commit?");
  }
  if (command==tlm::TLM_READ_COMMAND) {
    m_delayed_activity += READ_ACTIVITY_BASE + size*READ_ACTIVITY_PER_WORD;
  } else {
    m_delayed_activity += WRITE_ACTIVITY_BASE + size*WRITE_ACTIVITY_PER_WORD;
  }
}
