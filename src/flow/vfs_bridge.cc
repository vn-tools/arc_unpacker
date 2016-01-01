#include "flow/vfs_bridge.h"

using namespace au;
using namespace au::flow;

struct VirtualFileSystemBridge::Priv final
{
    Priv(
        const Logger &logger,
        const dec::BaseArchiveDecoder &decoder,
        const std::shared_ptr<dec::ArchiveMeta> meta,
        const std::shared_ptr<io::File> input_file,
        const io::path &base_name) :
            logger(logger),
            decoder(decoder),
            meta(meta),
            base_name(base_name),
            decoder_refcount(decoder.shared_from_this())
    {
        for (const auto &entry : meta->entries)
        {
            VirtualFileSystem::register_file(
                get_target_name(entry->path),
                [&logger, input_file, meta, &entry, &decoder]()
                {
                    io::File file_copy(*input_file);
                    return decoder.read_file(logger, file_copy, *meta, *entry);
                });
        }
    }

    ~Priv()
    {
        for (const auto &entry : meta->entries)
        {
            VirtualFileSystem::unregister_file(
                get_target_name(entry->path));
        }
    }

    io::path get_target_name(const io::path &input_path) const
    {
        return algo::apply_naming_strategy(
            decoder.naming_strategy(), base_name, input_path);
    }

    const Logger logger;
    const dec::BaseArchiveDecoder &decoder;
    const std::shared_ptr<dec::ArchiveMeta> meta;
    const io::path base_name;

    // Prolongs decoder life to match those of the bridge. Avoids
    // the need to downcast to shared_ptr<BaseArchiveDecoder>.
    // TODO: probably not needed after we get long-lived task chains
    const std::shared_ptr<const dec::IDecoder> decoder_refcount;
};

VirtualFileSystemBridge::VirtualFileSystemBridge(
    const Logger &logger,
    const dec::BaseArchiveDecoder &decoder,
    const std::shared_ptr<dec::ArchiveMeta> meta,
    const std::shared_ptr<io::File> input_file,
    const io::path &base_name)
    : p(new Priv(logger, decoder, meta, input_file, base_name))
{
}

VirtualFileSystemBridge::~VirtualFileSystemBridge()
{
}
