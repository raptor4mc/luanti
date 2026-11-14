// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "clientdynamicinfo.h"

TEST_CASE("ClientDynamicInfo compares network-visible fields", "[client][protocol]")
{
	ClientDynamicInfo base{};
	base.render_target_size = v2u32(1920, 1080);
	base.real_gui_scaling = 1.25f;
	base.real_hud_scaling = 1.5f;
	base.max_fs_size = v2f32(32, 18);
	base.touch_controls = false;

	CHECK(base.equal(base));

	SECTION("render target size") {
		auto changed = base;
		changed.render_target_size.X++;
		CHECK_FALSE(base.equal(changed));
	}

	SECTION("GUI scaling uses a tolerance") {
		auto changed = base;
		changed.real_gui_scaling += 0.0005f;
		CHECK(base.equal(changed));
		changed.real_gui_scaling += 0.002f;
		CHECK_FALSE(base.equal(changed));
	}

	SECTION("HUD scaling uses a tolerance") {
		auto changed = base;
		changed.real_hud_scaling += 0.002f;
		CHECK_FALSE(base.equal(changed));
	}

	SECTION("maximum formspec size changes are significant") {
		auto changed = base;
		changed.max_fs_size.X += 0.002f;
		CHECK_FALSE(base.equal(changed));
	}

	SECTION("touch-control changes are significant") {
		auto changed = base;
		changed.touch_controls = true;
		CHECK_FALSE(base.equal(changed));
	}
}
