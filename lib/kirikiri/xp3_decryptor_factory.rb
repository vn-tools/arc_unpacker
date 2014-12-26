XP3_DECRYPTORS = {
  none: lambda do
    require_relative 'decryptors/noop'
    NoopDecryptor.new
  end,

  fsn: lambda do
    require_relative 'decryptors/fsn'
    FsnDecryptor.new
  end,

  fha: lambda do
    require_relative 'decryptors/cxdec'
    require_relative 'decryptors/cxdec_plugin_fha'
    CxdecDecryptor.new(CxdecPluginFha.new)
  end
}
