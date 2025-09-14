/*
 * src/Application.cpp
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

#include "Application.hpp"

/* Local inclusions. */
#include "PlatformSpecific/Desktop/Dialog/Message.hpp"
#include "ApplicationSettingKeys.hpp"
#include "Graphics/Geometry/ResourceGenerator.hpp"
#include "Graphics/Renderable/SkyBoxResource.hpp"
#include "Graphics/Renderable/BasicFloorResource.hpp"
#include "Graphics/Renderable/SimpleMeshResource.hpp"

namespace ProjetNihil
{
	using namespace EmEn;
	using namespace EmEn::Libs;
	using namespace EmEn::Graphics;

	const size_t Application::ClassUID{getClassUID(ClassId)};

	Application::Application (int argc, char * * argv) noexcept
		: Core(argc, argv, ApplicationName, {ApplicationVersionMajor, ApplicationVersionMinor, ApplicationVersionPatch}, ApplicationOrganization, ApplicationDomain)
	{
		/* Register shortcuts. */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
	}

#if IS_WINDOWS
	Application::Application (int argc, wchar_t * * wargv) noexcept
		: Core(argc, wargv, ApplicationName, {ApplicationVersionMajor, ApplicationVersionMinor, ApplicationVersionPatch}, ApplicationOrganization, ApplicationDomain)
	{
		/* Register shortcuts. */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
	}
#endif

	bool
	Application::onBeforeSecondaryServicesInitialization () noexcept
	{
		/* NOTE: At this moment, the core have initialized primary services like arguments, file system, settings...
		 * All usable by a call to "this->primaryServices()". There will be no window, graphics renderer etc.
		 * If this function returns "true", the engine will properly stop the initialization. */

		m_useStaticLighting = this->primaryServices().settings().get< bool >(UseStaticLightingKey, DefaultUseStaticLighting);

		/* We let the engine continuing the initialization. */
		return false;
	}

	bool
	Application::onStart () noexcept
	{
		/* NOTE: Here the engine is fully initialized.
		 * This is where the user application can begin its own initialization. */

		auto & resourceManager = this->resourceManager();

		/* NOTE: Get the default skybox. */
		const auto defaultSkyBox = resourceManager.container< Renderable::SkyBoxResource >()->getDefaultResource();

		/* NOTE: Get the default scene area, or the ground. */
		const auto defaultSceneArea = resourceManager.container< Renderable::BasicFloorResource >()->getDefaultResource();

		/* NOTE: Create the new scene. */
		const auto newScene = this->sceneManager().newScene(
			"EmptyScene",
			1000.0F,
			defaultSkyBox,
			defaultSceneArea,
			nullptr
		);

		/* NOTE: Create a camera inside the scene. */
		{
			const auto sceneNode = newScene->root()->createChild("TheCameraNode", 0, Math::Vector< 3, float >{-512.0F, -80.0F, 256.0F});
			sceneNode->newCamera(true, true, "TheCamera");
			sceneNode->lookAt(Math::Vector< 3, float >{0.0F, -75.0F, 0.0F}, false);
		}

		/* NOTE: Create a directional light. */
		if ( m_useStaticLighting )
		{
			/* FIXME: This is broken for now. */
			newScene->lightSet().enableAsStaticLighting();
		}
		else
		{
			const auto sceneNode = newScene->root()->createChild("TheSunNode", 0, Math::Vector< 3, float >{-750.0F, -1000.0F, 250.0F});
			/* TODO: In an advanced version, this should be automatized. */
			sceneNode->lookAt(Math::Vector< 3, float >::origin(), true);

			const auto component = sceneNode->newDirectionalLight(0, "TheSun");
			component->setColor(PixelFactory::White);
			component->setIntensity(1.2F);

			/* TODO: In an advanced version, this should be automatized at light addition. */
			newScene->lightSet().enable();
		}

		/* NOTE: Create a cube. */
		{
			const auto cubeResource = resourceManager.container< Renderable::SimpleMeshResource >()->getOrCreateResource("TheCubeMesh", [&resourceManager] (Renderable::SimpleMeshResource & newMesh) {
				const Geometry::ResourceGenerator generator{resourceManager, Geometry::EnableNormal | Geometry::EnableVertexColor};

				const auto geometryResource = generator.cube(100.0F, "TheCubeGeometry");

				if ( geometryResource == nullptr )
				{
					return false;
				}

				const auto materialResource = resourceManager.container< Material::BasicResource >()->getDefaultResource();

				if ( materialResource == nullptr )
				{
				return false;
				}

				return newMesh.load(geometryResource, materialResource);
		   });

			const auto sceneNode = newScene->root()->createChild("TheCubeNode", 0, Math::Vector< 3, float >{0.0F, -75.0F, 0.0F});
			sceneNode->newVisual(cubeResource, false, true, "TheCube");

			m_cubeNode = sceneNode;
		}

		/* NOTE: Enable the new scene. */
		this->sceneManager().enableScene(newScene);

		/* Tells Core we are OK to run the application. */
		return true;
	}

	void
	Application::onResume () noexcept
	{
		Tracer::info(ClassId, "The application resumed !");
	}

	void
	Application::onProcessLogics (size_t engineCycle) noexcept
	{
		if ( const auto cubeNode = m_cubeNode.lock() )
		{
			cubeNode->yaw(0.01F, Math::TransformSpace::World);
		}
	}

	void
	Application::onPause () noexcept
	{
		Tracer::info(ClassId, "The application is paused !");
	}

	void
	Application::onStop () noexcept
	{
		Tracer::info(ClassId, "Stopping the application ...");
	}

	bool
	Application::onAppKeyPress (int32_t key, int32_t scancode, int32_t modifiers, bool repeat) noexcept
	{
		/* NOTE: Tells Core we don't consume the event. */
		return false;
	}

	bool
	Application::onAppKeyRelease (int32_t key, int32_t scancode, int32_t modifiers) noexcept
	{
		using namespace PlatformSpecific::Desktop;

		if ( key == Input::Key::KeyF1 )
		{
			std::stringstream message;

			message <<
				this->identification().applicationId() << "\n\n"
				"This application is a starting point to use Emeraude-Engine." "\n\n"
				"Engine ID: " << this->identification().engineId();

			Dialog::Message dialog{
				"About",
				message.str(),
				Dialog::ButtonLayout::OK,
				Dialog::MessageType::Info
			};

			dialog.execute(&this->window());

			/* NOTE: Tells Core we consumed the event. */
			return true;
		}

		/* NOTE: Tells Core we don't consume the event. */
		return false;
	}

	bool
	Application::onAppCharacterType (uint32_t unicode) noexcept
	{
		/* NOTE: Tells Core we don't consume the event. */
		return false;
	}

	bool
	Application::onAppNotification (const ObservableTrait * observable, int notificationCode, const std::any & data) noexcept
	{
		/* Check the observable here ... */

		/* NOTE: Tells Core we don't know this observable, so we 'forget it'.
		 * Set 'true' to keep the observable for further information. */
		TraceDebug{ClassId} <<
			"Received an unhandled notification (Code:" << notificationCode << ") from observable '" << whoIs(observable->classUID()) << "' (UID:" << observable->classUID() << ")  ! "
			"Forgetting it ...";

		return false;
	}

	void
	Application::onOpenFiles (const std::vector< std::filesystem::path > & filepaths) noexcept
	{
		for ( auto & filepath : filepaths )
		{
			TraceInfo{ClassId} << "Trying to do something with the file : " << filepath;
		}
	}
}
