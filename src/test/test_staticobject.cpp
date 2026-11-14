// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "staticobject.h"

#include <sstream>

TEST_CASE("StaticObject serialization preserves type, position, and data", "[serialization][static-object]")
{
	StaticObject original;
	original.type = 7;
	original.pos = v3f(1.25f, -2.5f, 3.75f);
	original.data = "entity:static-data";

	std::ostringstream output(std::ios::binary);
	original.serialize(output);
	std::istringstream input(output.str(), std::ios::binary);
	StaticObject decoded;
	decoded.deSerialize(input, 0);

	CHECK(decoded.type == original.type);
	CHECK(decoded.pos == original.pos);
	CHECK(decoded.data == original.data);
}

TEST_CASE("StaticObjectList stores active objects and reloads them as stored", "[serialization][static-object]")
{
	StaticObject stored;
	stored.type = 1;
	stored.pos = v3f(1, 2, 3);
	stored.data = "stored";
	StaticObject active;
	active.type = 2;
	active.pos = v3f(-4, 5, -6);
	active.data = "active";

	StaticObjectList list;
	list.insert(0, stored);
	list.insert(17, active);
	CHECK(list.getStoredSize() == 1);
	CHECK(list.getActiveSize() == 1);
	CHECK(list.storeActiveObject(17));
	CHECK_FALSE(list.storeActiveObject(17));
	CHECK(list.getStoredSize() == 2);
	CHECK(list.getActiveSize() == 0);

	std::ostringstream output(std::ios::binary);
	list.serialize(output);
	std::istringstream input(output.str(), std::ios::binary);
	StaticObjectList loaded;
	loaded.deSerialize(input);
	REQUIRE(loaded.getStoredSize() == 2);
	CHECK(loaded.getActiveSize() == 0);
	CHECK(loaded.getAllStored()[0].type == stored.type);
	CHECK(loaded.getAllStored()[0].pos == stored.pos);
	CHECK(loaded.getAllStored()[0].data == stored.data);
	CHECK(loaded.getAllStored()[1].type == active.type);
	CHECK(loaded.getAllStored()[1].pos == active.pos);
	CHECK(loaded.getAllStored()[1].data == active.data);
}
