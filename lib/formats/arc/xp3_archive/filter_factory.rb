XP3_FILTERS = {
  none: lambda do
    require_relative 'filters/noop'
    NoopFilter.new
  end,

  fsn: lambda do
    require_relative 'filters/fsn'
    FsnFilter.new
  end,

  fha: lambda do
    require_relative 'filters/cxdec'
    require_relative 'filters/cxdec_plugin_fha'
    CxdecFilter.new(CxdecPluginFha.new)
  end
}
