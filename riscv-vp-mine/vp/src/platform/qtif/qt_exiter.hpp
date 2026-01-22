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

// Small object allowing to call QApplication::exit(n) from a subthread.

#ifndef _QT_EXITER_HPP_
#define _QT_EXITER_HPP_

#include "qevent.h"
#include "qwidget.h"

class QtExiter: public QWidget
{
  Q_OBJECT
public:
  bool event(QEvent *event);

  static void exit(int retcode = 0);

private:
  QtExiter(int retcode, QWidget *parent = NULL, const char *name = NULL);
  int retcode;
};

#endif // _QT_EXITER_HPP_
