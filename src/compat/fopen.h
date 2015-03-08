#ifndef COMPAT_FOPEN_H
#define COMPAT_FOPEN_H
#include <boost/filesystem.hpp>

FILE *fopen(const boost::filesystem::path &path, const char *mode);

#endif
