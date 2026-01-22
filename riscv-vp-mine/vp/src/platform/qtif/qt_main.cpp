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

#include <systemc>
#include <qapplication.h>
#include <qthread.h>
#include <cassert>
#include <qevent.h>
#include <qtextcodec.h>

#include "qt_exiter.hpp"
#include "temp_monitor.hpp"

using namespace sc_core;
using namespace std;

struct SimulatorThread: QThread {
  SimulatorThread(sc_time simtime):
    m_simtime(simtime)
  {}

  virtual void run() {
    if (m_simtime!=SC_ZERO_TIME)
      sc_start(m_simtime);
    else
      sc_start();
    // QtExiter::exit(0);
  }

  sc_time m_simtime;
};

int qt_start(int argc, char *argv[], const sc_time &simtime,
             AtmiWrapper *atmi_wrapper)
{
  assert(atmi_wrapper!=NULL && "Nothing to attach to the GUI");

  QApplication qtapp(argc, argv);
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
  TempMonitor *monitor = new TempMonitor(atmi_wrapper);
  QObject::connect(&qtapp, SIGNAL(aboutToQuit()), monitor, SLOT(closing()));
  
  monitor->show();

  SimulatorThread sim_thread(simtime);
  sim_thread.start();

  const int status = qtapp.exec();

  // sc_stop(); // FIXME: protect with a lock / send async sc_event
  sim_thread.wait();    

  return status;
}
