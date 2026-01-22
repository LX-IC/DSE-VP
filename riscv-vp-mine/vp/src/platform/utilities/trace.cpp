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

#include "trace.hpp"

using namespace std;
using namespace tlm;

ParameterBool
trace_quiet("quiet", "Disable warnings");

ParameterInt
trace_level("info-level",
            "Set debug trace level (0: none, ..., 3: all)",
            0);

namespace {const char *name() {return "Traces";}}

ostream& operator<<(ostream &os, tlm_generic_payload &pl) {
  os << '<' << noshowbase << hex;
  os.fill('0');
  switch (pl.get_command()) {
  case TLM_READ_COMMAND: os << "READ"; break;
  case TLM_WRITE_COMMAND: os << "WRITE"; break;
  case TLM_IGNORE_COMMAND: os << "IGNORE"; break;
  }
  os <<" @ ";
  os.width(8);
  os <<pl.get_address() <<" [";
  if (pl.get_data_ptr()) {
    if (pl.get_streaming_width()!=4)
      TODO("manage other streaming width");
    const uint32_t *data =
      reinterpret_cast<const uint32_t*>(pl.get_data_ptr());
    const uint32_t *be =
      reinterpret_cast<const uint32_t*>(pl.get_byte_enable_ptr());
    const unsigned be_size =
      be ? pl.get_byte_enable_length()/4 : 1;
    const unsigned size = pl.get_data_length()/4;
    for (unsigned i = 0, bei = 0; i<size; ++i, ++bei) {
      if (be)
        if (bei>=be_size) bei = 0;
      if (i!=0) os <<", ";
      os.width(8);
      os << data[i];
      if (be) {
        os << " & ";
        os.width(8);
        os << be[bei];
      }
    }
  }
  os << "] -> " << pl.get_response_string();
  return os << '>' << dec << showbase;
}
