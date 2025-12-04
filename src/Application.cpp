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
#include "Animations/Sequence.hpp"
#include "Graphics/Geometry/ResourceGenerator.hpp"
#include "Graphics/Renderable/SkyBoxResource.hpp"
#include "Graphics/Renderable/BasicFloorResource.hpp"
#include "Graphics/Renderable/SimpleMeshResource.hpp"
#include "Scenes/Component/Camera.hpp"

namespace ProjetNihil
{
	using namespace EmEn;
	using namespace EmEn::Libs;
	using namespace EmEn::Graphics;
	using namespace EmEn::Scenes;

	Application::Application (int argc, char * * argv) noexcept
		: Core{argc, argv, ApplicationName, {ApplicationVersionMajor, ApplicationVersionMinor, ApplicationVersionPatch}, ApplicationOrganization, ApplicationDomain}
	{
		/* Register shortcuts. */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
	}

#if IS_WINDOWS
	Application::Application (int argc, wchar_t * * wargv) noexcept
		: Core{argc, wargv, ApplicationName, {ApplicationVersionMajor, ApplicationVersionMinor, ApplicationVersionPatch}, ApplicationOrganization, ApplicationDomain}
	{
		/* Register shortcuts. */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
	}
#endif

	bool
	Application::onBeforeCoreSecondaryServicesInitialization () noexcept
	{
		/* NOTE: At this moment, the core have initialized primary services like arguments, file system, settings...
		 * All usable by a call to "this->primaryServices()". There will be no window, graphics renderer etc.
		 * If this function returns "true", the engine will properly stop the initialization. */

		m_useStaticLighting = this->primaryServices().settings().getOrSetDefault< bool >(UseStaticLightingKey, DefaultUseStaticLighting);

		/* We let the engine continuing the initialization. */
		return false;
	}

	bool
	Application::onCoreStarted () noexcept
	{
		/* =====================================================================
		 * NOTE: Here the engine is fully initialized,
		 * the user application is ready to start.
		 * This is where the user application can begin its own initialization.
		 * ===================================================================== */

		/* NOTE: The resource manager is provided by Core and gave access to textures, mesh, etc. */
		auto & resourceManager = this->resourceManager();

		/* NOTE: Get the default skybox. */
		const auto defaultSkyBox = resourceManager.container< Renderable::SkyBoxResource >()->getDefaultResource();

		/* NOTE: Get the default scene area (ground). */
		//const auto defaultSceneArea = resourceManager.container< Renderable::BasicFloorResource >()->getDefaultResource();

		/* NOTE: Create a ground modified with the perlin noise algorithm. */
		const auto defaultSceneArea = resourceManager.container< Renderable::BasicFloorResource >()
			->getOrCreateResource("TheSceneArea", [&resourceManager] (Renderable::BasicFloorResource & newResource) {
				constexpr auto Boundary{4096.0F};
				const auto materialResource = resourceManager.container< Material::BasicResource >()->getDefaultResource();

				if ( !newResource.loadPerlinNoise(Boundary, 0.0F, 256, 10.0F, 200.0F, materialResource, 1.0F) )
				{
					return false;
				}

				return true;
			});

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
			const auto sceneNode = newScene->root()->createChild("TheCameraNode", Math::CartesianFrame{-512.0F, -80.0F, 256.0F});
			sceneNode->componentBuilder< Component::Camera >("TheCamera").asPrimary().build(true);
			sceneNode->lookAt(Math::Vector< 3, float >{0.0F, -75.0F, 0.0F}, false);

			{
				constexpr auto segmentCount{16U};
				constexpr auto radius{512.0F};
				constexpr float yMax = -70.0F;
				constexpr float yMin = -350.0F;
				constexpr float yCenter = (yMax + yMin) / 2.0F;
				constexpr float yAmplitude = (yMax - yMin) / 2.0F;

				/* NOTE: Create the animation interpolation */
				const auto interpolation = std::make_shared< Animations::Sequence >(30'000);

				for ( uint32_t index = 0; index <= segmentCount; ++index )
				{
					const auto timePoint = static_cast< float >(index) / static_cast< float >(segmentCount);

					const auto currentAngle = timePoint * (2.0F * std::numbers::pi_v< float >);

					const Math::Vector< 3, float > position{
						radius * std::cos(currentAngle),
						yCenter + (yAmplitude * std::cos(currentAngle * 2.0F)),
						radius * std::sin(currentAngle)
					};

					interpolation->addKeyFrame(timePoint, Variant{position}, Math::InterpolationType::Linear);
				}

				interpolation->play();

				sceneNode->addAnimation(Scenes::Node::WorldPosition, interpolation);
			}

			m_cameraNode = sceneNode;
		}

		/* NOTE: Create a directional light. */
		if ( m_useStaticLighting )
		{
			TraceInfo{ClassId} << "Using static lighting ...";

			auto & staticLighting = newScene->lightSet().enableAsStaticLighting();
			staticLighting.setAmbientParameters(PixelFactory::DarkBlue, 0.15F);
			staticLighting.setLightParameters(PixelFactory::DarkYellow, 1.5F);
			staticLighting.setAsDirectionalLight({-750.0F, -1000.0F, 250.0F});
		}
		else
		{
			TraceInfo{ClassId} << "Using dynamic lighting ...";

			const auto sceneNode = newScene->root()->createChild("TheSunNode", Math::CartesianFrame{-750.0F, -1000.0F, 250.0F});
			sceneNode->componentBuilder< Component::DirectionalLight >("TheSun")
				.setup([] (auto & component) {
					component.setColor(PixelFactory::White);
					component.setIntensity(1.2F);
				}).build();

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

			const auto sceneNode = newScene->root()->createChild("TheCubeNode", Math::CartesianFrame{0.0F, -75.0F, 0.0F});
			sceneNode->componentBuilder< Component::Visual >("TheCube").setup([] (auto & component) {
				component.getRenderableInstance()->enableLighting();
			}).build(cubeResource);

			m_cubeNode = sceneNode;
		}

		/* NOTE: Enable the new scene. */
		this->sceneManager().enableScene(newScene);

		/* Tells Core we are OK to run the application. */
		return true;
	}

	void
	Application::onCoreProcessLogics (size_t engineCycle) noexcept
	{
		if ( const auto cubeNode = m_cubeNode.lock() )
		{
			cubeNode->yaw(0.01F, Math::TransformSpace::World);
		}

		/* NOTE: Each cycle we make the camera look at the center of the scene. */
		if ( const auto cameraNode = m_cameraNode.lock() )
		{
			cameraNode->lookAt({0.0F, -75.0F, 0.0F}, false);
		}
	}

	bool
	Application::onCoreKeyRelease (int32_t key, int32_t scancode, int32_t modifiers) noexcept
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
}
