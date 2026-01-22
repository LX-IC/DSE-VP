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

#ifndef _TRACE_HPP_
#define _TRACE_HPP_

#include "parameter.hpp"
#include <iostream>
#include <systemc>
#include <tlm.h>

extern ParameterBool trace_quiet;
extern ParameterInt trace_level;

#define USER_ERROR(msg) do {                                    \
    std::cerr << sc_core::sc_time_stamp() << '\t'               \
              << name() << ": ERROR: " << msg <<std::endl;      \
    exit(1);                                                    \
  } while (false)

#define ERROR(msg) do {                                         \
    std::cerr << sc_core::sc_time_stamp() << '\t'               \
              << name() << ": ERROR: " << msg <<std::endl;      \
    abort();                                                    \
  } while (false)

#define TODO(msg) do {                                          \
    std::cerr << sc_core::sc_time_stamp() << '\t'               \
              << name() << ": TODO: " << msg <<std::endl;       \
    assert(false);                                              \
  } while (false)

#define WARNING(msg) do {                                          \
    if (!trace_quiet.get())                                        \
      std::cerr << sc_core::sc_time_stamp() << '\t'                \
                << name() << ": Warning: " << msg <<std::endl;     \
  } while (false)

#define INFO(level, msg) do {                                      \
    if (trace_level.get()>=level)                                  \
      std::cout << sc_core::sc_time_stamp() << '\t'                \
                << name() << ": " << msg <<std::endl;              \
  } while (false)

std::ostream& operator<<(std::ostream &os,
                         tlm::tlm_generic_payload &pl);

#endif // _TRACE_HPP_
