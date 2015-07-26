#ifndef AU_COMPAT_FOPEN_H
#define AU_COMPAT_FOPEN_H
#include <boost/filesystem.hpp>

FILE *fopen(const boost::filesystem::path &path, const char *mode);

#endif
