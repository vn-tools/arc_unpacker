# A mock for Archive::InputFilesMock.
class InputFilesMock
  def initialize(files, meta = nil)
    @files = files
    @meta = meta
  end

  def names
    @files.map { |f| f[:file_name] }
  end

  def each(&block)
    @files.each { |f| block.call(f[:file_name], f[:data]) }
  end

  def length
    @files.length
  end

  def read_meta
    @meta
  end
end
