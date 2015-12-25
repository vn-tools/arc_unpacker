#pragma once

#include <memory>
#include "io/file.h"
#include "logger.h"

namespace au {

    class FileSaver
    {
    public:
        virtual ~FileSaver() { }
        virtual void save(std::shared_ptr<io::File> file) const = 0;
    };

    class FileSaverHdd final : public FileSaver
    {
    public:
        FileSaverHdd(
            const Logger &logger,
            const io::path &output_dir,
            const bool overwrite);

        ~FileSaverHdd();

        void save(std::shared_ptr<io::File> file) const override;

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
        void save(std::shared_ptr<io::File> file) const override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}
