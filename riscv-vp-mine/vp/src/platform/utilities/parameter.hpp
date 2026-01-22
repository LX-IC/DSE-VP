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

#ifndef _PARAMETER_HPP_
#define _PARAMETER_HPP_

#include "printable.hpp"
#include <string>
#include <vector>
#include <iostream>

//--------------------------------------------------------------------

class ParameterBase: Printable {
public:
  static void parse(int argc, char *argv[]);
  static void load(const std::string &filename);

  static void dump(std::ostream &os = std::cout);

  ParameterBase(const std::string& name,
                const std::string& description);

  virtual void set_value(const std::string& value) = 0;

  virtual unsigned get_value_width() const = 0;
  virtual void print(std::ostream &os,
                     unsigned c1, unsigned c2) const = 0;

protected:
  static void set_value(const std::string& name,
                        const std::string& value);

  const std::string m_name;
  const std::string m_description;

  static std::vector<ParameterBase*>& all();
};

//--------------------------------------------------------------------

class ParameterInt: public ParameterBase {
public:
  ParameterInt(const std::string& name,
               const std::string& description,
               const int default_value);

  void set_value(const std::string& value);
  void print(std::ostream &os) const;

  unsigned get_value_width() const;
  void print(std::ostream &os, unsigned c1, unsigned c2) const;

  inline int get() const {return m_value;}

private:
  const int m_default_value;
  int m_value;
};

//--------------------------------------------------------------------

class ParameterString: public ParameterBase {
public:
  ParameterString(const std::string& name,
                  const std::string& description,
                  const std::string& default_value);

  void set_value(const std::string& value);
  void print(std::ostream &os) const;

  unsigned get_value_width() const;
  void print(std::ostream &os, unsigned c1, unsigned c2) const;

  inline std::string get() const {return m_value;}

private:
  const std::string m_default_value;
  std::string m_value;
};

//--------------------------------------------------------------------

class ParameterBool: public ParameterBase {
public:
  ParameterBool(const std::string& name,
                const std::string& description);

  void set_value(const std::string& value);
  void print(std::ostream &os) const;

  unsigned get_value_width() const;
  void print(std::ostream &os, unsigned c1, unsigned c2) const;

  inline bool get() const {return m_value;}

private:
  bool m_value;
};

//--------------------------------------------------------------------

class ParameterDouble: public ParameterBase {
public:
  ParameterDouble(const std::string& name,
                  const std::string& description,
                  const double default_value);

  void set_value(const std::string& value);
  void print(std::ostream &os) const;

  unsigned get_value_width() const;
  void print(std::ostream &os, unsigned c1, unsigned c2) const;

  inline double get() const {return m_value;}

private:
  const double m_default_value;
  double m_value;
};

#endif // _PARAMETER_HPP_
