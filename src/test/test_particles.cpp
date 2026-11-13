// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "particles.h"

#include <sstream>

using namespace ParticleParamTypes;

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
