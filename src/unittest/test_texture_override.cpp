// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "test.h"
#include "texture_override.h"

#include <fstream>

class TestTextureOverride : public TestBase
{
public:
	TestTextureOverride() { TestManager::registerTestModule(this); }
	const char *getName() { return "TestTextureOverride"; }

	void runTests(IGameDef *gamedef);
	void testTargetParsing();
	void testWorldAlignmentParsing();
};

static TestTextureOverride g_test_instance;

void TestTextureOverride::runTests(IGameDef *gamedef)
{
	TEST(testTargetParsing);
	TEST(testWorldAlignmentParsing);
}

void TestTextureOverride::testTargetParsing()
{
	const std::string filepath = getTestTempFile();
	{
		std::ofstream file(filepath);
		file << "# comment\r\n"
				"\r\n"
				"test:stone top,overlay_top stone.png\r\n"
				"test:item inventory item.png\r\n"
				"test:tool wield tool.png\r\n"
				"test:both all,inventory combo.png\r\n"
				"test:invalid unknown bad.png\r\n"
				"malformed\r\n";
	}

	TextureOverrideSource source(filepath);
	const auto nodes = source.getNodeTileOverrides();
	const auto items = source.getItemTextureOverrides();
	UASSERTEQ(size_t, nodes.size(), 2);
	UASSERTEQ(size_t, items.size(), 3);

	UASSERTEQ(auto, nodes[0].id, "test:stone");
	UASSERTEQ(auto, nodes[0].texture, "stone.png");
	UASSERT(nodes[0].hasTarget(OverrideTarget::TOP));
	UASSERT(nodes[0].hasTarget(OverrideTarget::OVERLAY_TOP));
	UASSERT(!nodes[0].hasTarget(OverrideTarget::BOTTOM));

	UASSERTEQ(auto, items[0].id, "test:item");
	UASSERT(items[0].hasTarget(OverrideTarget::INVENTORY));
	UASSERTEQ(auto, items[1].id, "test:tool");
	UASSERT(items[1].hasTarget(OverrideTarget::WIELD));
	UASSERTEQ(auto, items[2].id, "test:both");
	UASSERT(items[2].hasTarget(OverrideTarget::INVENTORY));
	UASSERT(nodes[1].hasTarget(OverrideTarget::TOP));
	UASSERT(nodes[1].hasTarget(OverrideTarget::BOTTOM));
}

void TestTextureOverride::testWorldAlignmentParsing()
{
	const std::string filepath = getTestTempFile();
	{
		std::ofstream file(filepath);
		file << "test:small top,align_world=17 small.png\n"
				"test:large bottom,align_world=300 large.png\n";
	}

	TextureOverrideSource source(filepath);
	const auto overrides = source.getNodeTileOverrides();
	UASSERTEQ(size_t, overrides.size(), 2);
	UASSERTEQ(auto, overrides[0].id, "test:small");
	UASSERTEQ(u8, overrides[0].world_scale, 17);
	UASSERTEQ(u8, overrides[1].world_scale, U8_MAX);
}
