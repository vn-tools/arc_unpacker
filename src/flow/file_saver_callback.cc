#include "flow/file_saver_callback.h"

using namespace au;
using namespace au::flow;

struct FileSaverCallback::Priv final
{
    Priv(FileSaveCallback callback);

    FileSaveCallback callback;
    size_t saved_file_count;
};

FileSaverCallback::Priv::Priv(FileSaveCallback callback)
    : callback(callback), saved_file_count(0)
{
}

FileSaverCallback::FileSaverCallback() : p(new Priv(nullptr))
{
}

FileSaverCallback::FileSaverCallback(FileSaveCallback callback)
    : p(new Priv(callback))
{
}

FileSaverCallback::~FileSaverCallback()
{
}

void FileSaverCallback::set_callback(FileSaveCallback callback)
{
    p->callback = callback;
}

io::path FileSaverCallback::save(std::shared_ptr<io::File> file) const
{
    p->callback(file);
    ++p->saved_file_count;
    return file->path;
}

size_t FileSaverCallback::get_saved_file_count() const
{
    return p->saved_file_count;
}
