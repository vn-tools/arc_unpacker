# A mock for Archive::OutputFilesMock.
class OutputFilesMock
  attr_reader :files

  def initialize
    @files = []
  end

  def write(&block)
    @files << block.call
  end
end
