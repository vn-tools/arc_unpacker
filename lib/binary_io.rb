require 'stringio'

# A StringIO that forcefully uses binary encoding.
# Using encoding other than binary leads to severe bugs in some cases.
class BinaryIO < StringIO
  def initialize(*args, &block)
    super
    set_encoding('ASCII-8BIT')
  end
end
