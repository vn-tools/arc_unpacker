require_relative '../binary_io'
require_relative 'pickle/opcodes'
require_relative 'pickle/encoder'
require_relative 'pickle/decoder'

# Used to parse Python Pickle-serialized binary strings.
module Pickle
  def self.loads(input)
    Decoder.new.decode(input)
  end

  def self.dumps(input)
    Encoder.new.encode(input)
  end
end
