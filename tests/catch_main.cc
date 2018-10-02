/**
 * Copyright (C) 2018 CS - Systemes d'Information (CS-SI)
 *
 * This file is part of Sirius
 *
 *     https://github.com/CS-SI/SIRIUS
 *
 * Sirius is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Sirius is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Sirius.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * \file catch_main.cc
 * \brief This compilation unit contains catch2 main
 * This implementation is inspired by
 *     https://github.com/catchorg/Catch2/blob/master/docs/own-main.md
 */

#define CATCH_CONFIG_RUNNER
#include <catch/catch.hpp>

#include "sirius/utils/log.h"

int main(int argc, char* argv[]) {
    Catch::Session session;  // There must be exactly one instance

    std::string verbosity_level = "info";  // Some user variable you want to be able to set

    // Build a new parser on top of Catch's
    using namespace Catch::clara;
    auto cli =
          session.cli()
          | Opt(verbosity_level, "sirius verbosity level")["-V"]["--sirius-verbosity"]
            ("verbosity level (trace, debug, info, warn, error, critical, off");

    // Now pass the new composite back to Catch so it uses that
    session.cli(cli);

    // Let Catch (using Clara) parse the command line
    int return_code = session.applyCommandLine(argc, argv);
    if (return_code != 0) {
        return return_code;
    }

    sirius::utils::SetVerbosityLevel(verbosity_level);

    return session.run();
}
