#pragma once

#include <memory>
#include "io/file.h"

namespace au {

    class FileSaver
    {
    public:
        virtual ~FileSaver() {}
        virtual io::path save(std::shared_ptr<io::File> file) const = 0;
    };

    class FileSaverHdd final : public FileSaver
    {
    public:
        FileSaverHdd(const io::path &output_dir, const bool overwrite);
        ~FileSaverHdd();

        io::path save(std::shared_ptr<io::File> file) const override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

    using FileSaveCallback = std::function<void(std::shared_ptr<io::File>)>;

    class FileSaverCallback final : public FileSaver
    {
    public:
        FileSaverCallback();
        FileSaverCallback(FileSaveCallback callback);
        ~FileSaverCallback();

        void set_callback(FileSaveCallback callback);
        io::path save(std::shared_ptr<io::File> file) const override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}
