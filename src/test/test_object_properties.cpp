// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "exceptions.h"
#include "object_properties.h"

#include <sstream>

TEST_CASE("ObjectProperties serialization preserves protocol fields", "[serialization][object-properties]")
{
	ObjectProperties original;
	original.textures = {"body.png", "overlay.png"};
	original.colors = {video::SColor(255, 12, 34, 56), video::SColor(128, 65, 43, 21)};
	original.collisionbox = aabb3f(-1, -2, -3, 4, 5, 6);
	original.selectionbox = aabb3f(-0.5f, -1, -0.5f, 0.5f, 2, 0.5f);
	original.visual = OBJECTVISUAL_MESH;
	original.mesh = "character.b3d";
	original.damage_texture_modifier = "^[colorize:#abcdef";
	original.nametag = "Builder";
	original.infotext = "Ready";
	original.wield_item = "default:pick";
	original.visual_size = v3f(1.5f, 2, 0.75f);
	original.nametag_color = video::SColor(255, 200, 180, 160);
	original.nametag_bgcolor = video::SColor(100, 20, 40, 60);
	original.nametag_fontsize = 18;
	original.spritediv = v2s16(8, 4);
	original.initial_sprite_basepos = v2s16(2, 1);
	original.stepheight = 0.6f;
	original.automatic_rotate = 0.25f;
	original.automatic_face_movement_dir_offset = 0.5f;
	original.automatic_face_movement_max_rotation_per_sec = 1.25f;
	original.eye_height = 1.75f;
	original.zoom_fov = 12;
	original.node = MapNode(CONTENT_AIR);
	original.node.param2 = 29;
	original.hp_max = 40;
	original.breath_max = 15;
	original.glow = -3;
	original.pointable = PointabilityType::POINTABLE_BLOCKING;
	original.physical = true;
	original.collideWithObjects = false;
	original.rotate_selectionbox = true;
	original.is_visible = false;
	original.makes_footstep_sound = true;
	original.automatic_face_movement_dir = true;
	original.backface_culling = false;
	original.use_texture_alpha = true;
	original.shaded = false;
	original.show_on_minimap = true;
	original.nametag_scale_z = true;
	original.step_up_mode = StepUpMode::RIGID;

	std::ostringstream output(std::ios::binary);
	original.serialize(output);
	std::istringstream input(output.str(), std::ios::binary);
	ObjectProperties decoded;
	decoded.deSerialize(input);
	CHECK(decoded == original);
}

TEST_CASE("ObjectProperties preserves absent and transparent nametag backgrounds", "[serialization][object-properties]")
{
	ObjectProperties without_background;
	std::ostringstream absent_output(std::ios::binary);
	without_background.serialize(absent_output);
	std::istringstream absent_input(absent_output.str(), std::ios::binary);
	ObjectProperties absent_decoded;
	absent_decoded.deSerialize(absent_input);
	CHECK_FALSE(absent_decoded.nametag_bgcolor.has_value());

	ObjectProperties transparent_background;
	transparent_background.nametag_bgcolor = video::SColor(0, 0, 0, 0);
	std::ostringstream transparent_output(std::ios::binary);
	transparent_background.serialize(transparent_output);
	std::istringstream transparent_input(transparent_output.str(), std::ios::binary);
	ObjectProperties transparent_decoded;
	transparent_decoded.deSerialize(transparent_input);
	REQUIRE(transparent_decoded.nametag_bgcolor.has_value());
	CHECK(transparent_decoded.nametag_bgcolor.value() == video::SColor(0, 0, 0, 0));
}

TEST_CASE("ObjectProperties rejects unsupported protocol versions", "[serialization][object-properties]")
{
	std::istringstream input(std::string("\3", 1), std::ios::binary);
	ObjectProperties properties;
	CHECK_THROWS_AS(properties.deSerialize(input), SerializationError);
}
