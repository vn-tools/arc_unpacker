#include "file_saver.h"

using namespace au;

struct FileSaverCallback::Priv final
{
    Priv(FileSaveCallback callback);

    FileSaveCallback callback;
};

FileSaverCallback::Priv::Priv(FileSaveCallback callback) : callback(callback)
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
    return file->path;
}
