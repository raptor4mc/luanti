// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "player.h"

#include <cmath>

TEST_CASE("Player controls pack action keys into protocol bits", "[player][controls]")
{
	PlayerControl controls;
	controls.jump = true;
	controls.aux1 = true;
	controls.sneak = true;
	controls.dig = true;
	controls.place = true;
	controls.zoom = true;
	CHECK(controls.getKeysPressed() == 0x3f0);

	controls.unpackKeysPressed(0x3ff);
	CHECK(controls.up == 1);
	CHECK(controls.down == 1);
	CHECK(controls.left == 1);
	CHECK(controls.right == 1);
	CHECK(controls.jump);
	CHECK(controls.aux1);
	CHECK(controls.sneak);
	CHECK(controls.dig);
	CHECK(controls.place);
	CHECK(controls.zoom);
}

TEST_CASE("Player keyboard movement maps cardinal directions", "[player][controls]")
{
	struct DirectionCase {
		float up;
		float down;
		float left;
		float right;
		u32 expected_bit;
		v2f expected_movement;
	};
	const DirectionCase cases[] = {
		{1, 0, 0, 0, 1u << 0, v2f(0, 1)},
		{0, 1, 0, 0, 1u << 1, v2f(0, -1)},
		{0, 0, 1, 0, 1u << 2, v2f(-1, 0)},
		{0, 0, 0, 1, 1u << 3, v2f(1, 0)},
	};

	for (const DirectionCase &test : cases) {
		PlayerControl controls;
		controls.up = test.up;
		controls.down = test.down;
		controls.left = test.left;
		controls.right = test.right;
		controls.setMovementFromKeys();
		CHECK((controls.getKeysPressed() & 0xf) == test.expected_bit);
		const v2f movement = controls.getMovement();
		CHECK(std::abs(movement.X - test.expected_movement.X) < 0.0001f);
		CHECK(std::abs(movement.Y - test.expected_movement.Y) < 0.0001f);
	}
}

TEST_CASE("Player diagonal movement is normalized and keyboard input preserves joystick state", "[player][controls]")
{
	PlayerControl controls;
	controls.up = 1;
	controls.right = 1;
	controls.setMovementFromKeys();
	const v2f diagonal = controls.getMovement();
	CHECK(std::abs(diagonal.X - 0.7071067f) < 0.0001f);
	CHECK(std::abs(diagonal.Y - 0.7071067f) < 0.0001f);
	CHECK((controls.getKeysPressed() & 0xf) == ((1u << 0) | (1u << 3)));

	controls.movement_speed = 0.25f;
	controls.movement_direction = 1.0f;
	controls.up = 0;
	controls.right = 0;
	controls.setMovementFromKeys();
	CHECK(controls.movement_speed == 0.25f);
	CHECK(controls.movement_direction == 1.0f);
}
