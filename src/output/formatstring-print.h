/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin V�th <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __FORMATSTRING_PRINT_H__
#define __FORMATSTRING_PRINT_H__

#include <string>
#include <output/formatstring.h>
#include <portage/package.h>
#include <portage/version.h>
#include <eixTk/exceptions.h>

std::string get_basic_version(const PrintFormat *fmt, const BasicVersion *version, bool pure_text, const std::string &intermediate = "");
std::string get_inst_use(const Package &p, InstVersion &i, const PrintFormat &fmt, const char **a);
std::string getFullInstalled(const Package &p, const PrintFormat &fmt);
std::string getInstalledString(const Package &p, const PrintFormat &fmt, bool pure_text, char formattype, const std::vector<std::string> &prepend);
void print_version(const PrintFormat *fmt, const Version *version, const Package *package, bool with_slot, bool exclude_overlay);
void print_versions_versions(const PrintFormat *fmt, const Package *p, bool with_slot);
void print_versions_slots(const PrintFormat *fmt, const Package *p);
void print_versions(const PrintFormat *fmt, const Package *p, bool with_slot);

bool print_package_property(const PrintFormat *fmt, const void *entity, const std::string &name) throw(ExBasic);
std::string get_package_property(const PrintFormat *fmt, const void *entity, const std::string &name) throw(ExBasic);

bool print_diff_package_property(const PrintFormat *fmt, const void *void_entity, const std::string &name) throw(ExBasic);
std::string get_diff_package_property(const PrintFormat *fmt, const void *void_entity, const std::string &name) throw(ExBasic);

#endif /* __FORMATSTRING-PRINT_H__ */
