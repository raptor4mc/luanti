// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "test.h"
#include "database/database-files.h"
#include "gamedef.h"
#include "remoteplayer.h"
#include "server/player_sao.h"

#include <memory>
#include <vector>

class TestPlayerDatabase : public TestBase
{
public:
	TestPlayerDatabase() { TestManager::registerTestModule(this); }
	const char *getName() { return "TestPlayerDatabase"; }

	void runTests(IGameDef *gamedef);
	void testSaveLoadRemove(IGameDef *gamedef);
};

static TestPlayerDatabase g_test_instance;

void TestPlayerDatabase::runTests(IGameDef *gamedef)
{
	TEST(testSaveLoadRemove, gamedef);
}

void TestPlayerDatabase::testSaveLoadRemove(IGameDef *gamedef)
{
	const std::string savedir = getTestTempDirectory() + DIR_DELIM + "players";
	PlayerDatabaseFiles database(savedir);
	std::unique_ptr<PlayerSAO> sao;
	RemotePlayer player("database_test", gamedef->getItemDefManager());
	sao = std::make_unique<PlayerSAO>(nullptr, &player, 42, false);
	player.setPlayerSAO(sao.get());

	sao->setHPRaw(17);
	sao->setBasePosition(v3f(10.5f, -2.0f, 30.25f));
	sao->setLookPitch(12.5f);
	sao->setPlayerYaw(-35.0f);
	sao->setBreath(6, false);
	sao->getMeta().setString("rank", "builder");
	player.inventory.addList("main", 3);

	database.savePlayer(&player);
	UASSERT(!player.checkModified());

	std::unique_ptr<PlayerSAO> loaded_sao;
	RemotePlayer loaded("database_test", gamedef->getItemDefManager());
	loaded_sao = std::make_unique<PlayerSAO>(nullptr, &loaded, 43, false);
	loaded.setPlayerSAO(loaded_sao.get());
	UASSERT(database.loadPlayer(&loaded, loaded_sao.get()));
	UASSERTEQ(auto, loaded.getName(), "database_test");
	UASSERTEQ(u16, loaded_sao->getHP(), 17);
	UASSERT(loaded_sao->getBasePosition() == v3f(10.5f, -2.0f, 30.25f));
	UASSERTEQ(f32, loaded_sao->getLookPitch(), 12.5f);
	UASSERTEQ(f32, loaded_sao->getRotation().Y, -35.0f);
	UASSERTEQ(u16, loaded_sao->getBreath(), 6);
	UASSERTEQ(auto, loaded_sao->getMeta().getString("rank"), "builder");
	UASSERT(!loaded_sao->getMeta().isModified());
	UASSERT(loaded.inventory.getList("main"));
	UASSERTEQ(u32, loaded.inventory.getList("main")->getSize(), 3);

	std::vector<std::string> names;
	database.listPlayers(names);
	UASSERTEQ(size_t, names.size(), 1);
	UASSERTEQ(auto, names.front(), "database_test");

	UASSERT(database.removePlayer("database_test"));
	UASSERT(!database.loadPlayer(&loaded, loaded_sao.get()));
	UASSERT(!database.removePlayer("database_test"));
}
