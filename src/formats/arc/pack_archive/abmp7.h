#ifndef FORMATS_ARC_PACK_ARCHIVE_ABMP7_H
#define FORMATS_ARC_PACK_ARCHIVE_ABMP7_H
#include <memory>
#include <string>
#include <vector>
#include "file.h"

void abmp7_unpack(std::vector<std::unique_ptr<File>> &files, File &b_file);

#endif
