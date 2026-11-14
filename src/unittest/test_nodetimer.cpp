// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "test.h"
#include "exceptions.h"
#include "nodetimer.h"

#include <sstream>

class TestNodeTimer : public TestBase
{
public:
	TestNodeTimer() { TestManager::registerTestModule(this); }
	const char *getName() { return "TestNodeTimer"; }

	void runTests(IGameDef *gamedef);
	void testTimerSerialization();
	void testListSerialization();
	void testTimerLifecycle();
	void testSetReplacesExistingTimer();
};

static TestNodeTimer g_test_instance;

void TestNodeTimer::runTests(IGameDef *gamedef)
{
	TEST(testTimerSerialization);
	TEST(testListSerialization);
	TEST(testTimerLifecycle);
	TEST(testSetReplacesExistingTimer);
}

void TestNodeTimer::testTimerSerialization()
{
	const NodeTimer original(12.5f, 2.25f, v3s16(3, 4, 5));
	std::stringstream stream;
	original.serialize(stream);

	NodeTimer decoded;
	decoded.deSerialize(stream);
	UASSERTEQ(f32, decoded.timeout, original.timeout);
	UASSERTEQ(f32, decoded.elapsed, original.elapsed);
}

void TestNodeTimer::testListSerialization()
{
	NodeTimerList empty;
	std::ostringstream empty_v24(std::ios::binary);
	empty.serialize(empty_v24, 24);
	UASSERTEQ(auto, empty_v24.str(), std::string("\0", 1));

	std::ostringstream empty_v25(std::ios::binary);
	empty.serialize(empty_v25, 25);
	UASSERTEQ(auto, empty_v25.str(), std::string("\x0a\0\0", 3));

	const v3s16 pos(3, 4, 5);
	NodeTimerList original;
	original.set(NodeTimer(12.5f, 2.5f, pos));
	const u8 versions[] = {24, 25};
	for (u8 version : versions) {
		std::ostringstream output(std::ios::binary);
		original.serialize(output, version);
		std::istringstream input(output.str(), std::ios::binary);
		NodeTimerList decoded;
		decoded.deSerialize(input, version);
		const NodeTimer timer = decoded.get(pos);
		UASSERTEQ(f32, timer.timeout, 12.5f);
		UASSERTEQ(f32, timer.elapsed, 2.5f);
	}

	std::istringstream bad_v24(std::string("\2", 1), std::ios::binary);
	EXCEPTION_CHECK(SerializationError, empty.deSerialize(bad_v24, 24));
	std::istringstream bad_v25(std::string("\x09", 1), std::ios::binary);
	EXCEPTION_CHECK(SerializationError, empty.deSerialize(bad_v25, 25));
}

void TestNodeTimer::testTimerLifecycle()
{
	NodeTimerList timers;
	const v3s16 first_pos(1, 2, 3);
	const v3s16 second_pos(4, 5, 6);
	timers.set(NodeTimer(3, 0, first_pos));
	timers.set(NodeTimer(5, 0, second_pos));

	UASSERTEQ(f32, timers.get(first_pos).elapsed, 0);
	UASSERT(timers.step(2).empty());
	UASSERTEQ(f32, timers.get(first_pos).elapsed, 2);

	std::vector<NodeTimer> expired = timers.step(1);
	UASSERTEQ(size_t, expired.size(), 1);
	UASSERT(expired[0].position == first_pos);
	UASSERTEQ(f32, expired[0].elapsed, 3);
	UASSERTEQ(f32, timers.get(first_pos).timeout, 0);
	UASSERTEQ(f32, timers.get(second_pos).elapsed, 3);

	timers.remove(second_pos);
	UASSERTEQ(f32, timers.get(second_pos).timeout, 0);
	UASSERT(timers.step(10).empty());

	timers.set(NodeTimer(20, 0, first_pos));
	timers.clear();
	UASSERTEQ(f32, timers.get(first_pos).timeout, 0);
	UASSERT(timers.step(30).empty());
}

void TestNodeTimer::testSetReplacesExistingTimer()
{
	NodeTimerList timers;
	const v3s16 pos(2, 3, 4);
	timers.set(NodeTimer(10, 0, pos));
	UASSERT(timers.step(2).empty());

	timers.set(NodeTimer(6, 1, pos));
	UASSERTEQ(f32, timers.get(pos).timeout, 6);
	UASSERTEQ(f32, timers.get(pos).elapsed, 1);
	UASSERT(timers.step(4).empty());

	const std::vector<NodeTimer> expired = timers.step(1);
	UASSERTEQ(size_t, expired.size(), 1);
	UASSERTEQ(f32, expired[0].timeout, 6);
	UASSERTEQ(f32, expired[0].elapsed, 6);
}
