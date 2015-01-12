# A mock for Archive::InputFilesMock.
class InputFilesMock
  def initialize(files)
    @files = files
  end

  def names
    @files.map { |f| f[:file_name] }
  end

  def each(&block)
    @files.each { |f| block.call(f[:file_name], f[:data]) }
  end

  def reverse_each(&block)
    @files.reverse_each { |f| block.call(f[:file_name], f[:data]) }
  end

  def length
    @files.length
  end
end
