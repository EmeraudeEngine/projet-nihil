/*
 * src/Application.cpp
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

#include "Application.hpp"

/* Standard inclusions. */
#include <array>
#include <cmath>

/* Local inclusions. */
#include "PlatformSpecific/Desktop/Dialog/Message.hpp"
#include "Animations/Sequence.hpp"
#include "Graphics/Material/PBRResource.hpp"
#include "Graphics/Geometry/ResourceGenerator.hpp"
#include "Graphics/Renderable/SkyBoxResource.hpp"
#include "Graphics/Renderable/BasicGroundResource.hpp"
#include "Graphics/Renderable/SimpleMeshResource.hpp"
#include "Graphics/TextureResource/Texture2D.hpp"
#include "Scenes/Component/Camera.hpp"
#include "Scenes/Toolkit.hpp"
#include "Graphics/Effects/Lens/ChromaticAberration.hpp"
#include "Graphics/Effects/Lens/ColorGrading.hpp"
#include "Graphics/Effects/Lens/Vignetting.hpp"
#include "Graphics/Effects/Lens/FilmGrain.hpp"
#include "Graphics/Effects/Framebuffer/Bloom.hpp"
#include "Graphics/Effects/Framebuffer/VolumetricLight.hpp"
#include "Graphics/PostProcessStack.hpp"
#include "Audio/MusicResource.hpp"

namespace ProjetNihil
{
	using namespace EmEn;
	using namespace EmEn::Libs;
	using namespace EmEn::Libs::PixelFactory;
	using namespace EmEn::Graphics;
	using namespace EmEn::Scenes;

	Application::Application (int argc, char * * argv) noexcept
		: Core{argc, argv, ApplicationName, {ApplicationVersionMajor, ApplicationVersionMinor, ApplicationVersionPatch}, ApplicationOrganization, ApplicationDomain}
	{
		/* Register shortcuts. */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
		m_applicationHelp.registerShortcut("Toggle post-processing effects.", Input::Key::KeySpace);
	}

#if IS_WINDOWS
	Application::Application (int argc, wchar_t * * wargv) noexcept
		: Core{argc, wargv, ApplicationName, {ApplicationVersionMajor, ApplicationVersionMinor, ApplicationVersionPatch}, ApplicationOrganization, ApplicationDomain}
	{
		/* Register shortcuts. */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
		m_applicationHelp.registerShortcut("Toggle post-processing effects.", Input::Key::KeySpace);
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
	Application::onCoreStarted (const Arguments & arguments, Settings & settings) noexcept
	{
		/* =====================================================================
		 * NOTE: Here the engine is fully initialized,
		 * the user application is ready to start.
		 * This is where the user application can begin its own initialization.
		 * ===================================================================== */

		/* NOTE: The resource manager is provided by Core and gave access to textures, mesh, etc. */
		auto & resources = this->resourceManager();

		/* NOTE: Get the default skybox. */
		const auto defaultSkyBox = resources.container< Renderable::SkyBoxResource >()->getDefaultResource();

		/* NOTE: Create a ground with a polished precious stone material. */
		const auto defaultSceneArea = resources.container< Renderable::BasicGroundResource >()
			->getOrCreateResource("DemoBasicGround", [&resources] (Renderable::BasicGroundResource & newResource) {
				const auto materialResource = resources.container< Material::PBRResource >()
					->getOrCreateResource("DemoBasicGroundMaterial", [] (auto & newMaterial) {
						/* Polished precious stone (dark sapphire/obsidian). */
						newMaterial.setAlbedoComponent({0.005F, 0.005F, 0.015F, 1.0F});
						newMaterial.setRoughnessComponent(0.15F);
						newMaterial.setMetalnessComponent(0.0F);
						newMaterial.setReflectionComponentFromEnvironmentCubemap(0.6F);
						newMaterial.setSpecularComponent(2.0F, {0.7F, 0.8F, 1.0F, 1.0F});
						newMaterial.setIridescenceComponent(0.5F, 1.8F, 150.0F, 450.0F);
						newMaterial.setClearCoatComponent(1.0F, 0.01F);

						return newMaterial.setManualLoadSuccess(true);
					});

				return newResource.loadPerlinNoise(
					Physics::SI::kilometers(8.196F),
					512,
					materialResource,
					{5.0F, 150.0F}
				);
			});

		/* NOTE: Create the new scene. */
		const auto newScene = this->sceneManager().newScene(
			"EmptyScene",
			Physics::SI::kilometers(1.0F),
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
				constexpr float yMax = -70.0F;
				constexpr float yMin = -350.0F;
				constexpr float yCenter = (yMax + yMin) / 2.0F;
				constexpr float yAmplitude = (yMax - yMin) / 2.0F;

				/* NOTE: Create the animation interpolation */
				const auto interpolation = std::make_shared< Animations::Sequence >(30'000);

				for ( uint32_t index = 0; index <= segmentCount; ++index )
				{
					constexpr auto radius{512.0F};
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

				sceneNode->addAnimation(Node::WorldPosition, interpolation);
			}

			m_cameraNode = sceneNode;
		}

		/* NOTE: Use the Toolkit to build the scene lighting and decorations. */
		Toolkit toolkit{settings, resources, newScene};

		if ( m_useStaticLighting )
		{
			TraceInfo{ClassId} << "Using static lighting ...";

			auto & staticLighting = newScene->lightSet().enableAsStaticLighting();
			staticLighting.setAmbientParameters(DarkBlue, 0.15F);
			staticLighting.setLightParameters(DarkYellow, 1.5F);
			staticLighting.setAsDirectionalLight({-750.0F, -1000.0F, 250.0F});
		}
		else
		{
			TraceInfo{ClassId} << "Using dynamic lighting ...";

			/* Sun-light with shadow mapping. */
			toolkit.setCursor(-750.0F, -1000.0F, 250.0F);
			toolkit.generateDirectionalLight("TheSun", {1.0F, 0.95F, 0.85F, 1.0F}, 1.2F, 4096, 500.0F);

			/* Warm amber point light orbiting clockwise. */
			toolkit.setCursor(200.0F, -300.0F, 200.0F);
			{
				const auto warmLight = toolkit.generatePointLight< Node >("WarmLight", {1.0F, 0.7F, 0.3F, 1.0F}, 600.0F, 0.8F, 1024);
				const auto warmLightNode = warmLight.entity();

				constexpr auto segmentCount{32U};

				const auto interpolation = std::make_shared< Animations::Sequence >(20'000);

				for ( uint32_t index = 0; index <= segmentCount; ++index )
				{
					constexpr auto orbitHeight{-250.0F};
					constexpr auto orbitRadius{300.0F};
					const auto timePoint = static_cast< float >(index) / static_cast< float >(segmentCount);
					const auto angle = timePoint * (2.0F * std::numbers::pi_v< float >);

					const Math::Vector< 3, float > position{
						orbitRadius * std::cos(angle),
						orbitHeight + 50.0F * std::sin(angle * 3.0F),
						orbitRadius * std::sin(angle)
					};

					interpolation->addKeyFrame(timePoint, Variant{position}, Math::InterpolationType::Linear);
				}

				interpolation->play();

				warmLightNode->addAnimation(Node::WorldPosition, interpolation);
			}

			/* Cool blue point light orbiting counter-clockwise. */
			toolkit.setCursor(-200.0F, -250.0F, -200.0F);
			{
				const auto coolLight = toolkit.generatePointLight< Node >("CoolLight", {0.3F, 0.5F, 1.0F, 1.0F}, 600.0F, 0.6F, 1024);
				const auto coolLightNode = coolLight.entity();

				constexpr auto segmentCount{32U};

				const auto interpolation = std::make_shared< Animations::Sequence >(25'000);

				for ( uint32_t index = 0; index <= segmentCount; ++index )
				{
					constexpr auto orbitHeight{-200.0F};
					constexpr auto orbitRadius{350.0F};
					const auto timePoint = static_cast< float >(index) / static_cast< float >(segmentCount);
					const auto angle = -timePoint * (2.0F * std::numbers::pi_v< float >);

					const Math::Vector< 3, float > position{
						orbitRadius * std::cos(angle),
						orbitHeight + 40.0F * std::sin(angle * 2.0F),
						orbitRadius * std::sin(angle)
					};

					interpolation->addKeyFrame(timePoint, Variant{position}, Math::InterpolationType::Linear);
				}

				interpolation->play();

				coolLightNode->addAnimation(Node::WorldPosition, interpolation);
			}

			/* Spotlight illuminating the center stage from above. */
			toolkit.setCursor(0.0F, -500.0F, 0.0F);
			toolkit.generateSpotLight("CenterSpot", {0.0F, -75.0F, 0.0F}, 25.0F, 35.0F, White, 800.0F, 1.5F, 2048);

			newScene->lightSet().enable();
		}

		/* NOTE: Create a cube with a porcelain material. */
		{
			const auto cubeResource = resources.container< Renderable::SimpleMeshResource >()
				->getOrCreateResource("TheCubeMesh", [&resources] (Renderable::SimpleMeshResource & meshResource) {
					const Geometry::ResourceGenerator generator{resources, Geometry::EnableTangentSpace | Geometry::EnablePrimaryTextureCoordinates};

					const auto material = resources.container< Material::PBRResource >()
						->getOrCreateResource("TheCubeMaterial", [] (auto & materialResource) {
							/* Glazed porcelain. */
							materialResource.setAlbedoComponent({0.95F, 0.93F, 0.88F, 1.0F});
							materialResource.setRoughnessComponent(0.08F);
							materialResource.setMetalnessComponent(0.0F);
							materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);
							materialResource.setClearCoatComponent(1.0F, 0.02F);
							materialResource.setSubsurfaceComponent(0.4F, 1.0F, {0.95F, 0.90F, 0.85F, 1.0F});
							materialResource.setSpecularComponent(1.5F, {1.0F, 0.98F, 0.95F, 1.0F});

							return materialResource.setManualLoadSuccess(true);
						});

					return meshResource.load(
						generator.cube(100.0F, "TheCubeGeometry"),
						material
					);
			   });

			const auto sceneNode = newScene->root()->createChild("TheCubeNode", Math::CartesianFrame{0.0F, -75.0F, 0.0F});

			sceneNode->componentBuilder< Component::Visual >("TheCube")
				.setup([] (auto & component) {
					component.getRenderableInstance()->enableLighting();
				}).build(cubeResource);

			m_cubeNode = sceneNode;
		}

		/* NOTE: Create decorative spheres using the Toolkit (showcases the scene builder). */
		{
			/* Gold sphere - polished brushed metal. */
			const auto goldMaterial = resources.container< Material::PBRResource >()
				->getOrCreateResource("GoldMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({1.0F, 0.86F, 0.57F, 1.0F});
					materialResource.setRoughnessComponent(0.2F);
					materialResource.setMetalnessComponent(1.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);
					materialResource.setAnisotropyComponent(0.3F);

					return materialResource.setManualLoadSuccess(true);
				});

			toolkit.setCursor(200.0F, -75.0F, 200.0F);

			m_goldSphere = toolkit.generateSphereInstance("GoldSphere", 35.0F, goldMaterial, false, true, 64).entity();

			/* Chrome sphere - perfect mirror. */
			const auto chromeMaterial = resources.container< Material::PBRResource >()
				->getOrCreateResource("ChromeMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({0.95F, 0.95F, 0.95F, 1.0F});
					materialResource.setRoughnessComponent(0.02F);
					materialResource.setMetalnessComponent(1.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);

					return materialResource.setManualLoadSuccess(true);
				});

			toolkit.setCursor(-200.0F, -75.0F, 200.0F);

			m_chromeSphere = toolkit.generateSphereInstance("ChromeSphere", 35.0F, chromeMaterial, false, true, 64).entity();

			/* Ruby sphere - translucent gemstone with subsurface scattering. */
			const auto rubyMaterial = resources.container< Material::PBRResource >()
				->getOrCreateResource("RubyMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({0.6F, 0.02F, 0.02F, 1.0F});
					materialResource.setRoughnessComponent(0.05F);
					materialResource.setMetalnessComponent(0.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);
					materialResource.setClearCoatComponent(1.0F, 0.01F);
					materialResource.setSubsurfaceComponent(0.5F, 0.8F, {0.8F, 0.1F, 0.1F, 1.0F});

					return materialResource.setManualLoadSuccess(true);
				});

			toolkit.setCursor(200.0F, -75.0F, -200.0F);

			m_rubySphere = toolkit.generateSphereInstance("RubySphere", 35.0F, rubyMaterial, false, true, 64).entity();

			/* Sapphire sphere - iridescent gemstone. */
			const auto sapphireMaterial = resources.container< Material::PBRResource >()
				->getOrCreateResource("SapphireMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({0.02F, 0.05F, 0.4F, 1.0F});
					materialResource.setRoughnessComponent(0.05F);
					materialResource.setMetalnessComponent(0.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);
					materialResource.setClearCoatComponent(1.0F, 0.01F);
					materialResource.setIridescenceComponent(0.3F, 1.5F, 200.0F, 400.0F);

					return materialResource.setManualLoadSuccess(true);
				});

			toolkit.setCursor(-200.0F, -75.0F, -200.0F);

			m_sapphireSphere = toolkit.generateSphereInstance("SapphireSphere", 35.0F, sapphireMaterial, false, true, 64).entity();
		}

		/* NOTE: Create a floating torus with an iridescent material (Toolkit + custom geometry). */
		{
			const auto torusMaterial = resources.container< Material::PBRResource >()
				->getOrCreateResource("TorusMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({0.02F, 0.02F, 0.03F, 1.0F});
					materialResource.setRoughnessComponent(0.05F);
					materialResource.setMetalnessComponent(0.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);
					materialResource.setIridescenceComponent(0.8F, 2.0F, 100.0F, 500.0F);
					materialResource.setClearCoatComponent(1.0F, 0.01F);
					materialResource.setSpecularComponent(2.0F, {0.9F, 0.9F, 1.0F, 1.0F});

					return materialResource.setManualLoadSuccess(true);
				});

			const Geometry::ResourceGenerator generator{resources, Geometry::EnableTangentSpace | Geometry::EnablePrimaryTextureCoordinates};

			toolkit.setCursor(0.0F, -200.0F, 0.0F);

			const auto torusEntity = toolkit.generateRenderableInstance< Node >(
				"TheTorus",
				generator.torus(60.0F, 12.0F, 64, 32, "TorusGeometry"),
				std::static_pointer_cast< Material::Interface >(torusMaterial)
			);

			m_torusNode = torusEntity.entity();
		}

		/* NOTE: Start the parametric music (generated procedurally by the engine). */
		{
			const auto music = resources.container< Audio::MusicResource >()->getDefaultResource();

			auto & trackMixer = this->audioManager().trackMixer();
			trackMixer.addToPlaylist(music);
			trackMixer.setVolume(0.5F);
			trackMixer.play();
		}

		/* NOTE: Configure subtle post-processing effects. */
		{
			auto & renderer = this->graphicsRenderer();

			/* Multi-pass effects → PostProcessStack owned by Scene. */
			{
				const auto & windowState = this->window().state();

				auto stack = std::make_unique< PostProcessStack >();

				/* HDR Bloom (Dual Kawase). */
				auto bloom = std::make_shared< Effects::Framebuffer::Bloom >();
				bloom->setParameters({
					.threshold = 0.85F,
					.softKnee = 0.5F,
					.intensity = 0.6F,
					.spread = 1.0F
				});

				stack->addEffect(std::move(bloom));

				/* Volumetric light (God Rays) matching the sun direction. */
				auto godRays = std::make_shared< Effects::Framebuffer::VolumetricLight >();
				godRays->setLightDirection(-750.0F, -1000.0F, 250.0F);
				godRays->setLightColor(1.0F, 0.95F, 0.85F);
				godRays->setLightIntensity(1.0F);
				godRays->setParameters({
					.density = 0.8F,
					.decay = 0.98F,
					.exposure = 0.15F,
					.numSamples = 64,
					.depthThreshold = 0.9999F
				});

				stack->addEffect(std::move(godRays));

				if ( !stack->createAll(renderer, windowState.framebufferWidth, windowState.framebufferHeight) )
				{
					TraceError{ClassId} << "Unable to create the post-process stack!";
				}

				newScene->setPostProcessStack(std::move(stack));
			}

			/* Single-pass lens effects → Camera. */
			if ( const auto cameraNode = m_cameraNode.lock() )
			{
				if ( const auto camera = cameraNode->getComponent< Component::Camera >("TheCamera") )
				{
					/* Subtle radial chromatic aberration (lens fringing). */
					auto chromaticAberration = std::make_shared< Effects::Lens::ChromaticAberration >(0.003F);
					chromaticAberration->enableRadial(true);
					camera->addLensEffect(chromaticAberration);

					/* Gentle warm color grading. */
					auto colorGrading = std::make_shared< Effects::Lens::ColorGrading >();
					colorGrading->setSaturation(1.1F);
					colorGrading->setHue(0.06F);
					colorGrading->setContrast(1.1F);
					colorGrading->setBrightness(0.02F);
					colorGrading->setGamma(1.05F);
					camera->addLensEffect(colorGrading);

					/* Light cinematic vignetting. */
					auto vignetting = std::make_shared< Effects::Lens::Vignetting >(0.4F);
					vignetting->setRadius(0.45F);
					vignetting->setSoftness(0.55F);
					camera->addLensEffect(vignetting);

					/* Barely perceptible film grain. */
					auto filmGrain = std::make_shared< Effects::Lens::FilmGrain >(0.05F);
					filmGrain->setSize(1.0F);
					camera->addLensEffect(filmGrain);
				}
			}

			renderer.postProcessor().enable(false);
		}

		/* NOTE: Enable the new scene. */
		this->sceneManager().enableScene(newScene);

		/* Tells Core we are OK to run the application. */
		return true;
	}

	void
	Application::onCoreProcessLogics (size_t engineCycle) noexcept
	{
		/* NOTE: Rotate the porcelain cube around its vertical axis. */
		if ( const auto cubeNode = m_cubeNode.lock() )
		{
			cubeNode->yaw(0.01F, Math::TransformSpace::World);
		}

		/* NOTE: Rotate the torus slowly around two axes for a floating effect. */
		if ( const auto torusNode = m_torusNode.lock() )
		{
			torusNode->yaw(0.005F, Math::TransformSpace::World);
			torusNode->pitch(0.003F, Math::TransformSpace::Local);
		}

		/* NOTE: Bobbing spheres: each with different period, amplitude, and phase offset. */
		{
			const auto time = static_cast< float >(engineCycle) * EngineUpdateCycleDurationS< float >;

			struct BobParams {
				float period;
				float amplitude;
				float phaseOffset;
				float baseY;
			};

			constexpr std::array< BobParams, 4 > bobbing{{
				{5.0F,  25.0F, 0.0F,            -75.0F},  /* Gold: slow, large. */
				{3.5F,  18.0F, 1.5707963F,      -75.0F},  /* Chrome: medium, offset pi/2. */
				{7.0F,  30.0F, 3.1415927F,      -75.0F},  /* Ruby: very slow, large, offset pi. */
				{4.2F,  20.0F, 0.7853982F,      -75.0F},  /* Sapphire: medium, offset pi/4. */
			}};

			auto applySphereY = [&time] (const std::weak_ptr< StaticEntity > & weakSphere, const BobParams & params) {
				if ( const auto sphere = weakSphere.lock() )
				{
					const auto y = params.baseY + params.amplitude * std::sin(2.0F * std::numbers::pi_v< float > * time / params.period + params.phaseOffset);

					sphere->setYPosition(y, Math::TransformSpace::World);
				}
			};

			applySphereY(m_goldSphere,     bobbing[0]);
			applySphereY(m_chromeSphere,   bobbing[1]);
			applySphereY(m_rubySphere,     bobbing[2]);
			applySphereY(m_sapphireSphere, bobbing[3]);
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

		if ( key == Input::Key::KeySpace )
		{
			m_postProcessingEnabled = !m_postProcessingEnabled;

			this->graphicsRenderer().postProcessor().enable(m_postProcessingEnabled);

			TraceInfo{ClassId} << "Post-processing " << (m_postProcessingEnabled ? "enabled" : "disabled") << ".";

			return true;
		}

		/* NOTE: Tells Core we don't consume the event. */
		return false;
	}
}
