// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "test.h"
#include "gamedef.h"
#include "nodemetadata.h"

#include <sstream>

class TestNodeMetadata : public TestBase
{
public:
	TestNodeMetadata() { TestManager::registerTestModule(this); }
	const char *getName() { return "TestNodeMetadata"; }

	void runTests(IGameDef *gamedef);
	void testPrivateMetadata(IGameDef *gamedef);
	void testMetadataListPositions(IGameDef *gamedef);
	void testEmptyMetadataPolicy(IGameDef *gamedef);
};

static TestNodeMetadata g_test_instance;

void TestNodeMetadata::runTests(IGameDef *gamedef)
{
	TEST(testPrivateMetadata, gamedef);
	TEST(testMetadataListPositions, gamedef);
	TEST(testEmptyMetadataPolicy, gamedef);
}

void TestNodeMetadata::testPrivateMetadata(IGameDef *gamedef)
{
	NodeMetadata original(gamedef->getItemDefManager());
	original.setString("public", "visible");
	original.setString("private", "hidden");
	UASSERT(original.markPrivate("private", true));
	UASSERT(!original.markPrivate("private", true));
	UASSERT(original.isPrivate("private"));

	std::ostringstream disk_output(std::ios::binary);
	original.serialize(disk_output, 2, true);
	std::istringstream disk_input(disk_output.str(), std::ios::binary);
	NodeMetadata disk_copy(gamedef->getItemDefManager());
	disk_copy.deSerialize(disk_input, 2);
	UASSERTEQ(auto, disk_copy.getString("public"), "visible");
	UASSERTEQ(auto, disk_copy.getString("private"), "hidden");
	UASSERT(disk_copy.isPrivate("private"));

	std::ostringstream legacy_output(std::ios::binary);
	original.serialize(legacy_output, 1, true);
	std::istringstream legacy_input(legacy_output.str(), std::ios::binary);
	NodeMetadata legacy_copy(gamedef->getItemDefManager());
	legacy_copy.deSerialize(legacy_input, 1);
	UASSERTEQ(auto, legacy_copy.getString("private"), "hidden");
	UASSERT(!legacy_copy.isPrivate("private"));

	std::ostringstream network_output(std::ios::binary);
	original.serialize(network_output, 2, false);
	std::istringstream network_input(network_output.str(), std::ios::binary);
	NodeMetadata network_copy(gamedef->getItemDefManager());
	network_copy.deSerialize(network_input, 2);
	UASSERTEQ(auto, network_copy.getString("public"), "visible");
	UASSERT(!network_copy.contains("private"));
	UASSERT(!network_copy.isPrivate("private"));

	UASSERT(network_copy.markPrivate("unused", true));
	UASSERT(network_copy.markPrivate("unused", false));
	UASSERT(!network_copy.markPrivate("unused", false));
	network_copy.clear();
	UASSERT(network_copy.empty());
}

void TestNodeMetadata::testMetadataListPositions(IGameDef *gamedef)
{
	const v3s16 local_pos(2, 3, 4);
	NodeMetadataList local_list;
	auto local_data = std::make_unique<NodeMetadata>(gamedef->getItemDefManager());
	local_data->setString("name", "chest");
	local_list.set(local_pos, local_data.release());

	std::ostringstream local_output(std::ios::binary);
	local_list.serialize(local_output, 28, true, false);
	std::istringstream local_input(local_output.str(), std::ios::binary);
	NodeMetadataList local_copy;
	local_copy.deSerialize(local_input, gamedef->getItemDefManager(), false);
	UASSERTEQ(size_t, local_copy.size(), 1);
	UASSERTEQ(auto, local_copy.get(local_pos)->getString("name"), "chest");

	std::ostringstream legacy_output(std::ios::binary);
	local_list.serialize(legacy_output, 27, true, false);
	std::istringstream legacy_input(legacy_output.str(), std::ios::binary);
	NodeMetadataList legacy_copy;
	legacy_copy.deSerialize(legacy_input, gamedef->getItemDefManager(), false);
	UASSERTEQ(size_t, legacy_copy.size(), 1);
	UASSERTEQ(auto, legacy_copy.get(local_pos)->getString("name"), "chest");

	const v3s16 absolute_pos(-10, -11, 12);
	NodeMetadataList absolute_list;
	auto absolute_data = std::make_unique<NodeMetadata>(gamedef->getItemDefManager());
	absolute_data->setString("text", "sign");
	absolute_list.set(absolute_pos, absolute_data.release());

	std::ostringstream absolute_output(std::ios::binary);
	absolute_list.serialize(absolute_output, 28, true, true);
	std::istringstream absolute_input(absolute_output.str(), std::ios::binary);
	NodeMetadataList absolute_copy;
	absolute_copy.deSerialize(absolute_input, gamedef->getItemDefManager(), true);
	UASSERTEQ(size_t, absolute_copy.size(), 1);
	UASSERTEQ(auto, absolute_copy.get(absolute_pos)->getString("text"), "sign");

	std::unique_ptr<NodeMetadata> removed = absolute_copy.remove(absolute_pos);
	UASSERT(removed);
	UASSERTEQ(size_t, absolute_copy.size(), 0);
	UASSERT(removed->empty());
}

void TestNodeMetadata::testEmptyMetadataPolicy(IGameDef *gamedef)
{
	NodeMetadataList list;
	const v3s16 pos(1, 2, 3);
	list.set(pos, new NodeMetadata(gamedef->getItemDefManager()));

	std::ostringstream omitted_output(std::ios::binary);
	list.serialize(omitted_output, 28, true, false, false);
	UASSERTEQ(auto, omitted_output.str(), std::string("\0", 1));
	std::istringstream omitted_input(omitted_output.str(), std::ios::binary);
	NodeMetadataList omitted_copy;
	omitted_copy.deSerialize(omitted_input, gamedef->getItemDefManager());
	UASSERTEQ(size_t, omitted_copy.size(), 0);

	std::ostringstream included_output(std::ios::binary);
	list.serialize(included_output, 28, true, false, true);
	std::istringstream included_input(included_output.str(), std::ios::binary);
	NodeMetadataList included_copy;
	included_copy.deSerialize(included_input, gamedef->getItemDefManager());
	UASSERTEQ(size_t, included_copy.size(), 1);
	UASSERT(included_copy.get(pos)->empty());
}
