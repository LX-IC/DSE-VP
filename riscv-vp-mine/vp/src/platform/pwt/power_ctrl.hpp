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

#ifndef _POWER_CTRL_HPP_
#define _POWER_CTRL_HPP_

#include "basic.hpp"
#include "pwt_module.hpp"
#include <vector>

class PowerCtrl: public sc_core::sc_module, public PwtModule {
public:

  static const unsigned voltages[4];
  static const unsigned frequences[4];

  basic::target_socket<PowerCtrl> target;

  PowerCtrl(sc_core::sc_module_name name, unsigned nb_module);

  void set_module(unsigned id, PwtModule *controlled_module,
                  unsigned v_id, unsigned f_id);

  tlm::tlm_response_status
  read(uint32_t addr, uint32_t& data);
  
  tlm::tlm_response_status
  write(uint32_t addr, uint32_t data);

protected:
  uint32_t m_state_reg;
  std::vector<PwtModule*> m_controlled_modules;
};

#endif // _POWER_CTRL_HPP_
