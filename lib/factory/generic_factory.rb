# Converts --fmt string to a corresponding module.
class GenericFactory
  def self.get(fmt)
    factory[fmt].call
  end

  def self.format_strings
    factory.keys
  end

  def self.each(&block)
    factory.each { |k, v| block.call(k, v.call) }
  end

  def self.factory
    fail 'Implement me'
  end
end
