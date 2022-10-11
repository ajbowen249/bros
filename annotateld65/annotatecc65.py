# Copied from and modified
# https://raw.githubusercontent.com/dustmop/annotatecc65/master/annotatecc65.py

import os
import re

def read_file(name):
  fp = open(name, 'r')
  content = fp.read()
  fp.close()
  return content

def annotate_intermediary(source_basename, content, fout, fmap):
  """Insert meta-labels into the compiled source while building map file."""
  is_code = n = 0
  for line in content.split('\n'):
    if not line:
      continue
    # cc65 outputs source code as three commented lines.
    if line[0] == ';':
      is_code += 1
    else:
      is_code = 0
    # cc65 disables debuginfo, turn it back on.
    if re.match(r'\W*.debuginfo.*off', line):
      fout.write('.debuginfo on\n')
      continue
    if re.match(r'\W*.debuginfo.*-', line):
      fout.write('.debuginfo +\n')
      continue
    fout.write(line + '\n')
    # The source code appears on the middle commented line.
    if is_code == 2:
      code_text = line[1:]
    elif is_code == 3:
      # Don't output source code for raw assembly, because it's not needed.
      if '__asm__' in code_text:
        continue
      if n:
        fout.write('_Rsource_map__%s__%04d:\n' % (source_basename, n))
        fmap.write('%s__%04d %s\n' % (source_basename, n, code_text))
      n += 1

def process():
  output_file = './bros.s'
  final_dir = os.path.dirname(output_file)
  final_name = os.path.basename(output_file)
  final_base, final_ext = os.path.splitext(final_name)
  inter_file = os.path.join(final_dir, '.annotate.' + final_base + '.s')
  outmap_file = os.path.join(final_dir, '.annotate.' + final_base + '.map')
  fout = open(output_file, 'w')
  fmap = open(outmap_file, 'w')
  annotate_intermediary(final_base, read_file(inter_file), fout, fmap)
  fout.close()
  fmap.close()


if __name__ == '__main__':
  process()
