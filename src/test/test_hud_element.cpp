// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "hud_element.h"
#include "util/enum_string.h"

#include <string_view>
#include <utility>

TEST_CASE("HUD element type names retain their protocol values", "[hud][protocol]")
{
	const std::pair<HudElementType, const char *> values[] = {
		{HUD_ELEM_IMAGE, "image"},
		{HUD_ELEM_TEXT, "text"},
		{HUD_ELEM_STATBAR, "statbar"},
		{HUD_ELEM_INVENTORY, "inventory"},
		{HUD_ELEM_WAYPOINT, "waypoint"},
		{HUD_ELEM_IMAGE_WAYPOINT, "image_waypoint"},
		{HUD_ELEM_COMPASS, "compass"},
		{HUD_ELEM_MINIMAP, "minimap"},
		{HUD_ELEM_HOTBAR, "hotbar"},
	};

	for (const auto &[type, name] : values) {
		CHECK(enum_to_string(es_HudElementType, type) == std::string_view(name));
		int decoded = -1;
		REQUIRE(string_to_enum(es_HudElementType, decoded, name));
		CHECK(decoded == static_cast<int>(type));
	}
}

TEST_CASE("HUD element stat aliases remain accepted", "[hud][compatibility]")
{
	HudElementStat stat = HudElementStat_END;
	CHECK(string_to_enum(es_HudElementStat, stat, "position"));
	CHECK(stat == HUD_STAT_POS);
	CHECK(string_to_enum(es_HudElementStat, stat, "pos"));
	CHECK(stat == HUD_STAT_POS);
	CHECK(string_to_enum(es_HudElementStat, stat, "precision"));
	CHECK(stat == HUD_STAT_ITEM);

	const HudElementStat previous = stat;
	CHECK_FALSE(string_to_enum(es_HudElementStat, stat, "not_a_hud_stat"));
	CHECK(stat == previous);
}

TEST_CASE("Built-in HUD flag names map to stable bits", "[hud][protocol]")
{
	const std::pair<u32, const char *> values[] = {
		{HUD_FLAG_HOTBAR_VISIBLE, "hotbar"},
		{HUD_FLAG_HEALTHBAR_VISIBLE, "healthbar"},
		{HUD_FLAG_CROSSHAIR_VISIBLE, "crosshair"},
		{HUD_FLAG_WIELDITEM_VISIBLE, "wielditem"},
		{HUD_FLAG_BREATHBAR_VISIBLE, "breathbar"},
		{HUD_FLAG_MINIMAP_VISIBLE, "minimap"},
		{HUD_FLAG_MINIMAP_RADAR_VISIBLE, "minimap_radar"},
		{HUD_FLAG_BASIC_DEBUG, "basic_debug"},
		{HUD_FLAG_CHAT_VISIBLE, "chat"},
	};

	for (const auto &[flag, name] : values) {
		int decoded = 0;
		REQUIRE(string_to_enum(es_HudBuiltinElement, decoded, name));
		CHECK(static_cast<u32>(decoded) == flag);
	}
}
