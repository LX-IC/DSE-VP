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

#include "power_ctrl.hpp"
#include "power_ctrl_regmap.h"
#include "trace.hpp"

using namespace sc_core;
using namespace std;

namespace {
  const unsigned MILLIVOLT_MAIN = 3000;
  const unsigned MILLIVOLT_LOW  = 1600;

  const unsigned FREQUENCE_MAIN = 50000000;
  const unsigned FREQUENCE_LOW  = 25000000;
}

const unsigned PowerCtrl::voltages[4] = {0, MILLIVOLT_MAIN, MILLIVOLT_LOW, 0};
const unsigned PowerCtrl::frequences[4] = {0, FREQUENCE_MAIN, FREQUENCE_LOW, 0};

PowerCtrl::PowerCtrl(sc_module_name name, unsigned nb_module):
  PwtModule(this),
  m_state_reg(0),
  m_controlled_modules(nb_module, NULL)
{
  assert(nb_module<=8 && "can control only 8 modules");
}

void PowerCtrl::set_module(unsigned id, PwtModule *controlled_module,
                           unsigned v_id, unsigned f_id) {
  assert(id<m_controlled_modules.size() && "out of bound access");
  assert(m_controlled_modules[id]==NULL && "module already set");
  m_controlled_modules[id] = controlled_module;
  m_state_reg |= ((v_id<<2)|f_id) << (4*id);
}

tlm::tlm_response_status
PowerCtrl::read(uint32_t addr, uint32_t& data)
{
  switch (addr) {
  case POWER_CTRL_REG_OFFSET:
    data = m_state_reg;
    break;
  default:
    WARNING("no register at offset " << hex << addr <<dec);
    return tlm::TLM_ADDRESS_ERROR_RESPONSE;
  }
  return tlm::TLM_OK_RESPONSE;
}

tlm::tlm_response_status
PowerCtrl::write(uint32_t addr, uint32_t data)
{
  switch (addr) {
  case POWER_CTRL_REG_OFFSET:
    for (unsigned i = 0; i<4; ++i) {
      const unsigned cmd = (data >> (i*8)) & 0xff;
      const bool valid = ! (cmd >> 7);
      if (valid) {
        const unsigned id = (cmd>>4) & 0x7;
        const unsigned v_id = (cmd>>2) & 0x3;
        const unsigned f_id = cmd & 0x3;
        assert(id<m_controlled_modules.size() && m_controlled_modules[id]!=NULL &&
               "invalid id");
        m_controlled_modules[id]->set_frequency(frequences[f_id]);
        m_controlled_modules[id]->set_voltage((double)voltages[v_id] * 1e-3);
        m_state_reg &=~ (0xf << (4*id));
        m_state_reg |= ((v_id<<2)|f_id) << (4*id);
      }
    }
    break;
  case POWER_CTRL_REG_TOPFREQ:
    assert(data<=FREQUENCE_MAIN && "Invalid frequency (too high)");
    assert(data>=1000 && "Invalid frequency (too low)");
    m_controlled_modules[0]->set_frequency(data);
    break;
  default:
    WARNING("no register at offset " << hex << addr <<dec);
    return tlm::TLM_ADDRESS_ERROR_RESPONSE;
  }
  return tlm::TLM_OK_RESPONSE;
}
