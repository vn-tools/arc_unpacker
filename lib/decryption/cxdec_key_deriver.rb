# A class used to derive the nontrivial key in CXDEC based encryption.
class CxdecKeyDeriver
  def initialize(plugin)
    @plugin = plugin
    @pos = 0
    @seed = 0
    @parameter = 0
  end

  def derive(seed, parameter)
    @seed = seed
    @parameter = parameter

    # What we do: we try to run a code a few times for different "stages".
    # The first one to succeed yields the key.

    # This mechanism of figuring out the valid stage number is really crappy,
    # but it's important we do it this way. This is because we initialize the
    # seed only once, and even if we fail to get a number from the given stage,
    # the internal state of randomizer is preserved to the next iteration.

    # Maintaining the randomizer state is essential for the decryption to work.

    5.downto(1).each do |stage|
      begin
        return derive_for_stage(stage) & 0xffff_ffff
      rescue MemoryException
        next
      end
    end

    fail 'Fatal: failed to derive the key from the parameter!'
  end

  private

  class MemoryException < StandardError
  end

  # The execution for current stage must fail when we run code for too long.
  # CRASS tracked executed opcode count for this purpose, and we use this
  # approach here as well.
  def advance(count = 1)
    @pos += count
    fail MemoryException if @pos > 128
  end

  def reset
    @pos = 0
  end

  # This is a modified glibc LCG randomization routine. It is used to make the
  # key as random as possible for each file, which is supposed to maximize
  # confusion.
  def rand
    old_seed = @seed
    @seed = ((1_103_515_245 * old_seed) + 12_345) & 0xffff_ffff
    (@seed ^ (old_seed << 16) ^ (old_seed >> 16)) & 0xffff_ffff
  end

  def derive_for_stage(stage)
    reset

    advance(5) # push edi, push esi, push ebx, push ecx, push edx
    advance(4) # mov edi, dword ptr ss:[esp+18] (esp+18 == @parameter)

    eax = run_stage_strategy_1(stage)

    advance(5) # pop edx, pop ecx, pop ebx, pop esi, pop edi
    advance # retn

    eax
  end

  def run_first_stage
    routine_number = @plugin.key_derivation_order1[rand % 3]

    if routine_number == 0
      advance(1) # mov eax, ...
      eax = rand
      advance(4) # ...rand()
    elsif routine_number == 1
      advance(2) # mov eax, edi
      eax = @parameter # edi = stage
    elsif routine_number == 2
      advance(1) # mov esi, ...
      advance(4) # ...&encryption_block
      advance(2) # mov eax, ...
      pos = (rand & 0x3ff) * 4
      advance(4) # ...dword ptr ds:[esi+((rand & 0x3ff)*4)]
      eax = @plugin.encryption_block[pos..(pos + 3)].unpack('L')[0]
    end

    eax & 0xffff_ffff
  end

  def run_stage_strategy_0(stage)
    return run_first_stage if stage == 1

    if rand & 1 == 1
      eax = run_stage_strategy_1(stage - 1)
    else
      eax = run_stage_strategy_0(stage - 1)
    end

    routine_number = @plugin.key_derivation_order2[rand % 8]

    if routine_number == 0
      advance(2) # not eax
      eax ^= 0xffff_ffff

    elsif routine_number == 1
      advance # dec eax
      eax -= 1

    elsif routine_number == 2
      advance(2) # neg eax
      eax = -eax

    elsif routine_number == 3
      advance # inc eax
      eax += 1

    elsif routine_number == 4
      advance(1) # mov esi, ...
      advance(4) # ...&encryption_block
      advance(5) # and eax, 3ff
      advance(3) # mov eax, dword ptr ds:[esi+eax*4]
      pos = (eax & 0x3ff) * 4
      eax = @plugin.encryption_block[pos..(pos + 3)].unpack('L')[0]

    elsif routine_number == 5
      advance(1) # push ebx
      advance(2) # mov ebx, eax
      advance(6) # and ebx, aaaaaaaa
      advance(5) # and eax, 55555555
      advance(2) # shr ebx, 1
      advance(2) # shl eax, 1
      advance(2) # or eax, ebx
      advance(1) # pop ebx

      ebx = eax
      ebx &= 0xaaaa_aaaa
      eax &= 0x5555_5555
      ebx >>= 1
      eax <<= 1
      eax |= ebx

    elsif routine_number == 6
      advance(1) # xor eax, ...
      eax ^= rand
      advance(4) # ...rand()

    elsif routine_number == 7
      if rand & 1 == 1
        advance(1) # add eax, ...
        eax += rand
        advance(4) # ...rand()
      else
        advance(1) # sub eax, ...
        eax -= rand
        advance(4) # ...rand()
      end
    end

    eax & 0xffff_ffff
  end

  def run_stage_strategy_1(stage)
    return run_first_stage if stage == 1

    advance(1) # push ebx

    if rand & 1 == 1
      eax = run_stage_strategy_1(stage - 1)
    else
      eax = run_stage_strategy_0(stage - 1)
    end

    advance(2) # mov ebx, eax
    ebx = eax

    if rand & 1 == 1
      eax = run_stage_strategy_1(stage - 1)
    else
      eax = run_stage_strategy_0(stage - 1)
    end

    routine_number = @plugin.key_derivation_order3[rand % 6]
    if routine_number == 0
      advance # push ecx
      advance(2) # mov ecx, ebx
      advance(3) # and ecx, 0f
      advance(2) # shr eax, cl
      advance # pop ecx
      ecx = ebx & 0x0f
      eax >>= (ecx & 0xff)

    elsif routine_number == 1
      advance # push ecx
      advance(2) # mov ecx, ebx
      advance(3) # and ecx, 0f
      advance(2) # shl eax, cl
      advance # pop ecx
      ecx = ebx & 0x0f
      eax <<= (ecx & 0xff)

    elsif routine_number == 2
      advance(2) # add eax, ebx
      eax += ebx

    elsif routine_number == 3
      advance(2) # neg eax
      advance(2) # add eax, ebx
      eax = ebx - eax

    elsif routine_number == 4
      advance(3) # imul eax, ebx
      eax *= ebx

    elsif routine_number == 5
      advance(2) # sub eax, ebx
      eax -= ebx

    end

    # pop ebx
    advance

    eax & 0xffff_ffff
  end
end
