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
      'xp3' => lambda do
        require_relative 'kirikiri/xp3_archive'
        Xp3Archive.new
      end,

      'ykc' => lambda do
        require_relative 'yuka/ykc_archive'
        YkcArchive.new
      end,

      'sar' => lambda do
        require_relative 'nscripter/sar_archive'
        SarArchive.new
      end,

      'nsa' => lambda do
        require_relative 'nscripter/nsa_archive'
        NsaArchive.new
      end,

      'melty_blood' => lambda do
        require_relative 'french_bread/melty_blood_archive'
        MeltyBloodArchive.new
      end,

      'nitroplus/pak' => lambda do
        require_relative 'nitroplus/pak_archive'
        PakArchive.new
      end,

      'fjsys' => lambda do
        require_relative 'nsystem/fjsys_archive.rb'
        FjsysArchive.new
      end,

      'rpa' => lambda do
        require_relative 'renpy/rpa_archive.rb'
        RpaArchive.new
      end,

      'exe' => lambda do
        require_relative 'misc/exe_reader.rb'
        ExeReader.new
      end
    }
  end
end
