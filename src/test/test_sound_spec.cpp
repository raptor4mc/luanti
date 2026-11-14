// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2026 Luanti Authors

#include "catch.h"
#include "sound_spec.h"

#include <sstream>

TEST_CASE("SoundSpec simple serialization preserves supported properties", "[sound][serialization]")
{
	const SoundSpec original("wind", 0.5f, true, 1.5f, 2.0f, 3.0f);
	std::ostringstream output(std::ios::binary);
	original.serializeSimple(output, 47);

	std::istringstream input(output.str(), std::ios::binary);
	SoundSpec decoded;
	decoded.deSerializeSimple(input, 47);
	CHECK(decoded.name == "wind");
	CHECK(decoded.gain == 0.5f);
	CHECK(decoded.pitch == 2.0f);
	CHECK(decoded.fade == 1.5f);
	CHECK_FALSE(decoded.loop);
	CHECK(decoded.start_time == 0.0f);
	CHECK(decoded.exists());
}

TEST_CASE("SoundSpec existence is based on the sound name", "[sound]")
{
	CHECK_FALSE(SoundSpec().exists());
	CHECK(SoundSpec("ambient").exists());
}
