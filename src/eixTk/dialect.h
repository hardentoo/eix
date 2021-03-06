// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_DIALECT_H_
#define SRC_EIXTK_DIALECT_H_ 1

#include <config.h>

// define C++-11 dialect specific keywords (if available)
// or poor man's substitutes

// check_includes: include "eixTk/dialect.h"

#ifdef HAVE_CONSTEXPR
#define CONSTEXPR constexpr
#else
#define CONSTEXPR
#endif

#ifdef HAVE_OVERRIDE
#define OVERRIDE override
#else
#define OVERRIDE
#endif

#ifdef HAVE_DELETE
#define ASSIGN_DELETE = delete
#else
#define ASSIGN_DELETE
#endif

#ifdef HAVE_NOEXCEPT
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

#endif  // SRC_EIXTK_DIALECT_H_
