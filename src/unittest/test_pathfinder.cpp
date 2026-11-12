// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "test.h"
#include "dummymap.h"
#include "gamedef.h"
#include "pathfinder.h"
#include "constants.h"

#include <cstdlib>

class TestPathfinder : public TestBase
{
public:
	TestPathfinder() { TestManager::registerTestModule(this); }
	const char *getName() { return "TestPathfinder"; }

	void runTests(IGameDef *gamedef);
	void testFlatGroundPath(IGameDef *gamedef);
	void testRejectsWalkableEndpoints(IGameDef *gamedef);
	void testNoRouteBetweenIsolatedSurfaces(IGameDef *gamedef);
};

static TestPathfinder g_test_instance;

void TestPathfinder::runTests(IGameDef *gamedef)
{
	TEST(testFlatGroundPath, gamedef);
	TEST(testRejectsWalkableEndpoints, gamedef);
	TEST(testNoRouteBetweenIsolatedSurfaces, gamedef);
}

void TestPathfinder::testFlatGroundPath(IGameDef *gamedef)
{
	DummyMap map(gamedef, {0, 0, 0}, {0, 0, 0});
	for (s16 z = 0; z < MAP_BLOCKSIZE; z++) {
		for (s16 x = 0; x < MAP_BLOCKSIZE; x++) {
			map.setNode({x, 0, z}, MapNode(t_CONTENT_STONE));
			map.setNode({x, 1, z}, MapNode(CONTENT_AIR));
		}
	}

	const v3s16 source(2, 1, 2);
	const v3s16 destination(5, 1, 2);
	const PathAlgorithm algorithms[] = {PA_DIJKSTRA, PA_PLAIN, PA_PLAIN_NP};

	for (PathAlgorithm algorithm : algorithms) {
		const std::vector<v3s16> path = get_path(&map,
			gamedef->getNodeDefManager(), source, destination, 2, 0, 0, algorithm);
		UASSERT(!path.empty());
		UASSERTEQ(v3s16, path.front(), source);
		UASSERTEQ(v3s16, path.back(), destination);

		for (size_t i = 1; i < path.size(); i++) {
			const v3s16 delta = path[i] - path[i - 1];
			UASSERT(std::abs(delta.X) + std::abs(delta.Y) + std::abs(delta.Z) == 1);
			UASSERT(delta.Y == 0);
			UASSERTEQ(content_t, map.getNode(path[i] + v3s16(0, -1, 0)).getContent(),
				t_CONTENT_STONE);
		}
	}
}

void TestPathfinder::testRejectsWalkableEndpoints(IGameDef *gamedef)
{
	DummyMap map(gamedef, {0, 0, 0}, {0, 0, 0});
	map.setNode({2, 0, 2}, MapNode(t_CONTENT_STONE));
	map.setNode({2, 1, 2}, MapNode(CONTENT_AIR));
	map.setNode({5, 0, 2}, MapNode(t_CONTENT_STONE));
	map.setNode({5, 1, 2}, MapNode(CONTENT_AIR));

	const v3s16 air_source(2, 1, 2);
	const v3s16 air_destination(5, 1, 2);
	const v3s16 walkable_source(2, 0, 2);
	const v3s16 walkable_destination(5, 0, 2);
	UASSERT(get_path(&map, gamedef->getNodeDefManager(), air_source,
			walkable_destination, 2, 0, 0, PA_PLAIN).empty());
	UASSERT(get_path(&map, gamedef->getNodeDefManager(), walkable_source,
			air_destination, 2, 0, 0, PA_PLAIN).empty());
}

void TestPathfinder::testNoRouteBetweenIsolatedSurfaces(IGameDef *gamedef)
{
	DummyMap map(gamedef, {0, 0, 0}, {0, 0, 0});
	const s16 island_positions[] = {2, 5};
	for (s16 x : island_positions) {
		map.setNode({x, 0, 2}, MapNode(t_CONTENT_STONE));
		map.setNode({x, 1, 2}, MapNode(CONTENT_AIR));
	}

	const v3s16 source(2, 1, 2);
	const v3s16 destination(5, 1, 2);
	for (PathAlgorithm algorithm : {PA_DIJKSTRA, PA_PLAIN, PA_PLAIN_NP}) {
		UASSERT(get_path(&map, gamedef->getNodeDefManager(), source, destination,
				1, 0, 0, algorithm).empty());
	}
}
