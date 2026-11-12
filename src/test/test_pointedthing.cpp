// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "exceptions.h"
#include "util/pointedthing.h"

#include <sstream>

TEST_CASE("PointedThing wire encoding is stable", "[serialization][pointedthing]")
{
	SECTION("nothing") {
		PointedThing value;
		std::ostringstream output(std::ios::binary);
		value.serialize(output);
		CHECK(output.str() == std::string("\0\0", 2));
	}

	SECTION("node coordinates use the versioned big-endian format") {
		PointedThing value(v3s16(1, -2, 3), v3s16(4, 5, 6), v3s16(7, 8, 9),
			v3f(), v3f(), 0, 0, PointabilityType::POINTABLE);
		std::ostringstream output(std::ios::binary);
		value.serialize(output);
		CHECK(output.str() == std::string(
			"\0\1\0\1\xff\xfe\0\3\0\4\0\5\0\6", 14));

		std::istringstream input(output.str(), std::ios::binary);
		PointedThing decoded;
		decoded.deSerialize(input);
		CHECK(decoded.type == POINTEDTHING_NODE);
		CHECK(decoded.node_undersurface == v3s16(1, -2, 3));
		CHECK(decoded.node_abovesurface == v3s16(4, 5, 6));
	}

	SECTION("object IDs use the versioned big-endian format") {
		PointedThing value(0x1234, v3f(), v3f(), v3f(), 0, PointabilityType::POINTABLE);
		std::ostringstream output(std::ios::binary);
		value.serialize(output);
		CHECK(output.str() == std::string("\0\2\x12\x34", 4));

		std::istringstream input(output.str(), std::ios::binary);
		PointedThing decoded;
		decoded.deSerialize(input);
		CHECK(decoded.type == POINTEDTHING_OBJECT);
		CHECK(decoded.object_id == 0x1234);
	}
}

TEST_CASE("PointedThing rejects unsupported and truncated encodings", "[serialization][pointedthing]")
{
	SECTION("unsupported version") {
		std::istringstream input(std::string("\1\0", 2), std::ios::binary);
		PointedThing value;
		CHECK_THROWS_AS(value.deSerialize(input), SerializationError);
	}

	SECTION("unknown type") {
		std::istringstream input(std::string("\0\xff", 2), std::ios::binary);
		PointedThing value;
		CHECK_THROWS_AS(value.deSerialize(input), SerializationError);
	}

	SECTION("truncated object ID") {
		std::istringstream input(std::string("\0\2\x12", 3), std::ios::binary);
		PointedThing value;
		CHECK_THROWS_AS(value.deSerialize(input), SerializationError);
	}
}
