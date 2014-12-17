# Converts --fmt string to instance of given archive.
class ArchiveFactory
  def self.get(fmt)
    factory[fmt].call
  end

  def self.format_strings
    factory.keys
  end

  def self.factory
    {
      'xp3/noop' => lambda do
        require_relative 'kirikiri/xp3_archive'
        require_relative 'kirikiri/decryptors/noop'
        Xp3Archive.new(NoopDecryptor.new)
      end,

      'xp3/fsn' => lambda do
        require_relative 'kirikiri/xp3_archive'
        require_relative 'kirikiri/decryptors/fsn'
        Xp3Archive.new(FsnDecryptor.new)
      end,

      'xp3/fha' => lambda do
        require_relative 'kirikiri/xp3_archive'
        require_relative 'kirikiri/decryptors/cxdec'
        require_relative 'kirikiri/decryptors/cxdec_plugin_fha'
        Xp3Archive.new(CxdecDecryptor.new(CxdecPluginFha.new))
      end,

      'ykc' => lambda do
        require_relative 'ykc/ykc_archive'
        YkcArchive.new
      end,

      'sar' => lambda do
        require_relative 'nscripter/sar/sar_archive'
        SarArchive.new
      end,

      'nsa' => lambda do
        require_relative 'nscripter/nsa/nsa_archive'
        NsaArchive.new
      end,

      'melty_blood' => lambda do
        require_relative 'french_bread/melty_blood/melty_blood_archive'
        MeltyBloodArchive.new
      end,

      'nitroplus/pak' => lambda do
        require_relative 'nitroplus/pak/pak_archive'
        PakArchive.new
      end
    }
  end
end
