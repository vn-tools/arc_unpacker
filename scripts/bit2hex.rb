#!/usr/bin/ruby

# Convert stream of 0s and 1s to hex numbers, ignoring everything else.
# Useful when manually editing bit streams.

x = STDIN.read().gsub(/[^01]/, '')
if x.length % 8 != 0
  x += '0' * (8 - x.length % 8)
end
print x.scan(/.{8}/).map { |b| '%02x' % b.to_i(2) }.join('')
