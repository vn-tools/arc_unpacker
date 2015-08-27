#pragma once

#include <boost/filesystem.hpp>

FILE *fopen(const boost::filesystem::path &path, const char *mode);
