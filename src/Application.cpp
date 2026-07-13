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
#include <numbers>

/* Local inclusions. */
#include "PlatformSpecific/Desktop/Dialog/Message.hpp"
#include "Animations/Sequence.hpp"
#include "Graphics/Material/PBRResource.hpp"
#include "Graphics/Geometry/ResourceGenerator.hpp"
#include "Graphics/Renderable/SkyBoxResource.hpp"
#include "Graphics/Renderable/BasicGroundResource.hpp"
#include "Graphics/Renderable/MeshResource.hpp"
#include "Scenes/Component/Camera.hpp"
#include "Scenes/Toolkit.hpp"
#include "Graphics/Effects/Lens/ChromaticAberration.hpp"
#include "Graphics/Effects/Lens/ColorGrading.hpp"
#include "Graphics/Effects/Lens/Vignetting.hpp"
#include "Graphics/Effects/Lens/FilmGrain.hpp"
#include "Graphics/Effects/Framebuffer/VolumetricLight.hpp"
#include "Graphics/PostProcessStack.hpp"
#include "Audio/MusicResource.hpp"
#include "ApplicationSettingKeys.hpp"

namespace ProjetNihil
{
	using namespace EmEn;
	using namespace EmEn::Base;
	using namespace EmEn::Base::PixelFactory;
	using namespace EmEn::Graphics;
	using namespace EmEn::Graphics::Effects;
	using namespace EmEn::Scenes;

	Application::Application (int argc, char * * argv) noexcept
		: Core{argc, argv, ApplicationName, {ApplicationVersionMajor, ApplicationVersionMinor, ApplicationVersionPatch}, ApplicationOrganization, ApplicationDomain}
	{
		/* Register shortcuts. */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
		m_applicationHelp.registerShortcut("Toggle the artistic post-processing effects.", Input::Key::KeySpace);
	}

#if IS_WINDOWS
	Application::Application (int argc, wchar_t * * wargv) noexcept
		: Core{argc, wargv, ApplicationName, {ApplicationVersionMajor, ApplicationVersionMinor, ApplicationVersionPatch}, ApplicationOrganization, ApplicationDomain}
	{
		/* Register shortcuts. */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
		m_applicationHelp.registerShortcut("Toggle the artistic post-processing effects.", Input::Key::KeySpace);
	}
#endif

	bool
	Application::onBeforeCoreSecondaryServicesInitialization () noexcept
	{
		/* NOTE: At this moment, the core have initialized primary services like arguments, file system, settings...
		 * All usable by a call to "this->primaryServices()". There will be no window, graphics renderer etc.
		 * If this function returns "true", the engine will properly stop the initialization. */

		m_useSkyLighting = this->primaryServices().settings().getOrSetDefault< bool >(UseSkyLightingKey, DefaultUseSkyLighting);

		/* We let the engine continuing the initialization. */
		return false;
	}

	bool
	Application::onCoreStarted (const Arguments & /*arguments*/, Settings & settings) noexcept
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
					Physics::SI::meters(81.96F),
					512,
					materialResource,
					{
						/* NOTE: 'size' is a frequency in UV space - dimensionless, so it does NOT
						 * follow the scene scale. 'factor' is a displacement in WORLD units: at
						 * human scale the terrain rolls by 1.5 m, not 150 m. */
						.size = 5.0F,
						.factor = 1.5F
					}
				);
			});

		/* NOTE: Create the new scene. */
		const auto newScene = this->sceneManager().newScene(
			"EmptyScene",
			Physics::SI::meters(10.0F),
			defaultSkyBox,
			defaultSceneArea,
			nullptr
		);

		/* NOTE: Create a camera inside the scene. */
		{
			const auto sceneNode = newScene->root()->createChild("TheCameraNode", Math::CartesianFrame{-5.12F, -0.8F, 2.56F});
			sceneNode->componentBuilder< Component::Camera >("TheCamera").asPrimary().build(true);
			sceneNode->lookAt(Math::Vector< 3, float >{0.0F, -0.75F, 0.0F}, false);

			{
				constexpr auto segmentCount{16U};
				constexpr float yMax = -0.7F;
				constexpr float yMin = -3.5F;
				constexpr float yCenter = (yMax + yMin) / 2.0F;
				constexpr float yAmplitude = (yMax - yMin) / 2.0F;

				/* NOTE: Create the animation interpolation */
				const auto interpolation = std::make_shared< Animations::Sequence >(30'000);

				for ( uint32_t index = 0; index <= segmentCount; ++index )
				{
					constexpr auto radius{5.12F};
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

		/* NOTE: Every light quantity below is PHOTOMETRIC, in real-world units: an illuminance
		 * in lux for a directional light (the sun), a luminous power in lumens for a point or a
		 * spot light (what a bulb is sold as). They are not [0..1] sliders. */
		if ( m_useSkyLighting )
		{
			/* NOTE: Sky-driven lighting. The engine derives the whole scene lighting from the
			 * photometric manifest of the background: the ambient (the average sky color times
			 * its ambient illuminance, served by the baked IBL irradiance) plus one directional
			 * light per declared celestial body. It is OPT-IN: nothing happens without this call.
			 * The default skybox declares no star, so here the sky is the ONLY light source —
			 * pure image-based lighting, no explicit light, no shadow map. Give the manifest a
			 * "Stars" array and the same call would light and shadow the scene from its sun. */
			TraceInfo{ClassId} << "Using sky-driven lighting ...";

			if ( !newScene->applyBackgroundLighting({.shadowMapResolution = 4096, .shadowCoverage = 500.0F}) )
			{
				TraceError{ClassId} << "Unable to derive the lighting from the background!";
			}
		}
		else
		{
			TraceInfo{ClassId} << "Using dynamic lighting ...";

			/* NOTE: A photometric ambient stands in for the sky and the bounce light, so the
			 * shadowed sides are not pure black. A surface shadowed under a clear sky receives
			 * 10 to 20% of the direct sun. */
			newScene->lightSet().setAmbientLightColor({0.55F, 0.68F, 1.0F, 1.0F});
			newScene->lightSet().setAmbientLightIntensity(2'500.0F);

			/* NOTE: A VEILED / overcast sun, 10 000 lux - the engine's own reference value for
			 * overcast daylight. This is a deliberate lighting choice, not a scale artefact: at
			 * 100 000 lux (clear-day direct sun) the hand-authored fixtures below would be a 2%
			 * contribution and you would not see them at all. Lighting is about RATIOS, and this
			 * mode exists to show what a point light and a spotlight do. Raise this to
			 * 100'000.0F to watch the coloured lamps vanish into the sun - that is the lesson. */
			toolkit.setCursor(-7.5F, -10.0F, 2.5F);
			toolkit.generateDirectionalLight("TheSun", {1.0F, 0.95F, 0.85F, 1.0F}, 10'000.0F, 4096, 5.0F);

			/* NOTE: A point or spot light is given its LUMINOUS POWER in lumens, and its
			 * illuminance falls off as E = I/d². At human scale the distances are metres, so
			 * ordinary catalogue numbers work: these are large floodlights (80-100 klm, real
			 * fixtures) hanging 2-4 m away, which lands them around 2000-4000 lux - a good
			 * fraction of the 10 000 lux sun, so they read as clear coloured pools of light.
			 * The 'radius' is NOT the falloff: with a physical inverse-square falloff it is only
			 * a culling bound, set where the contribution becomes negligible. */

			/* Warm amber point light orbiting clockwise. */
			toolkit.setCursor(2.0F, -3.0F, 2.0F);
			{
				const auto warmLight = toolkit.generatePointLight< Node >("WarmLight", {1.0F, 0.7F, 0.3F, 1.0F}, 25.0F, 100'000.0F, 1024);
				const auto warmLightNode = warmLight.entity();

				constexpr auto segmentCount{32U};

				const auto interpolation = std::make_shared< Animations::Sequence >(20'000);

				for ( uint32_t index = 0; index <= segmentCount; ++index )
				{
					constexpr auto orbitHeight{-2.5F};
					constexpr auto orbitRadius{3.0F};
					const auto timePoint = static_cast< float >(index) / static_cast< float >(segmentCount);
					const auto angle = timePoint * (2.0F * std::numbers::pi_v< float >);

					const Math::Vector< 3, float > position{
						orbitRadius * std::cos(angle),
						orbitHeight + (0.5F * std::sin(angle * 3.0F)),
						orbitRadius * std::sin(angle)
					};

					interpolation->addKeyFrame(timePoint, Variant{position}, Math::InterpolationType::Linear);
				}

				interpolation->play();

				warmLightNode->addAnimation(Node::WorldPosition, interpolation);
			}

			/* Cool blue point light orbiting counter-clockwise. */
			toolkit.setCursor(-2.0F, -2.5F, -2.0F);
			{
				const auto coolLight = toolkit.generatePointLight< Node >("CoolLight", {0.3F, 0.5F, 1.0F, 1.0F}, 25.0F, 80'000.0F, 1024);
				const auto coolLightNode = coolLight.entity();

				constexpr auto segmentCount{32U};

				const auto interpolation = std::make_shared< Animations::Sequence >(25'000);

				for ( uint32_t index = 0; index <= segmentCount; ++index )
				{
					constexpr auto orbitHeight{-2.0F};
					constexpr auto orbitRadius{3.5F};
					const auto timePoint = static_cast< float >(index) / static_cast< float >(segmentCount);
					const auto angle = -timePoint * (2.0F * std::numbers::pi_v< float >);

					const Math::Vector< 3, float > position{
						orbitRadius * std::cos(angle),
						orbitHeight + (0.4F * std::sin(angle * 2.0F)),
						orbitRadius * std::sin(angle)
					};

					interpolation->addKeyFrame(timePoint, Variant{position}, Math::InterpolationType::Linear);
				}

				interpolation->play();

				coolLightNode->addAnimation(Node::WorldPosition, interpolation);
			}

			/* Spotlight illuminating the center stage from above. The cone concentrates the flux
			 * into ~1.14 steradian instead of 4*pi, so the same power buys roughly ten times the
			 * candela of a point light: 80 klm at 4.25 m lands near 3900 lux on the cube. */
			toolkit.setCursor(0.0F, -5.0F, 0.0F);
			toolkit.generateSpotLight("CenterSpot", {0.0F, -0.75F, 0.0F}, 25.0F, 35.0F, White, 15.0F, 80'000.0F, 2048);

			newScene->lightSet().enable();
		}

		/* NOTE: Create a cube with a porcelain material. */
		{
			const auto cubeResource = resources.container< Renderable::MeshResource >()
				->getOrCreateResource("TheCubeMesh", [&resources] (Renderable::MeshResource & meshResource) {
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
						generator.cube(1.0F, "TheCubeGeometry"),
						material
					);
			   });

			const auto sceneNode = newScene->root()->createChild("TheCubeNode", Math::CartesianFrame{0.0F, -0.75F, 0.0F});

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

			toolkit.setCursor(2.0F, -0.75F, 2.0F);

			m_goldSphere = toolkit.generateSphereInstance("GoldSphere", 0.35F, goldMaterial, false, true, 64).entity();

			/* Chrome sphere - perfect mirror. */
			const auto chromeMaterial = resources.container< Material::PBRResource >()
				->getOrCreateResource("ChromeMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({0.95F, 0.95F, 0.95F, 1.0F});
					materialResource.setRoughnessComponent(0.02F);
					materialResource.setMetalnessComponent(1.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);

					return materialResource.setManualLoadSuccess(true);
				});

			toolkit.setCursor(-2.0F, -0.75F, 2.0F);

			m_chromeSphere = toolkit.generateSphereInstance("ChromeSphere", 0.35F, chromeMaterial, false, true, 64).entity();

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

			toolkit.setCursor(2.0F, -0.75F, -2.0F);

			m_rubySphere = toolkit.generateSphereInstance("RubySphere", 0.35F, rubyMaterial, false, true, 64).entity();

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

			toolkit.setCursor(-2.0F, -0.75F, -2.0F);

			m_sapphireSphere = toolkit.generateSphereInstance("SapphireSphere", 0.35F, sapphireMaterial, false, true, 64).entity();
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

			toolkit.setCursor(0.0F, -2.0F, 0.0F);

			const auto torusEntity = toolkit.generateRenderableInstance< Node >(
				"TheTorus",
				generator.torus(0.6F, 0.12F, 64, 32, "TorusGeometry"),
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

		/* =====================================================================
		 * NOTE: Post-processing. There are THREE distinct layers here, and telling them
		 * apart is the whole lesson of this block:
		 *
		 *  1. The SENSOR (tone mapping). The scene is lit in real photometric units — the sun
		 *     above is 10 000 lux, the sky 8000 nits — so the renderer produces physical
		 *     radiance, not [0..1] colors. A screen cannot show that. The tone mapper is what
		 *     maps the one onto the other, exactly like the sensor of a real camera, and it is
		 *     therefore NOT optional and NOT an "effect": without it the raw radiance reaches
		 *     an LDR swap-chain and a daylight scene comes out pure white. It is declared on
		 *     the CAMERA (the photographic authority) and stays on in both modes below.
		 *
		 *  2. The OPTICS (bloom). Veiling glare is light scattering inside the lens, so it
		 *     applies to the image the optics have already formed — after the defocus, before
		 *     the sensor. Declaring it on the camera is what puts it at that place in the
		 *     chain; the engine materializes it there for us. Its threshold is an absolute
		 *     scene luminance in nits, and its intensity the FRACTION of energy the glass
		 *     scatters (a clean lens: a few percent), not an artistic gain.
		 *
		 *  3. The ARTISTIC pass (god rays, grading, vignetting, grain). This is the only
		 *     layer that is a matter of taste, and the only one the KeySpace shortcut toggles.
		 * ===================================================================== */
		{
			auto & renderer = this->graphicsRenderer();

			/* Multi-pass effects → PostProcessStack owned by Scene. */
			{
				const auto & windowState = this->window().state();

				auto stack = std::make_unique< PostProcessStack >();

				/* Volumetric light (God Rays) matching the sun direction. Kept as an
				 * application-authored scene effect: it is a property of the ATMOSPHERE, it
				 * needs the light set, and it is not part of the camera body. */
				m_volumetricLight = std::make_shared< Framebuffer::VolumetricLight >(renderer, Framebuffer::VolumetricLight::Parameters{
					.density = 0.8F,
					.decay = 0.98F,
					.exposure = 0.15F,
					.numSamples = 64,
					.depthThreshold = 0.9999F
				});

				stack->addEffect(m_volumetricLight);

				if ( !stack->createAll(windowState.framebufferWidth, windowState.framebufferHeight) )
				{
					TraceError{ClassId} << "Unable to create the post-process stack!";
				}

				newScene->setPostProcessStack(std::move(stack));
			}

			if ( const auto cameraNode = m_cameraNode.lock() )
			{
				if ( const auto camera = cameraNode->getComponent< Component::Camera >("TheCamera") )
				{
					/* 1. The sensor: HDR rendering with the auto-exposure metering the scene. */
					camera->enableHDR(true);
					camera->setExposureCompensation(-0.3F);

					/* 2. The optics: the sky sits around 8000 nits, so glare only on what is
					 * brighter than that — the sun-lit highlights of the metals and the gems. */
					camera->setBloomThreshold(9'000.0F);
					camera->setBloomIntensity(0.04F);
					camera->enableBloom(true);

					/* 3. The artistic pass, single-pass lens effects. Kept so KeySpace can put
					 * them back: the composite shader is generated from the camera's effect
					 * LIST, so disabling one means taking it off the camera (the program cache
					 * makes putting the same set back a hit, not a rebuild). */
					{
						/* Subtle radial chromatic aberration (lens fringing). */
						auto chromaticAberration = std::make_shared< Lens::ChromaticAberration >(0.003F);
						chromaticAberration->enableRadial(true);
						m_lensEffects.emplace_back(std::move(chromaticAberration));

						/* Gentle warm color grading. */
						auto colorGrading = std::make_shared< Lens::ColorGrading >();
						colorGrading->setSaturation(1.1F);
						colorGrading->setHue(0.06F);
						colorGrading->setContrast(1.1F);
						colorGrading->setBrightness(0.02F);
						colorGrading->setGamma(1.05F);
						m_lensEffects.emplace_back(std::move(colorGrading));

						/* Light cinematic vignetting. */
						auto vignetting = std::make_shared< Lens::Vignetting >(0.4F);
						vignetting->setRadius(0.45F);
						vignetting->setSoftness(0.55F);
						m_lensEffects.emplace_back(std::move(vignetting));

						/* Barely perceptible film grain. */
						auto filmGrain = std::make_shared< Lens::FilmGrain >(0.05F);
						filmGrain->setSize(1.0F);
						m_lensEffects.emplace_back(std::move(filmGrain));
					}
				}
			}

			/* NOTE: The renderer master switch is a global kill-switch — it would take the tone
			 * mapping down with everything else. It defaults to ON and an application has no
			 * reason to touch it: a scene with nothing to run pays nothing anyway. Our own
			 * toggle below is selective, which is why the image stays viewable in both states. */
			renderer.postProcessor().enable(true);

			this->applyEffectsState();
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
			const auto time = static_cast< float >(engineCycle) * WorldPhysicsUpdateCycleDurationS< float >;

			struct BobParams {
				float period;
				float amplitude;
				float phaseOffset;
				float baseY;
			};

			constexpr std::array< BobParams, 4 > bobbing{{
				/* NOTE: The periods are SECONDS and do not scale; the amplitudes and the base
				 * height are world units, so they do: the spheres bob by 20-30 cm, not 30 m. */
				/* Gold: slow, large. */
				{
					.period = 5.0F,
					.amplitude = 0.25F,
					.phaseOffset = 0.0F,
					.baseY = -0.75F
				},
				/* Chrome: medium, offset pi/2. */
				{
					.period = 3.5F,
					.amplitude = 0.18F,
					.phaseOffset = std::numbers::pi_v< float > * 0.5F,
					.baseY = -0.75F
				},
				/* Ruby: very slow, large, offset pi. */
				{
					.period = 7.0F,
					.amplitude = 0.30F,
					.phaseOffset = std::numbers::pi_v< float >,
					.baseY = -0.75F
				},
				/* Sapphire: medium, offset pi/4. */
				{
					.period = 4.2F,
					.amplitude = 0.20F,
					.phaseOffset = std::numbers::pi_v< float > * 0.25F,
					.baseY = -0.75F
				}
			}};

			auto applySphereY = [&time] (const std::weak_ptr< StaticEntity > & weakSphere, const BobParams & params) {
				if ( const auto sphere = weakSphere.lock() )
				{
					const auto y = params.baseY + (params.amplitude * std::sin((2.0F * std::numbers::pi_v< float > * time / params.period) + params.phaseOffset));

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
			cameraNode->lookAt({0.0F, -0.75F, 0.0F}, false);
		}
	}

	bool
	Application::onCoreKeyRelease (int32_t key, int32_t /*scancode*/, int32_t /*modifiers*/) noexcept
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

			dialog.execute(this->window());

			/* NOTE: Tells Core we consumed the event. */
			return true;
		}

		if ( key == Input::Key::KeySpace )
		{
			m_effectsEnabled = !m_effectsEnabled;

			this->applyEffectsState();

			TraceInfo{ClassId} << "Artistic effects " << ( m_effectsEnabled ? "enabled" : "disabled" ) << ".";

			return true;
		}

		/* NOTE: Tells Core we don't consume the event. */
		return false;
	}

	void
	Application::applyEffectsState () const noexcept
	{
		/* NOTE: Only the ARTISTIC layer moves. The tone mapping (the sensor) and the bloom (the
		 * optics) stay exactly as they are, so both states are a photographically valid image:
		 * "off" is the raw rendered frame, not a broken one. That is the whole point of the
		 * comparison - press KeySpace and you see what the grading adds, not what the exposure
		 * pipeline was hiding. */

		/* The god rays live in the scene chain, whose executor honours the per-effect flag: a
		 * plain enable/disable, no pipeline rebuild. */
		if ( m_volumetricLight != nullptr )
		{
			m_volumetricLight->enable(m_effectsEnabled);
		}

		/* The lens effects are single-pass and are compiled INTO the composite shader, so the
		 * camera's effect list is the program cache key: the only way to turn one off is to take
		 * it off the list. */
		if ( const auto cameraNode = m_cameraNode.lock() )
		{
			if ( const auto camera = cameraNode->getComponent< Component::Camera >("TheCamera") )
			{
				if ( m_effectsEnabled )
				{
					for ( const auto & lensEffect : m_lensEffects )
					{
						camera->addLensEffect(lensEffect);
					}
				}
				else
				{
					camera->clearLensEffects();
				}
			}
		}
	}
}
