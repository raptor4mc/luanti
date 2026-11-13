// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "exceptions.h"
#include "nameidmapping.h"

#include <sstream>

TEST_CASE("NameIdMapping supports bidirectional lookup and removal", "[mapping]")
{
	NameIdMapping mapping;
	std::string name;
	u16 id = 0;
	CHECK(mapping.size() == 0);
	CHECK_FALSE(mapping.getName(42, name));
	CHECK_FALSE(mapping.getId("missing", id));

	mapping.set(42, "default:stone");
	mapping.set(300, "default:water");
	CHECK(mapping.size() == 2);
	REQUIRE(mapping.getName(42, name));
	CHECK(name == "default:stone");
	REQUIRE(mapping.getId("default:water", id));
	CHECK(id == 300);

	mapping.removeId(42);
	CHECK_FALSE(mapping.getName(42, name));
	CHECK_FALSE(mapping.getId("default:stone", id));
	mapping.eraseName("default:water");
	CHECK(mapping.size() == 0);
	CHECK_FALSE(mapping.getName(300, name));
}

TEST_CASE("NameIdMapping serialization preserves mappings and rejects unknown versions", "[mapping][serialization]")
{
	NameIdMapping original;
	original.set(1, "air");
	original.set(0x1234, "default:stone");

	std::ostringstream output(std::ios::binary);
	original.serialize(output);
	NameIdMapping decoded;
	decoded.set(99, "old:value");
	std::istringstream input(output.str(), std::ios::binary);
	decoded.deSerialize(input);

	CHECK(decoded.size() == 2);
	std::string name;
	u16 id = 0;
	REQUIRE(decoded.getName(1, name));
	CHECK(name == "air");
	REQUIRE(decoded.getId("default:stone", id));
	CHECK(id == 0x1234);
	CHECK_FALSE(decoded.getId("old:value", id));

	std::istringstream unsupported(std::string("\1", 1), std::ios::binary);
	CHECK_THROWS_AS(decoded.deSerialize(unsupported), SerializationError);
}
