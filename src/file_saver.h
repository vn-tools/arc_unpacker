#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include <vector>
#include "file.h"

class FileSaver
{
public:
    virtual void save(std::shared_ptr<File> file) const = 0;
};

class FileSaverHdd : public FileSaver
{
public:
    FileSaverHdd(const boost::filesystem::path &output_dir, bool overwrite);
    ~FileSaverHdd();

    virtual void save(std::shared_ptr<File> file) const override;
private:
    struct Priv;
    std::unique_ptr<Priv> p;
};

typedef std::function<void(std::shared_ptr<File>)> FileSaveCallback;

class FileSaverCallback : public FileSaver
{
public:
    FileSaverCallback();
    FileSaverCallback(FileSaveCallback callback);
    ~FileSaverCallback();

    void set_callback(FileSaveCallback callback);
    virtual void save(std::shared_ptr<File> file) const override;
private:
    struct Priv;
    std::unique_ptr<Priv> p;
};

#endif
