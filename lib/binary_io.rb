require 'stringio'

# A BinaryIO that encapsulates both IO and StringIO, adding a few methods and
# enforcing binary encoding. Using encoding other than binary leads to severe
# bugs in some cases.
class BinaryIO
  private_class_method :new

  def self.from_file(*args, &block)
    io = File.open(*args)
    io.set_encoding('ASCII-8BIT')
    run(io, &block)
  end

  def self.from_string(str = nil, &block)
    io = StringIO.new(str || '')
    io.set_encoding('ASCII-8BIT')
    run(io, &block)
  end

  def self.run(io, &block)
    instance = new(io)
    if block.nil?
      ObjectSpace.define_finalizer(instance, proc { io.close })
      return instance
    else
      ret = block.call(instance)
      io.close
      return ret
    end
  end

  def peek(pos, &block)
    old_pos = @io.tell
    @io.seek(pos)
    ret = block.call
    @io.seek(old_pos)
    ret
  end

  def skip(count)
    @io.seek(count, IO::SEEK_CUR)
  end

  def read_until_zero
    str = ''
    while ((c = @io.read(1)) || "\0") != "\0"
      str += c
    end
    str
  end

  def respond_to?(method_name, include_private = false)
    @io.respond_to?(method_name) || super(method_name, include_private)
  end

  def method_missing(method_name, *arguments, &block)
    if @io.respond_to?(method_name)
      return @io.send(method_name, *arguments, &block)
    else
      return super(method_name, *arguments, &block)
    end
  end

  private

  def initialize(io)
    @io = io
  end
end
