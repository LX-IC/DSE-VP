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

#ifndef _PWT_MODULE_HPP_
#define _PWT_MODULE_HPP_

#include <tlm.h>
#include <cassert>
#include <deque>

// Forward declarations
class AtmiWrapper;

extern sc_core::sc_time pwt_step; // ATMI step duration as sc_time
extern double pwt_step_duration; // ATMI step duration expressed in seconds
extern sc_core::sc_time pwt_step_begin;
extern sc_core::sc_time pwt_step_end;

// Module with Power and Temperature
class PwtModule {
public:
  // parameters
  // static const double INITIAL_TEMPERATURE = 26.20; // Celsius degree
  static constexpr double INITIAL_TEMPERATURE = 26.20; // Celsius degree

  // constructor
  PwtModule(sc_core::sc_module *module);

  // destructor
  ~PwtModule();

  // public methods
  void set_coordinates(double x1, double y1, double x2, double y2); // meters
  inline bool has_coordinates() const {return m_has_coordinates;}

  inline double get_width() const;
  inline double get_height() const;
  inline double get_area() const;

  void set_voltage(double volt);
  void set_frequency(unsigned hertz);

  // hooks (can be redefined by user module)
  virtual void set_voltage_hook(double volt);
  virtual void set_frequency_hook(unsigned hertz);
  virtual void set_temperature_hook(double temp);

  inline double get_voltage() const {return m_volt;}
  inline unsigned get_frequency() const {return m_freq;}
  inline sc_core::sc_time get_period() const {return m_period;}

  inline double get_temperature() const;

  // Set power consumption
  void set_activity(float ratio, // ratio of gates that are active during each cycle (in [0,1])
                    const sc_core::sc_time &date = sc_core::sc_time_stamp());

  // Set energy consumption
  inline void add_activity(float ratio_increment,
                           unsigned nb_cycle,
                           const sc_core::sc_time &date = sc_core::sc_time_stamp());

  inline void enable_trace() {m_to_be_traced = true;}
  inline void enable_irq_trace() {m_irq_to_be_traced = true;}
  inline bool should_be_traced() const {return m_to_be_traced;}
  inline bool should_irq_be_traced() const {return m_irq_to_be_traced;}

protected:
  // protected methods
  friend class AtmiWrapper;
  float dynamic_power_density(float activity) const;
  float static_power_density() const;
  void add_activity_long(float activity,
                         const sc_core::sc_time &from, const sc_core::sc_time &to);
  double compute_power_density();
  inline void set_temperature(double temp);
  inline void register_child(PwtModule *m) {m_children.push_back(m);}

  void trace_irq();

  // members
  sc_core::sc_module *m_module;
  PwtModule *m_parent; // used to get voltage and frequency
  std::vector<PwtModule*> m_children;
  bool m_to_be_traced;
  bool m_irq_to_be_traced;

  double m_volt;
  unsigned m_freq;
  sc_core::sc_time m_period;

  float m_activity_ratio; // ratio of gates that are active during each cycle
  sc_core::sc_time m_activity_date;

  std::deque<float> m_counters;
  float *m_counter0;

  bool   m_has_coordinates;
  double m_x1, m_y1, m_x2, m_y2; // meters
  double m_temp;
  double m_max_temp;

  std::vector<sc_core::sc_time> m_irq_dates;

  // configuration (mainly techno dependent)
  double m_capacitance;
  double m_leakage;
  double m_leaktemp;
};

// subclass with side effect methods ---
class PwtTargetModule: public PwtModule {
public:
  PwtTargetModule(sc_core::sc_module *m,
                  float read_acti_base, float write_acti_base,
                  float read_acti_per_word, float write_acti_per_word);
    
  inline void apply_side_effects(unsigned size,
                                 tlm::tlm_command command,
                                 const sc_core::sc_time &local_offset);

  inline void apply_side_effects_at_0_read(unsigned size);
  inline void apply_side_effects_at_0_write(unsigned size);

  void add_delayed_side_effects(unsigned size,
                                tlm::tlm_command command,
                                const sc_core::sc_time &local_offset);
  void commit_delayed_activity(const sc_core::sc_time &end);

protected:
  const float READ_ACTIVITY_BASE;
  const float WRITE_ACTIVITY_BASE;
  const float READ_ACTIVITY_PER_WORD;
  const float WRITE_ACTIVITY_PER_WORD;

  float m_delayed_activity;
  sc_core::sc_time m_begin;
};

// inline methods ----

double PwtModule::get_temperature() const {
  assert(m_has_coordinates && "modules without coordinates have no known temperature");
  return m_temp;
}

void PwtModule::set_temperature(double temp) {
  assert(m_has_coordinates==true);
  set_temperature_hook(temp+INITIAL_TEMPERATURE);
  m_temp = temp+INITIAL_TEMPERATURE;
  if (m_temp>m_max_temp) m_max_temp = m_temp;
}

double PwtModule::get_width() const {
  assert(m_has_coordinates && "illegal call");
  return m_x2-m_x1;
}
double PwtModule::get_height() const {
  assert(m_has_coordinates && "illegal call");
  return m_y2-m_y1;
}
double PwtModule::get_area() const {
  return get_width()*get_height();
}

void PwtModule::add_activity(float ratio_increment,
                             unsigned nb_cycle,
                             const sc_core::sc_time &date)
{
  if (nb_cycle<50 && date<=pwt_step_end) { // use fast mode
    *m_counter0 += ratio_increment * nb_cycle;
  } else { // dispatch on many steps (~ Tayeb traffic mechanism)
    add_activity_long(ratio_increment, date, date + nb_cycle*get_period());
  }
}

extern const sc_core::sc_time &pwt_time_stamp;

/******************************************************************************/

void PwtTargetModule::apply_side_effects(unsigned size,
                                         tlm::tlm_command command,
                                         const sc_core::sc_time &local_date)
{
  if (command==tlm::TLM_READ_COMMAND) {
    add_activity(READ_ACTIVITY_BASE + size*READ_ACTIVITY_PER_WORD,
                 1, local_date);
  } else {
    add_activity(WRITE_ACTIVITY_BASE + size*WRITE_ACTIVITY_PER_WORD,
                 1, local_date);
  }
}

void PwtTargetModule::apply_side_effects_at_0_read(unsigned size)
{
  *m_counter0 += READ_ACTIVITY_BASE + size*READ_ACTIVITY_PER_WORD;
}

void PwtTargetModule::apply_side_effects_at_0_write(unsigned size)
{
  *m_counter0 += WRITE_ACTIVITY_BASE + size*WRITE_ACTIVITY_PER_WORD;
}

class DelayedActivityManager {
  friend class PwtTargetModule;
  
  static std::vector<PwtTargetModule*> targets;
  static inline void register_target(PwtTargetModule *m) {targets.push_back(m);}

public:
  static void commit(const sc_core::sc_time &local_offset);
};

#endif // _PWT_MODULE_HPP_
