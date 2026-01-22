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

// Slow down simulation so that simulation speed does not exceed realtime * 1/K

#ifndef _SIM_BRAKE_HPP_
#define _SIM_BRAKE_HPP_

#include <systemc>
#include <ctime>

struct SimBrake: sc_core::sc_module {
  SC_HAS_PROCESS(SimBrake);
  SimBrake(sc_core::sc_module_name);
  void compute();
  void setK(double k);
private:
  double K;
  sc_core::sc_time period;
  time_t prev_time;
  sc_core::sc_event start;
};

#endif // _SIM_BRAKE_HPP_
