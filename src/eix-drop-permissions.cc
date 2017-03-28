// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <unistd.h>

#include <cstdlib>
#include <cstring>

#include <string>

#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"
#include "main/main.h"
#include "various/drop_permissions.h"

using std::string;

static void print_help() {
	eix::say(_("Usage: %s [--] command [arguments]\n"
"Executes \"command [arguments]\" with the permissions according to the eix\n"
"variables EIX_USER, EIX_GROUP, EIX_UID, and EIX_GID,\n"
"honouring REQUIRE_DROP and NODROP_FATAL.\n"
"\n"
"This program is covered by the GNU General Public License. See COPYING for\n"
"further information.")) % program_name;
}

int run_eix_drop_permissions(int argc, char *argv[]) {
	EixRc& eixrc(get_eixrc(DROP_VARS_PREFIX));
	if(argc > 0) {
		++argv;
		--argc;
	}
	if((argc > 0) && (std::strcmp("--", argv[0]) == 0)) {
		++argv;
		--argc;
	}
	if(argc == 0) {
		print_help();
		return EXIT_FAILURE;
	}
	string errtext;
	bool success(drop_permissions(&eixrc, &errtext));
	if(unlikely(!errtext.empty())) {
		eix::say_error() % errtext;
	}
	if(unlikely(!success)) {
		return EXIT_FAILURE;
	}
	execv(argv[0], argv);
	eix::say_error(_("failed to execute %s")) % argv[0];
	return EXIT_FAILURE;
}
