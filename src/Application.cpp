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
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

/* Local inclusions. */
#include "PlatformSpecific/Desktop/Dialog/Message.hpp"
#include "Animations/Sequence.hpp"
#include "Graphics/Material/StandardResource.hpp"
#include "Graphics/Geometry/ResourceGenerator.hpp"
#include "Graphics/Renderable/SkyBoxResource.hpp"
#include "Graphics/TextureResource/TextureCubemap.hpp"
#include "Graphics/CelestialBody.hpp"
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
#include "Audio/SoundResource.hpp"
#include "WaveFactory/Synthesizer.hpp"
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
		/* Register the application shortcuts (F1 shows them at runtime). */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
		m_applicationHelp.registerShortcut("Cycle the photographic look (golden hour, silver noir, raw sensor).", Input::Key::KeySpace);
	}

#if IS_WINDOWS
	Application::Application (int argc, wchar_t * * wargv) noexcept
		: Core{argc, wargv, ApplicationName, {ApplicationVersionMajor, ApplicationVersionMinor, ApplicationVersionPatch}, ApplicationOrganization, ApplicationDomain}
	{
		/* Register the application shortcuts (F1 shows them at runtime). */
		m_applicationHelp.registerShortcut("Show an informative dialog box.", Input::Key::KeyF1);
		m_applicationHelp.registerShortcut("Cycle the photographic look (golden hour, silver noir, raw sensor).", Input::Key::KeySpace);
	}
#endif

	bool
	Application::onBeforeCoreSecondaryServicesInitialization () noexcept
	{
		/* This hook runs before the window and the renderer exist: it is the right place
		 * to read settings. Returning "true" would abort the engine initialization. */
		m_useSkyLighting = this->primaryServices().settings().getOrSetDefault< bool >(UseSkyLightingKey, DefaultUseSkyLighting);

		return false;
	}

	bool
	Application::onCoreStarted (const Arguments & /*arguments*/, Settings & settings) noexcept
	{
		/* The engine is fully initialized: this hook is where the application builds its
		 * scene. Two conventions to keep in mind while reading:
		 *  - The world is right-handed and Y-up, and one unit is one metre.
		 *  - Light values are real photometric units: lux for a sun, lumens for a fixture,
		 *    nits (cd/m²) for what the sky emits. No [0..1] magic sliders. */

		/* The resource manager gives access to every resource type: textures, meshes,
		 * materials, skyboxes... A resource is fetched by name and created on first use. */
		auto & resources = this->resourceManager();

		/* === The sky ====================================================================
		 * A skybox is more than a picture: it carries a small photometric manifest — how
		 * bright the sky is, its average color, which celestial bodies it holds — that the
		 * engine can turn into actual scene lighting (see applyBackgroundLighting() below).
		 * We build a dusk sky from the engine's default sunset cubemap. */
		const auto duskSkyBox = resources.container< Renderable::SkyBoxResource >()
			->getOrCreateResource("DemoDuskSky", [&resources] (Renderable::SkyBoxResource & newSkyBox) {
				/* 1000 nits is a dusk sky (a clear day is ~8000, heavy overcast ~2000). */
				constexpr auto DuskSkyLuminance{1'000.0F};

				const auto cubemap = resources.container< TextureResource::TextureCubemap >()->getDefaultResource();

				/* A sky emits light instead of receiving it: an unlit, self-illuminated
				 * material whose cubemap texels are emitted at the luminance above. */
				const auto material = resources.container< Material::StandardResource >()
					->getOrCreateResource("DemoDuskSkyMaterial", [cubemap] (auto & materialResource) {
						if ( !materialResource.setAlbedoComponent(cubemap) )
						{
							return false;
						}

						materialResource.setAutoIlluminationComponent(1.0F);
						materialResource.setEmissiveStrength(DuskSkyLuminance);
						materialResource.enableUnlit();

						return materialResource.setManualLoadSuccess(true);
					}, Material::ComputePrimaryTextureCoordinates | Material::PrimaryTextureCoordinatesUses3D);

				if ( !newSkyBox.load(std::static_pointer_cast< Material::Interface >(material)) )
				{
					return false;
				}

				/* Declare the photometry: the luminance scales the image-based lighting,
				 * the cubemap feeds the reflections, and the ambient is the sky's average
				 * color at a dusk-shade intensity. */
				newSkyBox.setLuminance(DuskSkyLuminance);
				newSkyBox.setEnvironmentCubemap(cubemap);
				newSkyBox.setAverageColor({0.62F, 0.42F, 0.38F, 1.0F});
				newSkyBox.setAmbientIlluminance(1'200.0F);

				/* Declare a setting sun: the direction points toward it, the illuminance is
				 * what a surface facing it receives, and the color comes from a color
				 * temperature in kelvins, like a real bulb. */
				CelestialBody sun;
				sun.setDirection({-0.80F, 0.25F, 0.45F});
				sun.setIlluminance(6'000.0F);
				sun.setTemperature(2'700.0F);
				newSkyBox.addStar(sun);

				return true;
			});

		/* === The terrain ================================================================
		 * An 82 m Perlin-noise ground wearing a polished dark-gemstone material. It is
		 * loaded synchronously ("...Sync") because the placements below will immediately
		 * ask it for its height. */
		const auto demoGround = resources.container< Renderable::BasicGroundResource >()
			->getOrCreateResourceSync("DemoBasicGround", [&resources] (Renderable::BasicGroundResource & newResource) {
				/* Near-black polished obsidian: the surface mostly mirrors the dusk sky. */
				const auto materialResource = resources.container< Material::StandardResource >()
					->getOrCreateResource("DemoBasicGroundMaterial", [] (auto & newMaterial) {
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
						/* 'size' is the noise frequency; 'factor' is the bump height (m). */
						.size = 5.0F,
						.factor = 1.5F
					}
				);
			});

		/* === The scene ==================================================================
		 * A scene is created from a name, a boundary, a background and a ground. */
		const auto newScene = this->sceneManager().newScene(
			"EmptyScene",
			Physics::SI::meters(10.0F),
			duskSkyBox,
			demoGround,
			nullptr
		);

		/* The terrain rolls by ±1.5 m, so nothing should sit at an absolute height. The
		 * scene exposes the ground the physics uses: getLevelAt(x, z, deltaY) returns the
		 * point deltaY metres above the local ground. Every placement below uses it. */
		const auto groundLevel = newScene->groundLevel();

		/* The stage center: where the cube rests, 0.75 m above its patch of ground. */
		m_stageCenter = groundLevel->getLevelAt(0.0F, 0.0F, 0.75F);

		/* === The camera =================================================================
		 * A camera component on a scene node. The node follows a keyframed orbit around
		 * the stage (30 s per revolution), sweeping between low and high viewpoints. */
		{
			const auto sceneNode = newScene->root()->createChild("TheCameraNode", Math::CartesianFrame{-5.12F, 0.8F, 2.56F});
			sceneNode->componentBuilder< Component::Camera >("TheCamera").asPrimary().build(true);
			sceneNode->lookAt(m_stageCenter, false);

			{
				constexpr auto segmentCount{16U};
				constexpr float heightMin = 0.7F;
				constexpr float heightMax = 3.5F;
				constexpr float heightCenter = (heightMax + heightMin) / 2.0F;
				constexpr float heightAmplitude = (heightMax - heightMin) / 2.0F;

				/* An animation is a sequence of keyframes; the engine interpolates between
				 * them and loops. This one drives the node's world position. */
				const auto interpolation = std::make_shared< Animations::Sequence >(30'000);

				for ( uint32_t index = 0; index <= segmentCount; ++index )
				{
					constexpr auto radius{5.12F};
					const auto timePoint = static_cast< float >(index) / static_cast< float >(segmentCount);

					const auto currentAngle = timePoint * (2.0F * std::numbers::pi_v< float >);

					Math::Vector< 3, float > position{
						radius * std::cos(currentAngle),
						heightCenter - (heightAmplitude * std::cos(currentAngle * 2.0F)),
						radius * std::sin(currentAngle)
					};

					/* Never let the orbit dive below the terrain. */
					const auto floorY = groundLevel->getLevelAt(position[0], position[2], 0.5F)[1];
					position[1] = std::max(position[1], floorY);

					interpolation->addKeyFrame(timePoint, Variant{position}, Math::InterpolationType::Linear);
				}

				interpolation->play();

				sceneNode->addAnimation(Node::WorldPosition, interpolation);
			}

			m_cameraNode = sceneNode;
		}

		/* The Toolkit is a helper that creates common scene content (lights, primitive
		 * shapes...) in one call, at the position of its cursor. */
		Toolkit toolkit{settings, resources, newScene};

		/* === The lighting ===============================================================
		 * Two ways to light a scene, selected by the App/UseSkyLighting setting. */
		if ( m_useSkyLighting )
		{
			/* Sky-driven (the default): one call and the engine derives the whole lighting
			 * from the sky's manifest — the warm ambient AND a directional sun with its
			 * shadow map. Zero further authoring. The options are only a performance
			 * budget: the shadow map resolution and how many metres it covers. */
			TraceInfo{ClassId} << "Using sky-driven lighting ...";

			if ( !newScene->applyBackgroundLighting({.shadowMapResolution = 4096, .shadowCoverage = 100.0F}) )
			{
				TraceError{ClassId} << "Unable to derive the lighting from the background!";
			}
		}
		else
		{
			/* Hand-authored: the application places every light itself. */
			TraceInfo{ClassId} << "Using dynamic lighting ...";

			/* An ambient so the shadowed sides are not pure black: warm rose, like the
			 * dusk sky overhead. */
			newScene->lightSet().setAmbientLightColor({1.0F, 0.62F, 0.45F, 1.0F});
			newScene->lightSet().setAmbientLightIntensity(2'000.0F);

			/* A low setting sun (the light shines from the cursor toward the origin).
			 * 6000 lux keeps it gentle on purpose: raise it to 100'000 (a clear day) and
			 * watch the colored lamps below disappear — lighting is about ratios. */
			toolkit.setCursor(-8.0F, 2.0F, 4.5F);
			toolkit.generateDirectionalLight("TheSun", {1.0F, 0.60F, 0.35F, 1.0F}, 6'000.0F, 4096, 5.0F);

			/* Helper: a circular orbit that waves up and down and never dips into a
			 * terrain bump. */
			const auto createOrbit = [&groundLevel] (float radius, float height, float waveAmplitude, float waveFrequency, uint32_t durationMs, bool reverse) {
				constexpr auto segmentCount{32U};

				const auto interpolation = std::make_shared< Animations::Sequence >(durationMs);

				for ( uint32_t index = 0; index <= segmentCount; ++index )
				{
					const auto timePoint = static_cast< float >(index) / static_cast< float >(segmentCount);
					const auto angle = (reverse ? -1.0F : 1.0F) * timePoint * (2.0F * std::numbers::pi_v< float >);

					Math::Vector< 3, float > position{
						radius * std::cos(angle),
						height - (waveAmplitude * std::sin(angle * waveFrequency)),
						radius * std::sin(angle)
					};

					const auto floorY = groundLevel->getLevelAt(position[0], position[2], 0.3F)[1];
					position[1] = std::max(position[1], floorY);

					interpolation->addKeyFrame(timePoint, Variant{position}, Math::InterpolationType::Linear);
				}

				interpolation->play();

				return interpolation;
			};

			/* Two orbiting floodlights. A point light is given its power in LUMENS, like
			 * a real catalogue fixture, and naturally falls off with distance; the 25 m
			 * radius is only a culling bound, the last parameter a shadow map size. */
			toolkit.setCursor(2.0F, 3.0F, 2.0F);
			toolkit.generatePointLight< Node >("WarmLight", {1.0F, 0.7F, 0.3F, 1.0F}, 25.0F, 100'000.0F, 1024)
				.entity()->addAnimation(Node::WorldPosition, createOrbit(3.0F, 2.5F, 0.5F, 3.0F, 20'000, false));

			toolkit.setCursor(-2.0F, 2.5F, -2.0F);
			toolkit.generatePointLight< Node >("CoolLight", {0.3F, 0.5F, 1.0F, 1.0F}, 25.0F, 80'000.0F, 1024)
				.entity()->addAnimation(Node::WorldPosition, createOrbit(3.5F, 2.0F, 0.4F, 2.0F, 25'000, true));

			/* A white spotlight aimed at the stage from 5 m above it. */
			toolkit.setCursor(groundLevel->getLevelAt(0.0F, 0.0F, 5.0F));
			toolkit.generateSpotLight("CenterSpot", m_stageCenter, 25.0F, 35.0F, White, 15.0F, 80'000.0F, 2048);

			newScene->lightSet().enable();
		}

		/* === The stage decorations ======================================================
		 * Renderables are a geometry plus a material; materials are assembled from simple
		 * components: albedo, roughness, metalness, clear coat, subsurface, iridescence...
		 * Every placement asks the terrain for its local height. */

		/* A glazed porcelain cube, resting at the stage center. The values stay modest on
		 * purpose: a white material easily overexposes at dusk. */
		{
			const auto cubeResource = resources.container< Renderable::MeshResource >()
				->getOrCreateResource("TheCubeMesh", [&resources] (Renderable::MeshResource & meshResource) {
					const Geometry::ResourceGenerator generator{resources, Geometry::EnableTangentSpace | Geometry::EnablePrimaryTextureCoordinates};

					const auto material = resources.container< Material::StandardResource >()
						->getOrCreateResource("TheCubeMaterial", [] (auto & materialResource) {
							materialResource.setAlbedoComponent({0.95F, 0.93F, 0.88F, 1.0F});
							materialResource.setRoughnessComponent(0.15F);
							materialResource.setMetalnessComponent(0.0F);
							materialResource.setReflectionComponentFromEnvironmentCubemap(0.35F);
							materialResource.setClearCoatComponent(1.0F, 0.02F);
							materialResource.setSubsurfaceComponent(0.15F, 1.0F, {0.95F, 0.90F, 0.85F, 1.0F});
							materialResource.setSpecularComponent(1.0F, {1.0F, 0.98F, 0.95F, 1.0F});

							return materialResource.setManualLoadSuccess(true);
						});

					return meshResource.load(
						generator.cube(1.0F, "TheCubeGeometry"),
						material
					);
			   });

			const auto sceneNode = newScene->root()->createChild("TheCubeNode", Math::CartesianFrame{m_stageCenter});

			sceneNode->componentBuilder< Component::Visual >("TheCube")
				.setup([] (auto & component) {
					component.getRenderableInstance()->enableLighting();
				}).build(cubeResource);

			m_cubeNode = sceneNode;
		}

		/* Four material showcases bobbing above their own patch of ground. */
		{
			/* 0.75 m of rest height = the radius (0.35) + the largest bob amplitude (0.30)
			 * + some clearance: the bob never touches the ground. */
			constexpr auto SphereRestAltitude{0.75F};

			/* Helper: put a sphere above the local ground and remember its rest height for
			 * the bobbing animation in onCoreProcessLogics(). */
			const auto placeSphere = [&] (const char * name, float x, float z, const std::shared_ptr< Material::StandardResource > & material, size_t index) {
				const auto position = groundLevel->getLevelAt(x, z, SphereRestAltitude);

				toolkit.setCursor(position);
				m_sphereBaseHeights[index] = position[1];

				return toolkit.generateSphereInstance(name, 0.35F, material, false, true, 64).entity();
			};

			/* Gold - polished brushed metal. */
			const auto goldMaterial = resources.container< Material::StandardResource >()
				->getOrCreateResource("GoldMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({1.0F, 0.86F, 0.57F, 1.0F});
					materialResource.setRoughnessComponent(0.2F);
					materialResource.setMetalnessComponent(1.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);
					materialResource.setAnisotropyComponent(0.3F);

					return materialResource.setManualLoadSuccess(true);
				});

			m_goldSphere = placeSphere("GoldSphere", 2.0F, 2.0F, goldMaterial, 0);

			/* Chrome - a perfect mirror. */
			const auto chromeMaterial = resources.container< Material::StandardResource >()
				->getOrCreateResource("ChromeMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({0.95F, 0.95F, 0.95F, 1.0F});
					materialResource.setRoughnessComponent(0.02F);
					materialResource.setMetalnessComponent(1.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);

					return materialResource.setManualLoadSuccess(true);
				});

			m_chromeSphere = placeSphere("ChromeSphere", -2.0F, 2.0F, chromeMaterial, 1);

			/* Ruby - a translucent gemstone (subsurface scattering). */
			const auto rubyMaterial = resources.container< Material::StandardResource >()
				->getOrCreateResource("RubyMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({0.6F, 0.02F, 0.02F, 1.0F});
					materialResource.setRoughnessComponent(0.05F);
					materialResource.setMetalnessComponent(0.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);
					materialResource.setClearCoatComponent(1.0F, 0.01F);
					materialResource.setSubsurfaceComponent(0.5F, 0.8F, {0.8F, 0.1F, 0.1F, 1.0F});

					return materialResource.setManualLoadSuccess(true);
				});

			m_rubySphere = placeSphere("RubySphere", 2.0F, -2.0F, rubyMaterial, 2);

			/* Sapphire - an iridescent gemstone (thin-film interference, in nanometres). */
			const auto sapphireMaterial = resources.container< Material::StandardResource >()
				->getOrCreateResource("SapphireMaterial", [] (auto & materialResource) {
					materialResource.setAlbedoComponent({0.02F, 0.05F, 0.4F, 1.0F});
					materialResource.setRoughnessComponent(0.05F);
					materialResource.setMetalnessComponent(0.0F);
					materialResource.setReflectionComponentFromEnvironmentCubemap(1.0F);
					materialResource.setClearCoatComponent(1.0F, 0.01F);
					materialResource.setIridescenceComponent(0.3F, 1.5F, 200.0F, 400.0F);

					return materialResource.setManualLoadSuccess(true);
				});

			m_sapphireSphere = placeSphere("SapphireSphere", -2.0F, -2.0F, sapphireMaterial, 3);
		}

		/* An iridescent torus floating 2 m above the stage: same recipe as the cube, but
		 * with a custom geometry from the generator. */
		{
			const auto torusMaterial = resources.container< Material::StandardResource >()
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

			toolkit.setCursor(groundLevel->getLevelAt(0.0F, 0.0F, 2.0F));

			const auto torusEntity = toolkit.generateRenderableInstance< Node >(
				"TheTorus",
				generator.torus(0.6F, 0.12F, 64, 32, "TorusGeometry"),
				std::static_pointer_cast< Material::Interface >(torusMaterial)
			);

			m_torusNode = torusEntity.entity();
		}

		/* === The audio ==================================================================
		 * Play the engine's procedurally generated music, and synthesize a little chime
		 * from scratch for the space bar: two short ascending sine notes. */
		{
			const auto music = resources.container< Audio::MusicResource >()->getDefaultResource();

			auto & trackMixer = this->audioManager().trackMixer();
			trackMixer.addToPlaylist(music);
			trackMixer.setVolume(0.5F);
			trackMixer.play();

			m_lookChime = resources.container< Audio::SoundResource >()
				->getOrCreateResource("LookChime", [this] (Audio::SoundResource & soundResource) {
					const auto frequency = this->audioManager().frequencyPlayback();
					const auto noteLength = static_cast< size_t >(frequency) / 14; /* ~70 ms per note. */

					WaveFactory::Synthesizer synth{soundResource.localData(), noteLength * 2, frequency};

					/* A first note (A5) with a short, percussive envelope... */
					synth.setRegion(0, noteLength);

					if ( !synth.sineWave(880.0F, 0.35F) || !synth.applyADSR(0.005F, 0.01F, 0.6F, 0.03F) )
					{
						return false;
					}

					/* ... then a second one (E6), a fifth above. */
					synth.setRegion(noteLength, noteLength);

					if ( !synth.sineWave(1'318.5F, 0.30F) || !synth.applyADSR(0.005F, 0.01F, 0.6F, 0.04F) )
					{
						return false;
					}

					return soundResource.setManualLoadSuccess(true);
				});
		}

		/* === Post-processing ============================================================
		 * Three layers, from mandatory to optional:
		 *  1. Tone mapping (the camera "sensor"): scales the physical radiance down to
		 *     what a screen can show. Not an effect — an HDR scene is unwatchable raw.
		 *  2. Bloom (the camera "optics"): light scattering in the lens, thresholded in
		 *     nits, a few percent of intensity.
		 *  3. The artistic effects: the only layer that is a matter of taste. The space
		 *     bar cycles three ready-made looks — see buildPhotographicLooks(). */
		{
			/* God rays are a scene-level (multi-pass) effect, so they go into the scene's
			 * post-process stack; they follow the sun direction automatically. */
			{
				const auto & windowState = this->window().state();

				auto stack = std::make_unique< PostProcessStack >();

				m_volumetricLight = std::make_shared< Framebuffer::VolumetricLight >(this->graphicsRenderer(), Framebuffer::VolumetricLight::Parameters{
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
					/* 1. The sensor: HDR rendering, auto-exposure metering the scene. */
					camera->enableHDR(true);
					camera->setExposureCompensation(-0.3F);

					/* 2. The optics: glare only above 3000 nits — the sun's specular
					 * highlights on the metals and the gems, not the porcelain. */
					camera->setBloomThreshold(3'000.0F);
					camera->setBloomIntensity(0.04F);
					camera->enableBloom(true);
				}
			}

			/* 3. The artistic layer. */
			this->buildPhotographicLooks();

			this->applyLook();
		}

		/* Enable the scene, and tell Core we are OK to run. */
		this->sceneManager().enableScene(newScene);

		return true;
	}

	void
	Application::onCoreProcessLogics (size_t engineCycle) noexcept
	{
		/* This hook runs every logic cycle, on the main thread: the place for simple,
		 * per-frame animation logic. */

		/* Spin the cube around its vertical axis. */
		if ( const auto cubeNode = m_cubeNode.lock() )
		{
			cubeNode->yaw(0.01F, Math::TransformSpace::World);
		}

		/* Tumble the torus slowly around two axes. */
		if ( const auto torusNode = m_torusNode.lock() )
		{
			torusNode->yaw(0.005F, Math::TransformSpace::World);
			torusNode->pitch(0.003F, Math::TransformSpace::Local);
		}

		/* Bob each sphere around its rest height, each with its own rhythm.
		 * Periods are seconds; amplitudes are metres. */
		{
			const auto time = static_cast< float >(engineCycle) * WorldPhysicsUpdateCycleDurationS< float >;

			struct BobParams {
				float period;
				float amplitude;
				float phaseOffset;
			};

			constexpr std::array< BobParams, 4 > bobbing{{
				/* Gold: slow, large. */
				{
					.period = 5.0F,
					.amplitude = 0.25F,
					.phaseOffset = 0.0F
				},
				/* Chrome: medium, offset pi/2. */
				{
					.period = 3.5F,
					.amplitude = 0.18F,
					.phaseOffset = std::numbers::pi_v< float > * 0.5F
				},
				/* Ruby: very slow, large, offset pi. */
				{
					.period = 7.0F,
					.amplitude = 0.30F,
					.phaseOffset = std::numbers::pi_v< float >
				},
				/* Sapphire: medium, offset pi/4. */
				{
					.period = 4.2F,
					.amplitude = 0.20F,
					.phaseOffset = std::numbers::pi_v< float > * 0.25F
				}
			}};

			auto applySphereY = [&time] (const std::weak_ptr< StaticEntity > & weakSphere, const BobParams & params, float baseY) {
				if ( const auto sphere = weakSphere.lock() )
				{
					const auto y = baseY - (params.amplitude * std::sin((2.0F * std::numbers::pi_v< float > * time / params.period) + params.phaseOffset));

					sphere->setYPosition(y, Math::TransformSpace::World);
				}
			};

			applySphereY(m_goldSphere,     bobbing[0], m_sphereBaseHeights[0]);
			applySphereY(m_chromeSphere,   bobbing[1], m_sphereBaseHeights[1]);
			applySphereY(m_rubySphere,     bobbing[2], m_sphereBaseHeights[2]);
			applySphereY(m_sapphireSphere, bobbing[3], m_sphereBaseHeights[3]);
		}

		/* Keep the camera aimed at the stage while it travels its orbit. */
		if ( const auto cameraNode = m_cameraNode.lock() )
		{
			cameraNode->lookAt(m_stageCenter, false);
		}
	}

	bool
	Application::onCoreKeyRelease (int32_t key, int32_t /*scancode*/, int32_t /*modifiers*/) noexcept
	{
		using namespace PlatformSpecific::Desktop;

		/* F1: an "about" dialog box. Returning "true" consumes the event. */
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

			return true;
		}

		/* Space bar: next photographic look. */
		if ( key == Input::Key::KeySpace )
		{
			m_lookIndex = (m_lookIndex + 1) % m_looks.size();

			this->applyLook();

			/* The synthesized chime, as an audible acknowledgment. */
			this->audioManager().play(m_lookChime, Audio::PlayMode::Once, 0.6F);

			TraceInfo{ClassId} << "Photographic look: " << m_looks.at(m_lookIndex).name << ".";

			return true;
		}

		return false;
	}

	void
	Application::buildPhotographicLooks () noexcept
	{
		/* A look only changes the artistic layer: tone mapping and bloom stay untouched,
		 * so every look remains a correctly exposed image. */

		/* "Golden hour": warm grade, subtle lens flaws, god rays. */
		{
			auto & look = m_looks[0];
			look.name = "Golden hour";
			look.volumetricLight = true;

			auto chromaticAberration = std::make_shared< Lens::ChromaticAberration >(0.003F);
			chromaticAberration->enableRadial(true);
			look.lensEffects.emplace_back(std::move(chromaticAberration));

			auto colorGrading = std::make_shared< Lens::ColorGrading >();
			colorGrading->setSaturation(1.15F);
			colorGrading->setHue(0.06F);
			colorGrading->setContrast(1.1F);
			colorGrading->setBrightness(0.02F);
			colorGrading->setGamma(1.05F);
			look.lensEffects.emplace_back(std::move(colorGrading));

			auto vignetting = std::make_shared< Lens::Vignetting >(0.4F);
			vignetting->setRadius(0.45F);
			vignetting->setSoftness(0.55F);
			look.lensEffects.emplace_back(std::move(vignetting));

			auto filmGrain = std::make_shared< Lens::FilmGrain >(0.05F);
			filmGrain->setSize(1.0F);
			look.lensEffects.emplace_back(std::move(filmGrain));
		}

		/* "Silver noir": the same scene through a monochrome, contrasty, grainy eye. */
		{
			auto & look = m_looks[1];
			look.name = "Silver noir";
			look.volumetricLight = false;

			auto colorGrading = std::make_shared< Lens::ColorGrading >();
			colorGrading->setSaturation(0.10F);
			colorGrading->setContrast(1.4F);
			colorGrading->setGamma(0.95F);
			look.lensEffects.emplace_back(std::move(colorGrading));

			auto vignetting = std::make_shared< Lens::Vignetting >(0.65F);
			vignetting->setRadius(0.4F);
			vignetting->setSoftness(0.5F);
			look.lensEffects.emplace_back(std::move(vignetting));

			auto filmGrain = std::make_shared< Lens::FilmGrain >(0.12F);
			filmGrain->setSize(1.3F);
			look.lensEffects.emplace_back(std::move(filmGrain));
		}

		/* "Raw sensor": no artistic effect at all. */
		{
			auto & look = m_looks[2];
			look.name = "Raw sensor";
			look.volumetricLight = false;
		}
	}

	void
	Application::applyLook () const noexcept
	{
		const auto & look = m_looks.at(m_lookIndex);

		/* The god rays are toggled in place in the scene's post-process stack. */
		if ( m_volumetricLight != nullptr )
		{
			m_volumetricLight->enable(look.volumetricLight);
		}

		/* The lens effects are compiled into the camera's shader, so applying a look means
		 * swapping the camera's effect list (a look already seen is a shader cache hit). */
		if ( const auto cameraNode = m_cameraNode.lock() )
		{
			if ( const auto camera = cameraNode->getComponent< Component::Camera >("TheCamera") )
			{
				camera->clearLensEffects();

				for ( const auto & lensEffect : look.lensEffects )
				{
					camera->addLensEffect(lensEffect);
				}
			}
		}
	}
}
