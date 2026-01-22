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

#include "sim_brake.hpp"
#include <qthread.h>

using namespace std;
using namespace sc_core;

class Sleeper : public QThread {
public:
  static void usleep(unsigned long us) {QThread::usleep(us);}
  static void msleep(unsigned long ms) {QThread::msleep(ms);}
  static void sleep(unsigned long s) {QThread::sleep(s);}
};

SimBrake::SimBrake(sc_core::sc_module_name):
  K(0.0),
  period(8, SC_MS)
{
  SC_THREAD(compute);
}

void SimBrake::setK(double k) {
  K = k;
  if (K>=0.1) start.notify();
}

void SimBrake::compute() {
  time_t a, b;
  while (true) {
    while (K<0.1) sc_core::wait(start);
    time(&a);
    sc_core::wait(period);
    time(&b);
    double delay = K * period.to_seconds() - difftime(b,a) - 0.001;
    if (delay>0.001) {
      unsigned long ms_delay = delay*1000;
      Sleeper::msleep(ms_delay);
    }
    a = b;
  }
}
