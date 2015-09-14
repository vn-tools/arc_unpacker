#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include <vector>
#include "file.h"

namespace au {

    class FileSaver
    {
    public:
        virtual void save(std::shared_ptr<File> file) const = 0;
    };

    class FileSaverHdd final : public FileSaver
    {
    public:
        FileSaverHdd(const boost::filesystem::path &output_dir, bool overwrite);
        ~FileSaverHdd();

        virtual void save(std::shared_ptr<File> file) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

    using FileSaveCallback = std::function<void(std::shared_ptr<File>)>;

    class FileSaverCallback final : public FileSaver
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

}
