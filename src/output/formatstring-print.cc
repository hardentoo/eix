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


#include "formatstring-print.h"
#include <eixTk/sysutils.h>
#include <portage/vardbpkg.h>
#include <portage/conf/portagesettings.h>

using namespace std;

string
get_basic_version(const PrintFormat *fmt, const BasicVersion *version, bool pure_text, const string &intermediate)
{
	if((!fmt->show_slots))
		return version->getFull();
	if(pure_text || fmt->no_color || (!fmt->colored_slots))
		return version->getFullSlotted(fmt->colon_slots, intermediate);
	string slot = version->getSlotAppendix(fmt->colon_slots);
	if(slot.empty())
		return version->getFull();
	return version->getFull() + intermediate + fmt->color_slots + slot +
		AnsiColor::reset();
}

string
get_inst_use(const Package &p, InstVersion &i, const PrintFormat &fmt, const char **a)
{
	if(!fmt.vardb->readUse(p, i))
		return "";
	if(i.inst_iuse.empty())
		return "";
	string ret, add;
	for(vector<string>::iterator it = i.inst_iuse.begin();
		it != i.inst_iuse.end(); ++it)
	{
		int addindex = 0;
		string *curr = &ret;
		if(i.usedUse.find(*it) == i.usedUse.end()) {
			addindex = 2;
			if(!fmt.alpha_use)
				curr = &add;
		}
		if(!curr->empty())
			curr->append(" ");
		if(a[addindex])
			curr->append(a[addindex]);
		else if(addindex == 2)
			curr->append("-");
		curr->append(*it);
		if(a[addindex+1])
			curr->append(a[addindex+1]);
	}
	if(!add.empty()) {
		if(!ret.empty())
			ret.append(" ");
		ret.append(add);
	}
	return ret;
}

string
getFullInstalled(const Package &p, const PrintFormat &fmt)
{
	if(!fmt.vardb)
		return "";
	vector<InstVersion> *vec = fmt.vardb->getInstalledVector(p);
	if(!vec) {
		return "";
	}

	string ret;
	for(vector<InstVersion>::iterator it = vec->begin();
		it != vec->end(); ++it) {
		if(!ret.empty())
			ret.append("\n");
		ret.append(p.category + "/" + p.name + "-" + it->getFull());
	}
	return ret;
}

const char
	INST_WITH_DATE=1,      //< Installed string should contain date
	INST_WITH_USEFLAGS=2,  //< Installed string should contain useflags
	INST_WITH_NEWLINE=4,   //< Installed string should contain newlines
	INST_SHORTDATE=8;      //< date should be in short format
string
getInstalledString(const Package &p, const PrintFormat &fmt, bool pure_text, char formattype, const vector<string> &prepend)
{
	if(!fmt.vardb)
		return "";
	vector<InstVersion> *vec = fmt.vardb->getInstalledVector(p);
	if(!vec) {
		return "";
	}

	vector<InstVersion>::iterator it = vec->begin();
	if(it == vec->end())
		return "";
	string ret;
	bool useflags;
	bool color = !fmt.no_color;
	if(pure_text)
		color = false;
	for(;;) {
		if(!p.guess_slotname(*it, fmt.vardb))
			it->slot = "?";
		if(prepend.size() > 0)
			ret.append(prepend[0]);
		ret.append(get_basic_version(&fmt, &(*it), pure_text,
			((prepend.size() > 1) ? prepend[1] : "")));
		if(fmt.vardb->readOverlay(p, *it, *fmt.header, (*(fmt.portagesettings))["PORTDIR"].c_str())) {
			if(it->overlay_key>0) {
				if((!p.have_same_overlay_key) || (p.largest_overlay != it->overlay_key)) {
					if(prepend.size() > 2)
						ret.append(prepend[2]);
					ret.append(fmt.overlay_keytext(it->overlay_key, !color));
				}
			}
		}
		else
		// Uncomment if you do not want to print unreadable overlays as "[?]"
		// if(!it->overlay_keytext.empty())
		{
			if(prepend.size() > 2)
				ret.append(prepend[2]);
			if(color)
				ret.append(fmt.color_virtualkey);
			ret.append("[");
			if(it->overlay_keytext.empty())
				ret.append("?");
			else
				ret.append(it->overlay_keytext);
			ret.append("]");
			if(color)
				ret.append(AnsiColor(AnsiColor::acDefault).asString());
		}
		if(prepend.size() > 3)
			ret.append(prepend[3]);
		if(formattype & INST_WITH_DATE)
		{
			string date =
				date_conv(((formattype & INST_SHORTDATE) ?
					fmt.dateFormatShort.c_str() :
					fmt.dateFormat.c_str()),
					it->instDate);
			if(!date.empty())
			{
				if(prepend.size() > 5)
					ret.append(prepend[5]);
				ret.append(date);
				if(prepend.size() > 6)
					ret.append(prepend[6]);
			}
		}
		useflags = false;
		if(formattype & INST_WITH_USEFLAGS) {
			const char *a[4];
			for(vector<string>::size_type i=0; i<4; ++i) {
				a[i] = NULL;
				if(prepend.size() > i + 9)
					if((i == 2) || (!prepend[i + 9].empty()))
						a[i] = prepend[i + 9].c_str();
			}
			string inst_use = get_inst_use(p, *it, fmt, a);
			if(!inst_use.empty()) {
				if(prepend.size() > 7)
					ret.append(prepend[7]);
				ret.append(inst_use);
				useflags = true;
				if(prepend.size() > 8)
					ret.append(prepend[8]);
			}
		}
		if(prepend.size() > 4)
			ret.append(prepend[4]);
		if(++it == vec->end())
			return ret;
		if(prepend.size() > 13)
			ret.append(prepend[13]);
		else if((formattype & INST_WITH_NEWLINE) &&
				(useflags || fmt.style_version_lines))
				ret.append("\n\t\t\t  ");
		else
				ret.append(" ");
	}
}

void
print_version(const PrintFormat *fmt, const Version *version, const Package *package, bool with_slots, bool exclude_overlay, bool full)
{
	bool is_installed = false;
	bool is_marked = false;
	InstVersion *inst = NULL;
	if(!fmt->no_color)
	{
		if(fmt->vardb)
			is_installed = fmt->vardb->isInstalled(*package, version, &inst);
		if(fmt->marked_list)
			is_marked = fmt->marked_list->is_marked(*package, version);
	}

	// do not mark installed version if it is from a different overlay:
	if(is_installed && inst && (!(package->have_same_overlay_key))) {
		if(fmt->vardb->readOverlay(*package, *inst, *(fmt->header), (*(fmt->portagesettings))["PORTDIR"].c_str())) {
			if((inst->overlay_key) != (version->overlay_key))
				is_installed = false;
		}
	}

	if((!full) && fmt->style_version_lines)
		fputs("\n\t\t", stdout);

	bool need_color = !(fmt->no_color);
	string mask_text, keyword_text;

	Version::SavedMaskIndex orimask_i = fmt->orimask_index;
	if(!version->have_saved_masks[orimask_i]) {
		orimask_i = Version::SAVEMASK_FILE;
		if(!version->have_saved_masks[orimask_i]) {
			throw ExBasic("internal error: nonlocal mask %d/%d is read without being stored", int(fmt->orimask_index), int(orimask_i));
		}
	}
	MaskFlags orimask(version->saved_masks[orimask_i]),
		newmask(version->maskflags);
	if(orimask.isHardMasked()) {
		if( need_color && fmt->color_original ) {
			need_color = false;
			cout << fmt->color_masked;
		}
		if(newmask.isProfileMask()) {
			if( need_color ) {
				need_color = false;
				cout << fmt->color_masked;
			}
			mask_text = fmt->tag_for_profile;
		}
		else if(newmask.isPackageMask()) {
			if( need_color ) {
				need_color = false;
				cout << fmt->color_masked;
			}
			mask_text = fmt->tag_for_masked;
		}
		else if(orimask.isProfileMask()) {
			mask_text = fmt->tag_for_ex_profile;
		}
		else {
			mask_text = fmt->tag_for_ex_masked;
		}
	}
	else if(newmask.isHardMasked()) {
		if( need_color && fmt->color_local_mask ) {
			need_color = false;
			cout << fmt->color_masked;
		}
		mask_text = fmt->tag_for_locally_masked;
	}

	if(!version->have_saved_keywords[fmt->orikey_index]) {
		throw ExBasic("internal error: nonlocal key is read without being stored");
	}
	KeywordsFlags orikey(version->saved_keywords[fmt->orikey_index]),
		newkey(version->keyflags);
	if(newkey.isStable()) {
		if( need_color && !(fmt->color_original) ) {
			need_color = false;
			cout << fmt->color_stable;
		}
		if (orikey.isStable()) {
			if( need_color )
				cout << fmt->color_stable;
			keyword_text = fmt->tag_for_stable;
		}
		else if (orikey.isUnstable()) {
			if( need_color )
				cout << fmt->color_unstable;
			keyword_text = fmt->tag_for_ex_unstable;
		}
		else if (orikey.isMinusKeyword()) {
			if( need_color )
				cout << fmt->color_masked;
			keyword_text = fmt->tag_for_ex_minus_keyword;
		}
		else if (orikey.isAlienStable()) {
			if( need_color )
				cout << fmt->color_masked;
			keyword_text = fmt->tag_for_ex_alien_stable;
		}
		else if (orikey.isAlienUnstable()) {
			if( need_color )
				cout << fmt->color_masked;
			keyword_text = fmt->tag_for_ex_alien_unstable;
		}
		else if (orikey.isMinusAsterisk()) {
			if( need_color )
				cout << fmt->color_masked;
			keyword_text = fmt->tag_for_ex_minus_asterisk;
		}
		else {
			if( need_color )
				cout << fmt->color_masked;
			keyword_text = fmt->tag_for_ex_missing_keyword;
		}
	}
	else if (newkey.isUnstable()) {
		if( need_color )
			cout << fmt->color_unstable;
		keyword_text = fmt->tag_for_unstable;
	}
	else if (newkey.isMinusKeyword()) {
		if( need_color )
			cout << fmt->color_masked;
		keyword_text = fmt->tag_for_minus_keyword;
	}
	else if (newkey.isAlienStable()) {
		if( need_color )
			cout << fmt->color_masked;
		keyword_text = fmt->tag_for_alien_stable;
	}
	else if (newkey.isAlienUnstable()) {
		if( need_color )
			cout << fmt->color_masked;
		keyword_text = fmt->tag_for_alien_unstable;
	}
	else if (newkey.isMinusAsterisk()) {
		if( need_color )
			cout << fmt->color_masked;
		keyword_text = fmt->tag_for_minus_asterisk;
	}
	else {
		if( need_color )
			cout << fmt->color_masked;
		keyword_text = fmt->tag_for_missing_keyword;
	}
	cout << mask_text << keyword_text;

	if((!full) && fmt->style_version_lines)
		fputs("\t", stdout);

	if (is_installed)
		cout << fmt->mark_installed;
	if (is_marked)
		cout << fmt->mark_version;
	if (full)
		cout << package->category << "/" << package->name << "-";
	if (with_slots && fmt->show_slots && (!fmt->colored_slots))
		cout << version->getFullSlotted(fmt->colon_slots);
	else
		cout << version->getFull();
	if (is_marked)
	{
		cout << fmt->mark_version_end;
		if(is_installed &&
			(fmt->mark_version_end != fmt->mark_installed_end))
		{
			cout << fmt->mark_installed_end;
		}
	}
	else if (is_installed)
		cout << fmt->mark_installed_end;
	if (with_slots && fmt->show_slots && fmt->colored_slots)
	{
		string slot = version->getSlotAppendix(fmt->colon_slots);
		if(!slot.empty())
		{
			if(! fmt->no_color)
				cout << fmt->color_slots;
			cout << slot;
		}
	}
	if(!exclude_overlay)
	{
		if(!package->have_same_overlay_key && version->overlay_key)
			cout << fmt->overlay_keytext(version->overlay_key);
	}
	if(!fmt->no_color)
		cout << AnsiColor::reset();
	if(fmt->print_iuse && fmt->style_version_lines) {
		if(!(version->iuse.empty())) {
			cout << fmt->before_iuse <<
				join_vector(version->iuse) << fmt->after_iuse;
		}
	}
}

void
print_versions_versions(const PrintFormat *fmt, const Package* p, bool with_slots, bool full)
{
	for(Package::const_iterator vit = p->begin(); vit != p->end(); ++vit) {
		if(vit != p->begin()) {
			if(full)
				cout << "\n";
			else if(!fmt->style_version_lines)
			cout << " ";
		}
		print_version(fmt, *vit, p, with_slots, false, full);
	}
	if( !fmt->no_color )
		cout << AnsiColor::reset();
	bool print_coll_iuse = fmt->print_iuse;
#if !defined(NOT_FULL_USE)
	if((fmt->style_version_lines) && (p->versions_have_full_use))
		print_coll_iuse = false;
#endif
	if(print_coll_iuse) {
		if(!(p->coll_iuse.empty())) {
			cout << fmt->before_coll_iuse << p->coll_iuse << fmt->after_coll_iuse;
		}
	}
}

void
print_versions_slots(const PrintFormat *fmt, const Package* p, bool full)
{
	if(!p->have_nontrivial_slots)
	{
		print_versions_versions(fmt, p, false, full);
		return;
	}
	const SlotList *sl = &(p->slotlist);
	bool only_one = (sl->size() == 1);
	for(SlotList::const_iterator it = sl->begin();
		it != sl->end(); ++it)
	{
		if(!full) {
			const char *s = it->slot();
			if((!only_one) || fmt->style_version_lines)
				fputs("\n\t", stdout);
			if( !fmt->no_color)
				cout << fmt->color_slots;
			if(s[0])
				cout << "(" << s << ")";
			else
				cout << "(0)";
			if( !fmt->no_color)
				cout << AnsiColor::reset();
			if( !fmt->style_version_lines)
				cout << (only_one ? "  " : "\t");
		}
		const VersionList *vl = &(it->const_version_list());
		for(VersionList::const_iterator vit = vl->begin();
			vit != vl->end(); ++vit) {
			if(full) {
				if((vit != vl->begin()) || (it != sl->begin()))
					cout << "\n";
			}
			else if((vit != vl->begin()) && !fmt->style_version_lines)
				cout << " ";
			print_version(fmt, *vit, p, full, false, full);
		}
		if( !fmt->no_color )
			cout << AnsiColor::reset();
	}
	bool print_coll_iuse = fmt->print_iuse;
#if !defined(NOT_FULL_USE)
	if((fmt->style_version_lines) && (p->versions_have_full_use))
		print_coll_iuse = false;
#endif
	if(print_coll_iuse) {
		if(!(p->coll_iuse.empty())) {
			cout << fmt->before_slot_iuse << p->coll_iuse << fmt->after_slot_iuse;
		}
	}
}

void
print_versions(const PrintFormat *fmt, const Package* p, bool with_slots, bool full)
{
	if(fmt->slot_sorted)
		print_versions_slots(fmt, p, full);
	else
		print_versions_versions(fmt, p, with_slots, full);
}

bool
print_package_property(const PrintFormat *fmt, const void *void_entity, const string &name) throw(ExBasic)
{
	const Package *entity = static_cast<const Package *>(void_entity);

	vector<string> prepend = split_string(name, ":", false, true, true);
	string plainname = prepend[0];
	prepend.erase(prepend.begin());

	if((name == "availableversions") ||
		(name == "availableversionslong") ||
		(name == "availableversionsshort")) {
		print_versions(fmt, entity, (name != "availableversionsshort"), false);
		return true;
	}
	if((name == "fullavailable") ||
		(name == "fullavailablelong") ||
		(name == "fullavailableshort")) {
		print_versions(fmt, entity, (name != "fullavailableshort"), true);
		return true;
	}
	if((plainname == "installedversions") ||
		(plainname == "installedversionsdate") ||
		(plainname == "installedversionsshortdate") ||
		(plainname == "installedversionsshort")) {
		if(!fmt->vardb)
			return false;
		char formattype = 0;
		if(plainname != "installedversionsshort") {
			formattype = INST_WITH_DATE;
			if(plainname == "installedversions")
				formattype |= INST_WITH_USEFLAGS|INST_WITH_NEWLINE;
			else if(plainname == "installedversionsshortdate")
				formattype |= INST_SHORTDATE;
		}
		string s = getInstalledString(*entity, *fmt, false, formattype, prepend);
		if(s.empty())
			return false;
		cout << s;
		return true;
	}
	if(plainname == "fullinstalled") {
		string s = getFullInstalled(*entity, *fmt);
		if(s.empty())
			return false;
		cout << s;
		return true;
	}
	if(name == "overlaykey") {
		Version::Overlay ov_key = entity->largest_overlay;
		if(ov_key && entity->have_same_overlay_key) {
			cout << fmt->overlay_keytext(ov_key);
			return true;
		}
		return false;
	}
	if((name == "best") ||
		(name == "bestlong") ||
		(name == "bestshort")) {
		Version *best = entity->best();
		if(best == NULL)
			return false;
		print_version(fmt, best, entity, (name != "bestshort"), false, false);
		return true;
	}
	if((name == "bestslots") ||
		(name == "bestslotslong") ||
		(name == "bestslotsshort")) {
		vector<Version*> versions;
		entity->best_slots(versions);
		for(vector<Version*>::const_iterator it = versions.begin();
			it != versions.end(); ++it)
		{
			if(it != versions.begin())
				cout << " ";
			print_version(fmt, *it, entity, (name != "bestslotshort"), false, false);
		}
		return (versions.begin() != versions.end());
	}
	string s = get_package_property(fmt, void_entity, name);
	if(s.empty())
		return false;
	cout << s;
	return true;
}

string
get_package_property(const PrintFormat *fmt, const void *void_entity, const string &name) throw(ExBasic)
{
	const Package *entity = static_cast<const Package *>(void_entity);

	if(name == "category") {
		return entity->category;
	}
	if(name == "name") {
		return entity->name;
	}
	if(name == "description") {
		return entity->desc;
	}
	if(name == "homepage") {
		return entity->homepage;
	}
	if(name == "licenses") {
		return entity->licenses;
	}
	if((name == "installedversions") ||
		(name == "installedversionsdate") ||
		(name == "installedversionsshortdate") ||
		(name == "installedversionsshort")) {
		if(!fmt->vardb)
			return "";
		char formattype = 0;
		if(name != "installedversionsshort") {
			formattype = INST_WITH_DATE;
			if(name == "installedversions")
				formattype |= INST_WITH_USEFLAGS|INST_WITH_NEWLINE;
			else if(name == "installedversionsshortdate")
				formattype |= INST_SHORTDATE;
		}
		vector<string> prepend;
		return getInstalledString(*entity, *fmt, true, formattype, prepend);
	}
	if(name == "provide") {
		return entity->provide;
	}
	if(name == "overlaykey") {
		Version::Overlay ov_key = entity->largest_overlay;
		if(ov_key && entity->have_same_overlay_key) {
			return fmt->overlay_keytext(ov_key, false);
		}
		return "";
	}
	if(name == "system") {
		if(entity->is_system_package) {
			return "system";
		}
		return "";
	}
	if((name == "best") ||
		(name == "bestlong") ||
		(name == "bestshort")) {
		Version *best = entity->best();
		if(best == NULL) {
			return "";
		}
		if(fmt->show_slots && (name != "bestshort"))
			return best->getFullSlotted(fmt->colon_slots);
		return best->getFull();
	}
	if((name == "bestslots") ||
		(name == "bestslotslong") ||
		(name == "bestslotsshort")) {
		bool with_slots = (name != "bestslotshort");
		vector<Version*> versions;
		entity->best_slots(versions);
		string ret;
		for(vector<Version*>::const_iterator it = versions.begin();
			it != versions.end(); ++it)
		{
			if(ret.length())
				ret += " ";
			if(with_slots)
				ret += (*it)->getFullSlotted(fmt->colon_slots);
			else
				ret += (*it)->getFull();
		}
		return ret;
	}
	if(name == "marked")
	{
		if(fmt->marked_list)
		{
			if(fmt->marked_list->is_marked(*entity))
				return "1";
		}
		return "";
	}
	if(name == "markedversions")
	{
		if(fmt->marked_list)
			return fmt->marked_list->getMarkedString(*entity);
		return "";
	}
	if(name == "upgrade")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->can_upgrade(fmt->vardb, true, true);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	if(name == "upgradeorinstall")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->can_upgrade(fmt->vardb, false, true);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	if(name == "downgrade")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->must_downgrade(fmt->vardb, true);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	if(name == "recommend")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->recommend(fmt->vardb, true, true);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	if(name == "recommendorinstall")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->recommend(fmt->vardb, false, true);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	if(name == "bestupgrade")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->can_upgrade(fmt->vardb, true, false);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	if(name == "bestupgradeorinstall")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->can_upgrade(fmt->vardb, false, false);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	if(name == "bestdowngrade")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->must_downgrade(fmt->vardb, false);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	if(name == "bestrecommend")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->recommend(fmt->vardb, true, false);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	if(name == "bestrecommendorinstall")
	{
		LocalCopy copy(fmt, const_cast<Package*>(entity));
		bool result = entity->recommend(fmt->vardb, false, false);
		copy.restore(const_cast<Package*>(entity));
		if(result)
			return "1";
		return "";
	}
	throw(ExBasic("Unknown property '%s'.", name.c_str()));
}

const void *old_or_new(string *new_name, const Package *older, const Package *newer, const string &name)
{
	const char *s = name.c_str();
	if(strncmp(s, "old", 3) == 0)
	{
		*new_name = s + 3;
		return older;
	}
	if(strncmp(s, "new", 3) == 0)
	{
		*new_name = s + 3;
		return newer;
	}
	*new_name = name;
	return newer;
}

string
get_diff_package_property(const PrintFormat *fmt, const void *void_entity, const string &name) throw(ExBasic)
{
	const Package *older = (static_cast<const Package* const*>(void_entity))[0];
	const Package *newer = (static_cast<const Package* const*>(void_entity))[1];
	if(name == "better")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = newer->have_worse(*older, true);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "1";
		return "";
	}
	if(name == "bestbetter")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = newer->have_worse(*older, false);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "1";
		return "";
	}
	if(name == "worse")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = older->have_worse(*newer, true);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "1";
		return "";
	}
	if(name == "bestworse")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = older->have_worse(*newer, false);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "1";
		return "";
	}
	if(name == "differ")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = newer->differ(*older, true);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "1";
		return "";
	}
	if(name == "bestdiffer")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = newer->differ(*older, false);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "1";
		return "";
	}
	string new_name;
	const void *entity = old_or_new(&new_name, older, newer, name);
	return get_package_property(fmt, entity, new_name);
}

bool
print_diff_package_property(const PrintFormat *fmt, const void *void_entity, const string &name) throw(ExBasic)
{
	const Package *older = (static_cast<const Package* const*>(void_entity))[0];
	const Package *newer = (static_cast<const Package* const*>(void_entity))[1];
	string new_name;
	const void *entity = old_or_new(&new_name, older, newer, name);
	return print_package_property(fmt, entity, new_name);
}
