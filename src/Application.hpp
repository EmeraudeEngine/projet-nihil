/*
 * src/Application.hpp
 * This file is part of Projet-Nihil
 *
 * Copyright (C) 2010-2026 - Sébastien Léon Claude Christian Bémelmans "LondNoir" <londnoir@gmail.com>
 *
 * Projet-Nihil is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * Projet-Nihil is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Projet-Nihil; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Complete project and additional information can be found at :
 * https://github.com/EmeraudeEngine/projet-nihil
 *
 * --- THIS IS AUTOMATICALLY GENERATED, DO NOT CHANGE ---
 */

#pragma once

/* Project configuration. */
#include "config.hpp"

/* Local inclusions for inheritances. */
#include "Core.hpp"
#include "ApplicationSettingKeys.hpp"

namespace ProjetNihil
{
	/**
	 * @brief The main class to initialize the user application.
	 * @extends EmEn::Core This is the base class for an emeraude-based application.
	 */
	class Application final : public EmEn::Core
	{
		public:

			/** @brief Class identifier. */
			static constexpr auto ClassId{"ProjetNihil"};

			/**
			 * @brief Constructs the application.
			 * @param argc The argument count from the standard C/C++ main() function.
			 * @param argv The argument value from the standard C/C++ main() function.
			 */
			Application (int argc, char * * argv) noexcept;

#if IS_WINDOWS
			/**
			 * @brief Constructs the application (Windows version).
			 * @param argc The argument count from the standard C/C++ main() function.
			 * @param wargv The argument value from the standard C/C++ main() function.
			 */
			Application (int argc, wchar_t * * wargv) noexcept;
#endif

			/**
			 * @brief Copy constructor.
			 * @param copy A reference to the copied instance.
			 */
			Application (const Application & copy) noexcept = delete;

			/**
			 * @brief Move constructor.
			 * @param copy A reference to the copied instance.
			 */
			Application (Application && copy) noexcept = delete;

			/**
			 * @brief Copy assignment.
			 * @param copy A reference to the copied instance.
			 * @return AbstractWeapon &
			 */
			Application & operator= (const Application & copy) noexcept = delete;

			/**
			 * @brief Move assignment.
			 * @param copy A reference to the copied instance.
			 * @return Application &
			 */
			Application & operator= (Application && copy) noexcept = delete;

			/**
			 * @brief Destructs the application.
			 */
			~Application () override = default;

			/**
			 * @brief Returns the unique identifier for this class [Thread-safe].
			 * @return size_t
			 */
			static
			size_t
			getClassUID () noexcept
			{
				return EmEn::Libs::Hash::FNV1a(ClassId);
			}

			/** @copydoc EmEn::Libs::ObservableTrait::classUID() const */
			[[nodiscard]]
			size_t
			classUID () const noexcept override
			{
				return getClassUID();
			}

			/** @copydoc EmEn::Libs::ObservableTrait::is() const */
			[[nodiscard]]
			bool
			is (size_t classUID) const noexcept override
			{
				return classUID == getClassUID();
			}

			/**
			 * @brief Returns the reference to the application help service.
			 * @return const EmEn::Help &
			 */
			[[nodiscard]]
			const EmEn::Help &
			help () const noexcept
			{
				return m_applicationHelp;
			}

			/**
			 * @brief Returns the reference to the application help service.
			 * @return const Help &
			 */
			[[nodiscard]]
			EmEn::Help &
			applicationHelp () noexcept
			{
				return m_applicationHelp;
			}

		private:

			/** @copydoc EmEn::Core::onBeforeCoreSecondaryServicesInitialization() */
			[[nodiscard]]
			bool onBeforeCoreSecondaryServicesInitialization () noexcept override;

			/** @copydoc EmEn::Core::onCoreStarted(const EmEn::Arguments &, EmEn::Settings &) */
			bool onCoreStarted (const EmEn::Arguments & arguments, EmEn::Settings & settings) noexcept override;

			/** @copydoc EmEn::Core::onCoreProcessLogics() */
			void onCoreProcessLogics (size_t engineCycle) noexcept override;

			/** @copydoc EmEn::Core::onCoreKeyRelease() */
			bool onCoreKeyRelease (int32_t key, int32_t scancode, int32_t modifiers) noexcept override;

			EmEn::Help m_applicationHelp{"Application"};
			std::weak_ptr< EmEn::Scenes::Node > m_cameraNode;
			std::weak_ptr< EmEn::Scenes::Node > m_cubeNode;
			bool m_useStaticLighting{DefaultUseStaticLighting};
	};
}
