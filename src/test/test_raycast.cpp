// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "constants.h"
#include "raycast.h"

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
