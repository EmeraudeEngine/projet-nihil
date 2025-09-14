/*
 * src/Application.hpp
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

#pragma once

/* Project configuration. */
#include "config.hpp"

/* Local inclusions for inheritances. */
#include "Core.hpp"

namespace ProjetNihil
{
	class Application final : public EmEn::Core
	{
		public:

			/** @brief Class identifier. */
			static constexpr auto ClassId{"ProjetNihil"};

			/** @brief Observable class unique identifier. */
			static const size_t ClassUID;

			/**
			 * @brief Constructs the application.
			 * @param argc The argument count from the standard C/C++ main() function.
			 * @param argv The argument value from the standard C/C++ main() function.
			 */
			Application (int argc, char * * argv) noexcept;

#if IS_WINDOWS
			/**
			 * @brief Constructs the application.
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

			/** @copydoc EmEn::Libs::ObservableTrait::classUID() const */
			[[nodiscard]]
			size_t
			classUID () const noexcept override
			{
				return ClassUID;
			}

			/** @copydoc EmEn::Libs::ObservableTrait::is() const */
			[[nodiscard]]
			bool
			is (size_t classUID) const noexcept override
			{
				return classUID == ClassUID;
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

			/** @copydoc EmEn::Core::onBeforeSecondaryServicesInitialization() */
			[[nodiscard]]
			bool onBeforeSecondaryServicesInitialization () noexcept override;

			/** @copydoc EmEn::Core::onStart() */
			bool onStart () noexcept override;

			/** @copydoc EmEn::Core::onResume() */
			void onResume () noexcept override;

			/** @copydoc EmEn::Core::onProcessLogics() */
			void onProcessLogics (size_t engineCycle) noexcept override;

			/** @copydoc EmEn::Core::onPause() */
			void onPause () noexcept override;

			/** @copydoc EmEn::Core::onStop() */
			void onStop () noexcept override;

			/** @copydoc EmEn::Core::onAppKeyPress() */
			bool onAppKeyPress (int32_t key, int32_t scancode, int32_t modifiers, bool repeat) noexcept override;

			/** @copydoc EmEn::Core::onAppKeyRelease() */
			bool onAppKeyRelease (int32_t key, int32_t scancode, int32_t modifiers) noexcept override;

			/** @copydoc EmEn::Core::onAppCharacterType() */
			bool onAppCharacterType (uint32_t unicode) noexcept override;

			/** @copydoc EmEn::Core::onAppNotification() */
			bool onAppNotification (const ObservableTrait * observable, int notificationCode, const std::any & data) noexcept override;

			/** @copydoc EmEn::Core::onOpenFiles() */
			void onOpenFiles (const std::vector< std::filesystem::path > & filepaths) noexcept override;

			EmEn::Help m_applicationHelp{"Application"};
			std::weak_ptr< EmEn::Scenes::Node > m_cubeNode;
			bool m_useStaticLighting{false};
	};
}
