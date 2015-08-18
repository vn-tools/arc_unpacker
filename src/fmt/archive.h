#ifndef AU_FMT_ARCHIVE_H
#define AU_FMT_ARCHIVE_H
#include "transformer.h"

namespace au {
namespace fmt {

    class Archive : public Transformer
    {
    public:
        virtual void add_cli_help(ArgParser &) const override;
        virtual void parse_cli_options(const ArgParser &) override;
        virtual FileNamingStrategy get_file_naming_strategy() const override;
        virtual ~Archive();

        virtual void unpack(File &, FileSaver &, bool) const override;
    protected:
        virtual void unpack_internal(File &, FileSaver &) const = 0;
        void add_transformer(Transformer *transformer);

    private:
        std::vector<Transformer*> transformers;
    };

} }

#endif
