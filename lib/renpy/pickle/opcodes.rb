# Contains list of opcodes used by Pickle.
module Pickle
  OPCODES = {
    # Pickle protocol 1 and 0
    MARK:            '(', # push special markobject on stack
    STOP:            '.', # every pickle ends with STOP
    POP:             '0', # discard topmost stack item
    POP_MARK:        '1', # discard stack top through topmost markobject
    DUP:             '2', # duplicate top stack item
    FLOAT:           'F', # push float object; decimal string argument
    INT:             'I', # push integer or bool; decimal string argument
    BININT1:         'K', # push 1-byte unsigned int
    BININT2:         'M', # push 2-byte unsigned int
    BININT4:         'J', # push four-byte signed int
    LONG:            'L', # push long; decimal string argument
    NONE:            'N', # push None
    PERSID:          'P', # push persistent object; id is taken from string arg
    BINPERSID:       'Q', # push persistent object; id is taken from stack
    REDUCE:          'R', # apply callable to argtuple, both on stack
    STRING:          'S', # push string; NL-terminated string argument
    BINSTRING:       'T', # push string; counted binary string argument
    SHORT_BINSTRING: 'U', # push string; counted binary string < 256 bytes long
    UNICODE:         'V', # push Unicode string; raw-unicode-escaped'd argument
    BINUNICODE:      'X', # push Unicode string; counted UTF-8 string argument
    APPEND:          'a', # append stack top to list below it
    BUILD:           'b', # call __setstate__ or __dict__.update()
    GLOBAL:          'c', # push self.find_class(modname, name); 2 string args
    DICT:            'd', # build a dict from stack items
    EMPTY_DICT:      '}', # push empty dict
    APPENDS:         'e', # extend list on stack by topmost stack slice
    GET:             'g', # push item from memo on stack; index is string arg
    BINGET:          'h', # push item from memo on stack; index is 1-byte arg
    LONG_BINGET:     'j', # push item from memo on stack; index is 4-byte arg
    INST:            'i', # build & push class instance
    LIST:            'l', # build list from topmost stack items
    EMPTY_LIST:      ']', # push empty list
    OBJ:             'o', # build & push class instance
    PUT:             'p', # store stack top in memo; index is string arg
    BINPUT:          'q', # store stack top in memo; index is 1-byte arg
    LONG_BINPUT:     'r', # store stack top in memo; index is 4-byte arg
    SETITEM:         's', # add key+value pair to dict
    TUPLE:           't', # build tuple from topmost stack items
    EMPTY_TUPLE:     ')', # push empty tuple
    SETITEMS:        'u', # modify dict by adding topmost key+value pairs
    BINFLOAT:        'G', # push float; arg is 8-byte float encoding

    # Pickle protocol 2
    PROTO:      "\x80".b, # identify pickle protocol
    NEWOBJ:     "\x81".b, # build object by applying cls.__new__ to argtuple
    EXT1:       "\x82".b, # push object from extension registry; 1-byte index
    EXT2:       "\x83".b, # ditto, but 2-byte index
    EXT4:       "\x84".b, # ditto, but 4-byte index
    TUPLE1:     "\x85".b, # build 1-tuple from stack top
    TUPLE2:     "\x86".b, # build 2-tuple from two topmost stack items
    TUPLE3:     "\x87".b, # build 3-tuple from three topmost stack items
    NEWTRUE:    "\x88".b, # push True
    NEWFALSE:   "\x89".b, # push False
    LONG1:      "\x8a".b, # push long from < 256 bytes
    LONG4:      "\x8b".b, # push really big long
  }
end
