// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "metadata.h"

#include <algorithm>

TEST_CASE("SimpleMetadata tracks key-value changes", "[metadata]")
{
	SimpleMetadata metadata;
	CHECK(metadata.empty());
	CHECK(metadata.size() == 0);
	CHECK_FALSE(metadata.isModified());
	CHECK_FALSE(metadata.contains("owner"));
	CHECK(metadata.getString("owner").empty());

	CHECK(metadata.setString("owner", "builder"));
	CHECK(metadata.contains("owner"));
	CHECK(metadata.getString("owner") == "builder");
	CHECK(metadata.isModified());
	CHECK(metadata.size() == 1);

	metadata.setModified(false);
	CHECK_FALSE(metadata.setString("owner", "builder"));
	CHECK_FALSE(metadata.isModified());
	CHECK(metadata.setString("owner", "miner"));
	CHECK(metadata.isModified());
	CHECK(metadata.getString("owner") == "miner");

	metadata.setModified(false);
	CHECK(metadata.removeString("owner"));
	CHECK(metadata.empty());
	CHECK(metadata.isModified());
	CHECK_FALSE(metadata.removeString("missing"));
}

TEST_CASE("SimpleMetadata exposes keys and compares values", "[metadata]")
{
	SimpleMetadata first;
	SimpleMetadata second;
	first.setString("a", "one");
	first.setString("b", "two");
	second.setString("b", "two");
	second.setString("a", "one");

	CHECK(first == second);
	std::vector<std::string> keys;
	CHECK(first.getKeys(&keys).size() == 2);
	std::sort(keys.begin(), keys.end());
	const std::vector<std::string> expected_keys{"a", "b"};
	CHECK(keys == expected_keys);

	second.setString("b", "different");
	CHECK(first != second);
	second.clear();
	CHECK(second.empty());
	CHECK(first != second);
}

TEST_CASE("Metadata resolves bounded references and missing values", "[metadata]")
{
	SimpleMetadata metadata;
	metadata.setString("name", "Stone");
	metadata.setString("display", "${name}");
	metadata.setString("loop_a", "${loop_b}");
	metadata.setString("loop_b", "${loop_a}");

	CHECK(metadata.getString("display") == "Stone");
	CHECK(metadata.resolveString("plain") == "plain");
	CHECK(metadata.getString("absent").empty());

	std::string value = "unchanged";
	CHECK_FALSE(metadata.getStringToRef("absent", value));
	CHECK(value == "unchanged");
	CHECK(metadata.getStringToRef("display", value));
	CHECK(value == "Stone");

	const std::string cycle = metadata.getString("loop_a");
	CHECK(cycle == "${loop_b}");
}
