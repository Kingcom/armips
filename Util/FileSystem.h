#pragma once

#ifdef ARMIPS_USE_STD_FILESYSTEM

#include <filesystem>
#include <fstream>

namespace fs
{
	using namespace std::filesystem;
	using ifstream = std::ifstream;
	using ofstream = std::ofstream;
	using fstream = std::fstream;
}

#else

#include <ghc/fs_fwd.hpp>

namespace fs
{
	using namespace ghc::filesystem;
	using ifstream = ghc::filesystem::ifstream;
	using ofstream = ghc::filesystem::ofstream;
	using fstream = ghc::filesystem::fstream;
}

#endif
