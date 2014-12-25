require 'stringio'

# A BinaryIO that encapsulates both IO and StringIO, enforcing binary encoding.
# Using encoding other than binary leads to severe bugs in some cases.
class BinaryIO
  private_class_method :new

  def self.from_file(*args, &block)
    io = File.open(*args)
    io.set_encoding('ASCII-8BIT')
    run(io, &block)
  end

  def self.from_string(*args, &block)
    io = StringIO.new(*args)
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
