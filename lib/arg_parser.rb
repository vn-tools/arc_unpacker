# Generic error used by argument parser.
class OptionError < StandardError
  def initialize(message)
    super
  end
end

# Better argument parser.
# The key reason for using this over built-in OptParser is that it can be
# decorated dynamically without weird hacks.
class ArgParser
  def initialize(args)
    @args = args
  end

  def on(short, long, &block)
    get_internal(short, long, false, &block)
  end

  def get(short, long, &block)
    get_internal(short, long, true, &block)
  end

  def on_stray(&block)
    get_stray_internal(false, &block)
  end

  def get_stray(&block)
    get_stray_internal(true, &block)
  end

  private

  def myfail(message)
    fail OptionError, message
  end

  def get_internal(short, long, mandatory, &block)
    index = \
      @args.index('-' + strip_dashes(short)) || \
      @args.index('--' + strip_dashes(long))

    if index.nil?
      myfail(format('Required argument %s is missing.', long)) if mandatory
      return
    end

    indices = @args.each_index.select do |i|
      i > index && !@args[i].start_with?('-')
    end.first(block.arity)

    if mandatory && indices.length < block.arity
      myfail(format('Required values for %s are missing.', long))
    end

    values = @args.values_at(*indices)
    block.call(*values)

    @args.delete_if.with_index { |_, i| indices.include?(i) }
    @args.delete_at(index)
  end

  def get_stray_internal(mandatory, &block)
    fail 'This doesn\'t make sense!' if block.arity == 0

    indices = @args.each_index.select do |i|
      !@args[i].start_with?('-')
    end.first(block.arity)

    if mandatory && indices.length < block.arity
      myfail('Required arguments are missing.')
    end

    values = @args.values_at(*indices)
    block.call(*values)

    @args.delete_if.with_index { |_, i| indices.include?(i) }
  end

  def strip_dashes(key)
    (key || '').gsub(/\A-+/, '')
  end
end
