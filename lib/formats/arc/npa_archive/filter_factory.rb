NPA_FILTERS = {
  chaos_head: lambda do
    require_relative 'filters/chaos_head_filter'
    ChaosHeadFilter.new
  end
}
