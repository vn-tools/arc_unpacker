# A mock for Archive::InputFilesMock.
class InputFilesMock
  attr_reader :files

  def initialize(files)
    @files = files
  end

  def names
    @files.map(&:name)
  end

  def each(&block)
    @files.each { |f| block.call(f.clone) }
  end

  def reverse_each(&block)
    @files.reverse_each { |f| block.call(f.clone) }
  end

  def length
    @files.length
  end
end
