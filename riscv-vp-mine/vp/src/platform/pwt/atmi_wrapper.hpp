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

#ifndef _ATMI_WRAPPER_HPP_
#define _ATMI_WRAPPER_HPP_

#include "platform/utilities/parameter.hpp"
#include "pwt_module.hpp"
#include <systemc>
#include <fstream>

// Forward declarations
struct AtmiWrapperInternalState;

// Interface implemented by the GUI
struct TempMonitorIf {
  virtual ~TempMonitorIf() {}
  virtual void recordTemp(const double *T, const double *D) = 0;
  virtual void recordIRQ(unsigned module_id, double date) = 0;
};

class AtmiWrapper: public sc_core::sc_module {
public:

  struct rectangle_t { // all units in meter
    double x1;
    double y1;
    double x2;
    double y2;
    void set(double x1, double y1, double x2, double y2);
  };

  SC_HAS_PROCESS(AtmiWrapper);
  AtmiWrapper(sc_core::sc_module_name module_name, unsigned nb_module);
  ~AtmiWrapper();

  void end_of_elaboration();

  void compute();

  void config_rectangle(PwtModule *pw_graph,
                        double x1, double y1, double x2, double y2); // cm

  // public methods used by Temperatuer Monitor
  unsigned count_areas() const;
  const char *get_area_name(unsigned id) const;
  double get_area(unsigned id) const;
  void set_monitor(TempMonitorIf *tm);
  void get_bounding_rect(rectangle_t &rect) const;
  void get_rect(rectangle_t &rect, unsigned id) const;
  bool should_be_traced(unsigned id) const;
  bool should_irq_be_traced(unsigned id) const;

protected:
  sc_core::sc_time m_step;
  
  const unsigned m_nb_module;
  std::vector<PwtModule*> m_modules;

  AtmiWrapperInternalState *m_atmi_state;

  TempMonitorIf *m_monitor;

  std::ofstream m_ofs;
};

#endif // _ATMI_WRAPPER_HPP_
