/*
 * src/Boot/mac/main.mm
 * This file is part of Emeraude-Engine
 *
 * Copyright (C) 2010-2025 - Sébastien Léon Claude Christian Bémelmans "LondNoir" <londnoir@gmail.com>
 *
 * Emeraude-Engine is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * Emeraude-Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Emeraude-Engine; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Complete project and additional information can be found at :
 * https://github.com/londnoir/emeraude-engine
 *
 * --- THIS IS AUTOMATICALLY GENERATED, DO NOT CHANGE ---
 */

/* STL inclusions. */
#include <iostream>

/* Local inclusions. */
#include "Application.hpp"

int
main (int argc, char * argv[])
{
	ProjetNihil::Application application{argc, argv};

	if ( application.showHelp() )
	{
		std::cout << application.coreHelp().getHelp() << application.help().getHelp();

		return EXIT_SUCCESS;
	}

	std::cout << "[BOOT:MACOS] Running the main process ..." "\n";

	if ( !application.run() )
	{
		std::cerr << "[BOOT:MACOS] Application terminated with errors !" "\n";

		return EXIT_FAILURE;
	}

	std::cout << "[BOOT:MACOS] Application terminated successfully !" "\n";

	return EXIT_SUCCESS;
}
