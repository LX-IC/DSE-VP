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

#ifndef _PRINTABLE_HPP_
#define _PRINTABLE_HPP_

#include <ostream>
#include <sstream>

// Base interface for objects that can be printed
struct Printable {
  virtual void print(std::ostream &os) const = 0;
};
static inline std::ostream& operator<<(std::ostream &os,
                                       const Printable &x)
{
  x.print(os);
  return os;
}

// Generic to string conversion function
template <typename T>
std::string string_of(const T& x) {
  std::ostringstream os;
  os << x;
  return os.str();
}

template <class ForwardIterator>
std::string string_of(ForwardIterator begin, ForwardIterator end,
                      const std::string &sep = ", ") {
  if (begin==end) return "";
  std::ostringstream os;
  ForwardIterator it = begin;
  os << *it; ++it;
  while (it!=end) {
    os << sep << *it; ++it;
  }
  return os.str();
}

// Write n times the same character
static inline void fill(std::ostream &os, unsigned n, char c) {
  for (unsigned i = 0; i<n; ++i)
    os << c;
}
#endif // _PRINTABLE_HPP_
