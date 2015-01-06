$LOAD_PATH.unshift(File.join(File.dirname(__FILE__), '..'))

# Converts strings like 00 01 02 to [0x00, 0x01, 0x02].
def hex_s_to_a(str)
  str.scan(/[0-9A-Fa-f]{2}/).map(&:hex)
end

# Converts strings like 00 01 02 to "\x00\x01\x02".
def hex_s_to_s(str)
  hex_s_to_a(str).map(&:chr).join.b
end

# Generic error thrown by the archive readers.
class ArcError < RuntimeError
end
