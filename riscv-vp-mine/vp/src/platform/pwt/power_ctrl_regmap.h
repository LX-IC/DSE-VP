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

#ifndef _POWER_CTRL_REGMAP_H_
#define _POWER_CTRL_REGMAP_H_

#define POWER_CTRL_REG_OFFSET 0x0
#define POWER_CTRL_REG_TOPFREQ 0x4

/* macro composing power commands */
/* command = 1bit (invalid) -- 3bits (componentID) -- 2bits (voltage) -- 2bits (freq)*/

#define PWCTRL_ID_TOP 0
#define PWCTRL_ID_MB 1
#define PWCTRL_ID_MBn(i) 1+i

#define PWCTRL_VOLT_OFF 0
#define PWCTRL_VOLT_NORMAL 1
#define PWCTRL_VOLT_LOW 2

#define PWCTRL_FREQ_OFF 0
#define PWCTRL_FREQ_NORMAL 1
#define PWCTRL_FREQ_LOW 2

#define PW_COM(cpt, volt, freq) ((0) | (cpt << 4) | (volt << 2) | freq)

#define PW_COMS1(com1)		\
	com1 | 0XFFFFFF << 8

#define PW_COMS2(com1, com2)		\
	com1 | com2 << 8 | 0XFFFF << 16

#define PW_COMS3(com1, com2, com3)			\
	com1 | com2 << 8 | com3 << 16 | 0xFF << 24

#define PW_COMS4(com1, com2, com3, com4)		\
	com1 | com2 << 8 | com3 << 16 | com4 << 24

#endif // _POWER_CTRL_REGMAP_H_
