# Runs a block of code without warnings.
def silence_warnings(&block)
  warn_level, $VERBOSE = $VERBOSE = nil
  result = block.call
  $VERBOSE = warn_level
  result
end
