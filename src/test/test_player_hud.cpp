// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "player.h"
#include "hud_element.h"

#include <memory>

TEST_CASE("Player HUD reuses removed IDs and trims empty tails", "[player][hud]")
{
	PlayerHud hud;
	CHECK(hud.get(0) == nullptr);

	const u32 first_id = hud.add(std::make_unique<HudElement>());
	const u32 second_id = hud.add(std::make_unique<HudElement>());
	REQUIRE(first_id == 0);
	REQUIRE(second_id == 1);
	CHECK(hud.get(first_id) != nullptr);
	CHECK(hud.get(second_id) != nullptr);
	CHECK(hud.get(2) == nullptr);

	CHECK(hud.remove(first_id));
	CHECK(hud.get(first_id) == nullptr);
	const u32 reused_id = hud.add(std::make_unique<HudElement>());
	CHECK(reused_id == first_id);

	CHECK(hud.remove(second_id));
	CHECK(hud.get(second_id) == nullptr);
	CHECK(hud.remove(reused_id));
	CHECK(hud.get(reused_id) == nullptr);
	CHECK_FALSE(hud.remove(0));

	const u32 next_id = hud.add(std::make_unique<HudElement>());
	CHECK(next_id == 0);
	hud.clear();
	CHECK(hud.get(next_id) == nullptr);
}
