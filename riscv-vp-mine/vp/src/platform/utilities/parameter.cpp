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

#include "parameter.hpp"
#include "trace.hpp"
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace std;

namespace {
  const char* name() {return "Config";}
}

// Base --------------------------------------------------------------

namespace {
  bool is_cfg_file(const string &str) {
    const size_t size = str.size();
    return size>=5 && 0==str.compare(size-4, 4, ".cfg");
  }

  char skip(istream &is) {
    char c = '\n';
    is >> c;
    while (!!is && (c==' ' || c=='\t')) is >> c;
    if (c=='#')
      do {is >> c;} while (!!is && c!='\n');
    if (c!='\n')
      is.putback(c);
    return c;
  }
}

void ParameterBase::parse(int argc, char *argv[]) {
  for (int i = 1; i<argc; ++i) {
    const char * arg = argv[i];
    if (arg[0]=='-') {
      while (arg[0]=='-') ++arg; // discard "--"
      const string str = arg;
      const size_t sep = str.find_first_of('=');
      if (sep==string::npos) {
        set_value(str, "");
      } else {
        const string name = str.substr(0, sep);
        const string value = str.substr(sep+1);
        set_value(name, value);
      }
    } else {
      const string str = arg;
      if (is_cfg_file(str)) {
        load(str);
      } else {
        USER_ERROR("don't know what to do with "
                   "command line parameter \""
                   <<arg <<'"');
      }
    }
  }
}

void ParameterBase::load(const string &filename) {
  ifstream is(filename.c_str());
  if (!is) {
    USER_ERROR("fail to open \"" << filename <<  "\"");
  }
  is >> noskipws;
  unsigned line = 1;
  while (true) {
    char c = skip(is);
    if (!is) break;
    if (c!='\n') {
      string param_name;
      is >> param_name;
      if (is_cfg_file(param_name)) {
        load(param_name);
      } else {
        c = skip(is);
        if (c!='\n') {
          string value;
          is >> value;
          set_value(param_name, value);
        } else {
          set_value(param_name, "");
        }
      }
      c = skip(is);
      if (c!='\n') {
        USER_ERROR(filename <<':' <<line
                   <<": garbage found after parameter value");
      }
    }
    ++line;
  }
  is.close();
}

void ParameterBase::set_value(const string& param_name,
                              const string& value) {
  unsigned found = 0;
  for (unsigned i = 0, ei = all().size(); i!=ei; ++i)
    if (all()[i]->m_name==param_name) {
      ++found;
      all()[i]->set_value(value);
    }
  if (found==0) {
    USER_ERROR("No parameter with this name: " << param_name);
  }
}

void ParameterBase::dump(ostream &os) {
  unsigned name_width = 5;
  unsigned value_width = 3;
  for (unsigned i = 0, ei = all().size(); i!=ei; ++i) {
    const ParameterBase &p = *all()[i];
    if (p.m_name.size()>name_width)
      name_width = p.m_name.size();
    if (p.get_value_width()>value_width)
      value_width = p.get_value_width();
  }

  os <<'#';
  fill(os, name_width-1, '-');
  os <<'+';
  fill(os, value_width+1, '-');
  os << "#---\n";
  for (unsigned i = 0, ei = all().size(); i!=ei; ++i) {
    const ParameterBase &p = *all()[i];
    p.print(os, name_width, value_width);
  }
  os <<'#';
  fill(os, name_width-1, '-');
  os <<'+';
  fill(os, value_width+1, '-');
  os << "#---\n";
}

ParameterBase::ParameterBase(const string &name,
                             const string &description):
  m_name(name),
  m_description(description)
{
  all().push_back(this);
}

vector<ParameterBase*>& ParameterBase::all() {
  static vector<ParameterBase*> params;
  return params;
}

// Integers ----------------------------------------------------------

ParameterInt::ParameterInt(const string &param_name,
                           const string &description,
                           const int default_value):
  ParameterBase(param_name, description),
  m_default_value(default_value),
  m_value(m_default_value)
{}

void ParameterInt::set_value(const string& value) {
  istringstream is;
  is.str(value);
  is >> m_value;
}

void ParameterInt::print(ostream &os) const {
  os <<m_name <<"  " <<m_value <<" # " <<m_description <<'\n';
}
 
unsigned ParameterInt::get_value_width() const {
  return string_of(m_value).size();
}

void ParameterInt::print(std::ostream &os,
                         unsigned c1, unsigned c2) const {
  os <<m_name;
  fill(os, c1-m_name.size()+1, ' ');
  os <<m_value;
  fill(os, c2-get_value_width(), ' ');
  os <<" # " <<m_description <<'\n';
}

// Strings -----------------------------------------------------------

ParameterString::ParameterString(const string &name,
                                 const string &description,
                                 const string &default_value):
  ParameterBase(name, description),
  m_default_value(default_value),
  m_value(m_default_value)
{}

void ParameterString::set_value(const string& value) {
  m_value = value;
}

void ParameterString::print(ostream &os) const {
  os <<m_name <<'\t' <<m_value <<"\t# " <<m_description <<'\n';
}
 
unsigned ParameterString::get_value_width() const {
  return m_value.size();
}

void ParameterString::print(std::ostream &os,
                            unsigned c1, unsigned c2) const {
  os <<m_name;
  fill(os, c1-m_name.size()+1, ' ');
  os <<m_value;
  fill(os, c2-get_value_width(), ' ');
  os <<" # " <<m_description <<'\n';
}

// Bools -------------------------------------------------------------

ParameterBool::ParameterBool(const string &name,
                             const string &description):
  ParameterBase(name, description),
  m_value(false)
{}

void ParameterBool::set_value(const string& value) {
  string tmp = value;
  transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
  m_value = !(tmp=="0" || tmp=="false" || tmp=="no");
}

void ParameterBool::print(ostream &os) const {
  os <<m_name <<'\t' <<(m_value ? "true" : "false")
     <<"\t# " <<m_description <<'\n';
}
 
unsigned ParameterBool::get_value_width() const {
  return m_value ? 4 : 5;
}

void ParameterBool::print(std::ostream &os,
                            unsigned c1, unsigned c2) const {
  os <<m_name;
  fill(os, c1-m_name.size()+1, ' ');
  os <<(m_value ? "true" : "false");
  fill(os, c2-get_value_width(), ' ');
  os <<" # " <<m_description <<'\n';
}

// Doubles ----------------------------------------------------------

ParameterDouble::ParameterDouble(const string &param_name,
                                 const string &description,
                                 const double default_value):
  ParameterBase(param_name, description),
  m_default_value(default_value),
  m_value(m_default_value)
{}

void ParameterDouble::set_value(const string& value) {
  istringstream is;
  is.str(value);
  is >> m_value;
}

void ParameterDouble::print(ostream &os) const {
  os <<m_name <<"  " <<m_value <<" # " <<m_description <<'\n';
}
 
unsigned ParameterDouble::get_value_width() const {
  return string_of(m_value).size();
}

void ParameterDouble::print(std::ostream &os,
                            unsigned c1, unsigned c2) const {
  os <<m_name;
  fill(os, c1-m_name.size()+1, ' ');
  os <<m_value;
  fill(os, c2-get_value_width(), ' ');
  os <<" # " <<m_description <<'\n';
}
