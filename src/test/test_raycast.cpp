// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "constants.h"
#include "raycast.h"

#include <cmath>

TEST_CASE("Ray-box collision finds each axis-aligned face", "[raycast]")
{
	const aabb3f box(0, 0, 0, 1, 1, 1);
	struct FaceCase {
		v3f start;
		v3f direction;
		v3f point;
		v3f normal;
	};
	const FaceCase cases[] = {
		{{-1, 0.5f, 0.5f}, {2, 0, 0}, {0, 0.5f, 0.5f}, {-1, 0, 0}},
		{{2, 0.5f, 0.5f}, {-2, 0, 0}, {1, 0.5f, 0.5f}, {1, 0, 0}},
		{{0.5f, -1, 0.5f}, {0, 2, 0}, {0.5f, 0, 0.5f}, {0, -1, 0}},
		{{0.5f, 2, 0.5f}, {0, -2, 0}, {0.5f, 1, 0.5f}, {0, 1, 0}},
		{{0.5f, 0.5f, -1}, {0, 0, 2}, {0.5f, 0.5f, 0}, {0, 0, -1}},
		{{0.5f, 0.5f, 2}, {0, 0, -2}, {0.5f, 0.5f, 1}, {0, 0, 1}},
	};

	for (const FaceCase &test : cases) {
		v3f point, normal;
		REQUIRE(boxLineCollision(box, test.start, test.direction, &point, &normal));
		CHECK(point == test.point);
		CHECK(normal == test.normal);
	}
}

TEST_CASE("Raycast candidate sorting prioritizes nearer hits", "[raycast]")
{
	const PointedThing near_node(v3s16(1, 0, 0), v3s16(0, 0, 0), v3s16(1, 0, 0),
		v3f(), v3f(), 0, BS * BS, PointabilityType::POINTABLE);
	const PointedThing farther_node(v3s16(2, 0, 0), v3s16(1, 0, 0), v3s16(2, 0, 0),
		v3f(), v3f(), 0, 3 * BS * BS, PointabilityType::POINTABLE);
	RaycastSort sort;
	CHECK_FALSE(sort(near_node, farther_node));
	CHECK(sort(farther_node, near_node));

	const PointedThing object(7, v3f(), v3f(), v3f(), 1.5f * BS * BS,
		PointabilityType::POINTABLE);
	CHECK_FALSE(sort(object, near_node));
	CHECK(sort(near_node, object));
}

TEST_CASE("Rotated ray-box collision transforms points and normals", "[raycast]")
{
	const aabb3f box(0, 0, 0, 1, 1, 1);
	core::quaternion rotation;
	rotation.fromAngleAxis(1.57079632679f, core::vector3df(0, 0, 1));
	const v3f local_start(-1, 0.5f, 0.5f);
	const v3f local_direction(2, 0, 0);
	const v3f start = rotation * local_start;
	const v3f direction = rotation * local_direction;
	v3f point, normal, raw_normal;

	REQUIRE(boxLineCollision(box, rotation, start, direction,
			&point, &normal, &raw_normal));
	CHECK(std::abs(point.X + 0.5f) < 0.0001f);
	CHECK(std::abs(point.Y) < 0.0001f);
	CHECK(std::abs(point.Z - 0.5f) < 0.0001f);
	CHECK(std::abs(normal.X) < 0.0001f);
	CHECK(std::abs(normal.Y + 1.0f) < 0.0001f);
	CHECK(std::abs(normal.Z) < 0.0001f);
	CHECK(raw_normal == v3f(-1, 0, 0));
}

TEST_CASE("Ray-box collision handles contained, endpoint, and missed rays", "[raycast]")
{
	const aabb3f box(0, 0, 0, 1, 1, 1);
	v3f point, normal;

	SECTION("a start inside the box collides immediately") {
		const v3f start(0.25f, 0.5f, 0.75f);
		CHECK(boxLineCollision(box, start, v3f(2, 0, 0), &point, &normal));
		CHECK(point == start);
		CHECK(normal == v3f(0, 0, 0));
	}

	SECTION("a collision exactly at the line endpoint is included") {
		CHECK(boxLineCollision(box, v3f(-1, 0.5f, 0.5f), v3f(1, 0, 0), &point, &normal));
		CHECK(point == v3f(0, 0.5f, 0.5f));
		CHECK(normal == v3f(-1, 0, 0));
	}

	SECTION("a parallel ray outside the box misses") {
		CHECK_FALSE(boxLineCollision(box, v3f(-1, 2, 0.5f), v3f(2, 0, 0), &point, &normal));
	}

	SECTION("a zero-length ray outside the box misses") {
		CHECK_FALSE(boxLineCollision(box, v3f(-1, 0.5f, 0.5f), v3f(0, 0, 0), &point, &normal));
	}
}
