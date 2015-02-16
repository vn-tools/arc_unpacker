#ifndef FORMATS_ARC_PACK_ARCHIVE_ABMP10_H
#define FORMATS_ARC_PACK_ARCHIVE_ABMP10_H
#include <memory>
#include <string>
#include <vector>
#include "file.h"

bool abmp10_unpack(std::vector<std::unique_ptr<File>> &files, File &b_file);

#endif
