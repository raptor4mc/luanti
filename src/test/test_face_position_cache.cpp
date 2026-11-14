// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "face_position_cache.h"

#include <algorithm>
#include <cstdlib>
#include <set>
#include <thread>
#include <tuple>

using Position = std::tuple<s16, s16, s16>;

static Position toPosition(const v3s16 &pos)
{
	return {pos.X, pos.Y, pos.Z};
}

TEST_CASE("Face position cache returns the center and the six-neighbor shell", "[face-position-cache]")
{
	const auto &center = FacePositionCache::getFacePositions(0);
	REQUIRE(center.size() == 1);
	CHECK(center[0] == v3s16(0, 0, 0));

	const auto &neighbors = FacePositionCache::getFacePositions(1);
	REQUIRE(neighbors.size() == 26);
	std::set<Position> unique_positions;
	for (const v3s16 &pos : neighbors) {
		CHECK((pos.X >= -1 && pos.X <= 1));
		CHECK((pos.Y >= -1 && pos.Y <= 1));
		CHECK((pos.Z >= -1 && pos.Z <= 1));
		CHECK(pos != v3s16(0, 0, 0));
		unique_positions.insert(toPosition(pos));
	}
	CHECK(unique_positions.size() == neighbors.size());

	const auto &cached_again = FacePositionCache::getFacePositions(1);
	CHECK(&cached_again == &neighbors);
}

TEST_CASE("Face position cache returns only the outer cube shell", "[face-position-cache]")
{
	for (s16 radius = 1; radius <= 5; radius++) {
		const auto &positions = FacePositionCache::getFacePositions(radius);
		CHECK(positions.size() == static_cast<size_t>(24 * radius * radius + 2));
		std::set<Position> unique_positions;
		for (const v3s16 &pos : positions) {
			CHECK(std::max({std::abs(pos.X), std::abs(pos.Y), std::abs(pos.Z)}) == radius);
			unique_positions.insert(toPosition(pos));
		}
		CHECK(unique_positions.size() == positions.size());
	}
}

TEST_CASE("Face position cache serializes concurrent first access", "[face-position-cache][threading]")
{
	constexpr size_t thread_count = 8;
	std::vector<const std::vector<v3s16> *> results(thread_count);
	std::vector<std::thread> threads;
	threads.reserve(thread_count);
	for (size_t i = 0; i < thread_count; i++) {
		threads.emplace_back([&, i] {
			results[i] = &FacePositionCache::getFacePositions(6);
		});
	}
	for (std::thread &thread : threads)
		thread.join();

	REQUIRE(results[0] != nullptr);
	for (const auto *result : results)
		CHECK(result == results[0]);
	CHECK(results[0]->size() == 24 * 6 * 6 + 2);
}
