// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "exceptions.h"
#include "particles.h"
#include "util/serialize.h"

#include <sstream>

using namespace ParticleParamTypes;

static std::string makeLegacyParticleData(bool includeNode, bool includeRanges)
{
	std::ostringstream output(std::ios::binary);
	writeV3F32(output, v3f(1, 2, 3));
	writeV3F32(output, v3f(-1, 0.5f, 0));
	writeV3F32(output, v3f(0, -9.81f, 0));
	writeF32(output, 4.5f);
	writeF32(output, 2.25f);
	writeU8(output, true);
	output << serializeString32("legacy.png");
	writeU8(output, true);
	writeU8(output, false);
	TileAnimationParams animation;
	animation.type = TAT_NONE;
	animation.serialize(output, 47);
	writeU8(output, 4);
	writeU8(output, true);

	if (includeNode) {
		writeU16(output, 12);
		writeU8(output, 34);
		writeU8(output, 5);
	}

	if (includeRanges) {
		writeV3F32(output, v3f(0.1f, 0.2f, 0.3f));
		v3fRange jitter;
		jitter.min.val = v3f(-1, -2, -3);
		jitter.max.val = v3f(1, 2, 3);
		jitter.bias = -0.25f;
		jitter.serialize(output);
		f32Range bounce;
		bounce.min.val = 0.1f;
		bounce.max.val = 0.9f;
		bounce.bias = 0.5f;
		bounce.serialize(output);
	}

	return output.str();
}

TEST_CASE("Particle tween parameters interpolate over time", "[particles]")
{
	f32Tween tween(f32Parameter(2), f32Parameter(10));
	CHECK(tween.blend(0).val == 2);
	CHECK(tween.blend(0.5f).val == 6);
	CHECK(tween.blend(1).val == 10);

	tween.style = TweenStyle::rev;
	CHECK(tween.blend(0).val == 10);
	CHECK(tween.blend(0.5f).val == 6);
	CHECK(tween.blend(1).val == 2);

	tween.style = TweenStyle::pulse;
	CHECK(tween.blend(0).val == 2);
	CHECK(tween.blend(0.25f).val == 6);
	CHECK(tween.blend(0.5f).val == 10);
	CHECK(tween.blend(0.75f).val == 6);
	CHECK(tween.blend(1).val == 2);

	tween.style = TweenStyle::flicker;
	for (int i = 0; i < 32; i++) {
		const float flicker = tween.blend(0.25f).val;
		CHECK(flicker >= 4.8f);
		CHECK(flicker <= 6.0f);
	}
}

TEST_CASE("Particle tween offsets and repetitions preserve endpoint behavior", "[particles]")
{
	f32Tween tween(f32Parameter(2), f32Parameter(10));
	tween.beginning = 0.25f;
	tween.reps = 2;
	CHECK(tween.blend(0).val == 2);
	CHECK(tween.blend(0.25f).val == 2);
	CHECK(tween.blend(0.625f).val == 10);
	CHECK(tween.blend(0.8125f).val == 6);
	CHECK(tween.blend(1).val == 2);
}

TEST_CASE("Particle tween decoding falls back for unknown styles", "[particles][serialization]")
{
	f32Tween original(f32Parameter(1.5f), f32Parameter(2.5f));
	original.reps = 3;
	original.beginning = 0.2f;
	std::ostringstream output(std::ios::binary);
	original.serialize(output);

	std::string serialized = output.str();
	serialized[0] = static_cast<char>(0xff);
	std::istringstream input(serialized, std::ios::binary);
	f32Tween decoded;
	decoded.deSerialize(input);
	CHECK(decoded.style == TweenStyle::fwd);
	CHECK(decoded.reps == original.reps);
	CHECK(decoded.beginning == original.beginning);
	CHECK(decoded.start.val == original.start.val);
	CHECK(decoded.end.val == original.end.val);
}

TEST_CASE("Ranged particle parameters preserve bias and legacy encoding", "[particles][serialization]")
{
	f32Range original(f32Parameter(-1.5f), f32Parameter(4.25f));
	original.bias = -0.75f;

	std::ostringstream output(std::ios::binary);
	original.serialize(output);
	std::istringstream input(output.str(), std::ios::binary);
	f32Range decoded;
	decoded.deSerialize(input);
	CHECK(decoded.min.val == original.min.val);
	CHECK(decoded.max.val == original.max.val);
	CHECK(decoded.bias == original.bias);

	std::ostringstream legacy_output(std::ios::binary);
	original.legacySerialize(legacy_output);
	std::istringstream legacy_input(legacy_output.str(), std::ios::binary);
	f32Range legacy_decoded;
	legacy_decoded.legacyDeSerialize(legacy_input);
	CHECK(legacy_decoded.min.val == original.min.val);
	CHECK(legacy_decoded.max.val == original.max.val);
	CHECK(legacy_decoded.bias == 0);

	for (int i = 0; i < 100; i++) {
		const float picked = original.pickWithin().val;
		CHECK(picked >= original.min.val);
		CHECK(picked <= original.max.val);
	}
}

TEST_CASE("Particle texture protocol gates clip blending", "[particles][serialization]")
{
	ServerParticleTexture original;
	original.string = "spark.png";
	original.blendmode = BlendMode::clip;
	original.alpha.start.val = 0.25f;
	original.alpha.end.val = 0.75f;
	original.scale.start.val = v2f(0.5f, 0.75f);
	original.scale.end.val = v2f(1.5f, 2.0f);

	for (const u16 version : {u16(46), u16(47)}) {
		std::ostringstream output(std::ios::binary);
		original.serialize(output, version, false, true);
		std::istringstream input(output.str(), std::ios::binary);
		ServerParticleTexture decoded;
		decoded.deSerialize(input, version, false, true);

		CHECK(decoded.string == original.string);
		CHECK(decoded.alpha.start.val == original.alpha.start.val);
		CHECK(decoded.alpha.end.val == original.alpha.end.val);
		CHECK(decoded.scale.start.val == original.scale.start.val);
		CHECK(decoded.scale.end.val == original.scale.end.val);
		CHECK(decoded.blendmode == (version < 47 ? BlendMode::alpha : BlendMode::clip));
	}
}

TEST_CASE("Animated particle texture lengths respect protocol compatibility", "[particles][serialization]")
{
	ServerParticleTexture original;
	original.animated = true;
	original.string = "animated.png";
	original.animation.type = TAT_VERTICAL_FRAMES;
	original.animation.vertical_frames.aspect_w = 1;
	original.animation.vertical_frames.aspect_h = 2;
	original.animation.vertical_frames.length = -4.5f;

	for (const u16 version : {u16(39), u16(40)}) {
		std::ostringstream output(std::ios::binary);
		original.serialize(output, version);
		std::istringstream input(output.str(), std::ios::binary);
		ServerParticleTexture decoded;
		decoded.deSerialize(input, version);
		CHECK(decoded.animated);
		CHECK(decoded.string == original.string);
		CHECK(decoded.animation.type == TAT_VERTICAL_FRAMES);
		CHECK(decoded.animation.vertical_frames.aspect_w == 1);
		CHECK(decoded.animation.vertical_frames.aspect_h == 2);
		CHECK(decoded.animation.vertical_frames.length == (version < 40 ? 4.5f : -4.5f));
	}
}

TEST_CASE("Particle textures reject reserved blend modes", "[particles][serialization]")
{
	const char invalid_flags = static_cast<char>(static_cast<u8>(BlendMode::BlendMode_END) << 1);
	std::istringstream input(std::string(1, invalid_flags), std::ios::binary);
	ServerParticleTexture decoded;
	CHECK_THROWS_AS(decoded.deSerialize(input, 47), SerializationError);
}

TEST_CASE("Particle decoder accepts payloads from older protocol versions", "[particles][compatibility]")
{
	SECTION("pre-5.3 particle payload") {
		std::istringstream input(makeLegacyParticleData(false, false), std::ios::binary);
		ParticleParameters decoded;
		decoded.deSerialize(input, 47);
		CHECK(decoded.pos == v3f(1, 2, 3));
		CHECK(decoded.texture.string == "legacy.png");
		CHECK(decoded.node.getContent() == CONTENT_IGNORE);
	}

	SECTION("5.3 particle payload includes the node") {
		std::istringstream input(makeLegacyParticleData(true, false), std::ios::binary);
		ParticleParameters decoded;
		decoded.deSerialize(input, 47);
		CHECK(decoded.node.param0 == 12);
		CHECK(decoded.node.param2 == 34);
		CHECK(decoded.node_tile == 5);
	}

	SECTION("5.6 particle payload includes drag and ranges") {
		std::istringstream input(makeLegacyParticleData(true, true), std::ios::binary);
		ParticleParameters decoded;
		decoded.deSerialize(input, 47);
		CHECK(decoded.drag == v3f(0.1f, 0.2f, 0.3f));
		CHECK(decoded.jitter.min.val == v3f(-1, -2, -3));
		CHECK(decoded.jitter.max.val == v3f(1, 2, 3));
		CHECK(decoded.jitter.bias == -0.25f);
		CHECK(decoded.bounce.min.val == 0.1f);
		CHECK(decoded.bounce.max.val == 0.9f);
		CHECK(decoded.bounce.bias == 0.5f);
	}
}

TEST_CASE("Particle parameter serialization preserves common and ranged values", "[particles][serialization]")
{
	ParticleParameters original;
	original.pos = v3f(1, 2, 3);
	original.vel = v3f(-1, 0.5f, 0);
	original.acc = v3f(0, -9.81f, 0);
	original.drag = v3f(0.1f, 0.2f, 0.3f);
	original.expirationtime = 4.5f;
	original.size = 2.25f;
	original.collisiondetection = true;
	original.collision_removal = true;
	original.object_collision = true;
	original.vertical = true;
	original.texture.string = "smoke.png";
	original.texture.animated = false;
	original.texture.blendmode = BlendMode::add;
	original.texture.alpha.start.val = 0.2f;
	original.texture.alpha.end.val = 0.8f;
	original.texture.scale.start.val = v2f(0.5f, 0.5f);
	original.texture.scale.end.val = v2f(1.5f, 1.5f);
	original.glow = 4;
	original.node.setContent(CONTENT_AIR);
	original.node.param0 = 12;
	original.node.param2 = 34;
	original.node_tile = 5;
	original.jitter.min.val = v3f(-1, -2, -3);
	original.jitter.max.val = v3f(1, 2, 3);
	original.jitter.bias = -0.25f;
	original.bounce.min.val = 0.1f;
	original.bounce.max.val = 0.9f;
	original.bounce.bias = 0.5f;

	std::ostringstream output(std::ios::binary);
	original.serialize(output, 47);
	std::istringstream input(output.str(), std::ios::binary);
	ParticleParameters decoded;
	decoded.deSerialize(input, 47);

	CHECK(decoded.pos == original.pos);
	CHECK(decoded.vel == original.vel);
	CHECK(decoded.acc == original.acc);
	CHECK(decoded.drag == original.drag);
	CHECK(decoded.expirationtime == original.expirationtime);
	CHECK(decoded.size == original.size);
	CHECK(decoded.collisiondetection == original.collisiondetection);
	CHECK(decoded.collision_removal == original.collision_removal);
	CHECK(decoded.object_collision == original.object_collision);
	CHECK(decoded.vertical == original.vertical);
	CHECK(decoded.texture.string == original.texture.string);
	CHECK(decoded.texture.blendmode == original.texture.blendmode);
	CHECK(decoded.texture.alpha.start.val == original.texture.alpha.start.val);
	CHECK(decoded.texture.alpha.end.val == original.texture.alpha.end.val);
	CHECK(decoded.texture.scale.start.val == original.texture.scale.start.val);
	CHECK(decoded.texture.scale.end.val == original.texture.scale.end.val);
	CHECK(decoded.glow == original.glow);
	CHECK(decoded.node.param0 == original.node.param0);
	CHECK(decoded.node.param2 == original.node.param2);
	CHECK(decoded.node_tile == original.node_tile);
	CHECK(decoded.jitter.min.val == original.jitter.min.val);
	CHECK(decoded.jitter.max.val == original.jitter.max.val);
	CHECK(decoded.jitter.bias == original.jitter.bias);
	CHECK(decoded.bounce.min.val == original.bounce.min.val);
	CHECK(decoded.bounce.max.val == original.bounce.max.val);
	CHECK(decoded.bounce.bias == original.bounce.bias);
}
