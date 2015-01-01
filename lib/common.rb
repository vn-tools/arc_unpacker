$LOAD_PATH.unshift(File.join(File.dirname(__FILE__), '..'))

# Generic error thrown by the archive readers.
class ArcError < RuntimeError
end
