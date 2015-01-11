# Add ../ (with ./ being lib/) to LOAD_PATH so that files can
# require 'lib/whatever' instead of doing require_relative all the time.
$LOAD_PATH.unshift(File.join(File.dirname(__FILE__), '..'))

# Converts strings like "00 01 02" to [0x00, 0x01, 0x02].
def hex_s_to_a(str)
  str.scan(/[0-9A-Fa-f]{2}/).map(&:hex)
end

# Converts strings like "00 01 02" to "\x00\x01\x02".
def hex_s_to_s(str)
  hex_s_to_a(str).map(&:chr).join.b
end

# Error thrown by file readers, when they fail to recognize the file.
class RecognitionError < RuntimeError
end

# Expands list of folders and files into flat array of file paths.
def expand_paths(input_paths)
  paths = []
  [*input_paths].each do |input_path|
    if File.directory?(input_path)
      paths += Dir.glob(input_path + '/*')
    else
      paths << input_path
    end
  end
  paths
end

# Runs a block of code without showing warnings from Ruby.
# Useful when including a module that is written poorly.
def silence_warnings(&block)
  warn_level, $VERBOSE = $VERBOSE = nil
  result = block.call
  $VERBOSE = warn_level
  result
end
