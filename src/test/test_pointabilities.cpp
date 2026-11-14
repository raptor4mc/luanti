// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "exceptions.h"
#include "util/pointabilities.h"

#include <sstream>

TEST_CASE("Pointability matches names before groups", "[pointabilities]")
{
	Pointabilities abilities;
	abilities.nodes["default:stone"] = PointabilityType::POINTABLE_NOT;
	abilities.node_groups["cracky"] = PointabilityType::POINTABLE;

	CHECK(abilities.matchNode("default:stone", {{"cracky", 3}}) ==
		PointabilityType::POINTABLE_NOT);
	CHECK(abilities.matchNode("other:stone", {{"cracky", 3}}) ==
		PointabilityType::POINTABLE);
	CHECK_FALSE(abilities.matchNode("other:stone", {{"crumbly", 1}}).has_value());
}

TEST_CASE("Pointability group rules have stable precedence", "[pointabilities]")
{
	Pointabilities abilities;
	abilities.node_groups["blocked"] = PointabilityType::POINTABLE_BLOCKING;
	abilities.node_groups["ignored"] = PointabilityType::POINTABLE_NOT;

	CHECK(abilities.matchNode("node", {{"blocked", 1}}) ==
		PointabilityType::POINTABLE_BLOCKING);
	CHECK(abilities.matchNode("node", {{"ignored", 1}}) ==
		PointabilityType::POINTABLE_NOT);
	CHECK(abilities.matchNode("node", {{"ignored", 1}, {"blocked", 1}}) ==
		PointabilityType::POINTABLE_NOT);

	abilities.node_groups["allowed"] = PointabilityType::POINTABLE;
	CHECK(abilities.matchNode("node", {{"allowed", 1}, {"ignored", 1}}) ==
		PointabilityType::POINTABLE);
}

TEST_CASE("Object and player pointability use object overrides", "[pointabilities]")
{
	Pointabilities abilities;
	abilities.objects["creatures:critter"] = PointabilityType::POINTABLE_BLOCKING;
	abilities.object_groups["fleshy"] = PointabilityType::POINTABLE_NOT;

	CHECK(abilities.matchObject("creatures:critter", {{"fleshy", 1}}) ==
		PointabilityType::POINTABLE_BLOCKING);
	CHECK(abilities.matchObject("creatures:other", {{"fleshy", 1}}) ==
		PointabilityType::POINTABLE_NOT);
	CHECK(abilities.matchPlayer({{"fleshy", 1}}) == PointabilityType::POINTABLE_NOT);
}

TEST_CASE("Pointabilities serialization round-trips all maps", "[pointabilities][serialization]")
{
	Pointabilities original;
	original.nodes["default:stone"] = PointabilityType::POINTABLE;
	original.node_groups["cracky"] = PointabilityType::POINTABLE_BLOCKING;
	original.objects["creatures:critter"] = PointabilityType::POINTABLE_NOT;
	original.object_groups["fleshy"] = PointabilityType::POINTABLE_BLOCKING;

	std::ostringstream output(std::ios::binary);
	original.serialize(output);
	std::istringstream input(output.str(), std::ios::binary);
	Pointabilities decoded;
	decoded.deSerialize(input);

	CHECK(decoded.nodes == original.nodes);
	CHECK(decoded.node_groups == original.node_groups);
	CHECK(decoded.objects == original.objects);
	CHECK(decoded.object_groups == original.object_groups);
}

TEST_CASE("Pointabilities reject unknown versions and safely map unknown enum values", "[pointabilities][serialization]")
{
	SECTION("unsupported version") {
		std::istringstream input(std::string("\1", 1), std::ios::binary);
		Pointabilities abilities;
		CHECK_THROWS_AS(abilities.deSerialize(input), SerializationError);
	}

	SECTION("unknown enum value defaults to pointable") {
		std::istringstream input(std::string("\xff", 1), std::ios::binary);
		CHECK(Pointabilities::deSerializePointabilityType(input) ==
			PointabilityType::POINTABLE);
	}
}
