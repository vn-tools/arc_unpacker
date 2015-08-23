import io
import sys
import struct

class open_ext(object):
    def __init__ (self, *args):
        if args:
            self.file = open(*args)
        else:
            self.file = sys.stdout.buffer

    def __getattr__(self, attr):
        return getattr(self.file, attr)

    def __enter__ (self):
        return self

    def __exit__ (self, exc_type, exc_value, traceback):
        self.file.close()

    def skip(self, bytes):
        self.file.seek(bytes, io.SEEK_CUR)

    def peek(self, *args):
        return self.PeekObject(self.file, *args)

    def read_to_zero(self):
        out = b''
        byte = self.file.read(1)
        while byte != b"\x00":
            out += byte
            byte = self.file.read(1)
        return out

    def read_to_eof(self):
        return self.file.read()

    def read_u8(self):
        return struct.unpack('B', self.file.read(1))[0]

    def read_u16_le(self):
        return struct.unpack('<H', self.file.read(2))[0]

    def read_u32_le(self):
        return struct.unpack('<I', self.file.read(4))[0]

    def read_u64_le(self):
        return struct.unpack('<Q', self.file.read(8))[0]

    def read_u16_be(self):
        return struct.unpack('>H', self.file.read(2))[0]

    def read_u32_be(self):
        return struct.unpack('>I', self.file.read(4))[0]

    def read_u64_be(self):
        return struct.unpack('>Q', self.file.read(8))[0]

    def write_u8(self, x):
        self.file.write(struct.pack('B', x))

    def write_u16_le(self, x):
        self.file.write(struct.pack('<H', x))

    def write_u32_le(self, x):
        self.file.write(struct.pack('<I', x))

    def write_u64_le(self, x):
        self.file.write(struct.pack('<Q', x))

    def write_u16_be(self, x):
        self.fibe.write(struct.pack('>H', x))

    def write_u32_be(self, x):
        self.fibe.write(struct.pack('>I', x))

    def write_u64_be(self, x):
        self.fibe.write(struct.pack('>Q', x))

    class PeekObject(object):
        def __init__(self, file, *seek_args):
            self.file = file
            self.seek_args = seek_args
        def __enter__(self):
            self.old_pos = self.file.tell()
            self.file.seek(*self.seek_args)
        def __exit__(self, *unused):
            self.file.seek(self.old_pos)
