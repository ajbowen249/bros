from audioop import add
import sys
import re

def build_symbol_map(lines):
    symbol_map = {}
    address_map = {}
    for line in lines:
        match = re.match(r"^al (?P<address>[\dA-F]{6}) (?P<name>.*)$", line)
        if not match:
            continue
        groups = match.groups()
        symbol_map[groups[1]] = groups[0]
        address_map[groups[0]] = groups[1]

    return (symbol_map, address_map)

def main():
    args = sys.argv[1:]
    left_filename = args[0]
    right_filename = args[1]
    left_text = ''
    right_text = ''

    with open(left_filename, 'r') as input_file:
        left_text = input_file.read()

    with open(right_filename, 'r') as input_file:
        right_text = input_file.read()

    left_lines = left_text.split('\n')
    right_lines = right_text.split('\n')

    (left_s_map, left_a_map) = build_symbol_map(left_lines)
    (right_s_map, right_a_map) = build_symbol_map(right_lines)

    files_match = True

    for symname in left_a_map:
        left_symname = left_a_map[symname]
        if symname in right_a_map:
            right_symname = right_a_map[symname]
            if right_symname != left_symname:
                files_match = False
                print("address " + symname + " named " + left_symname + " renamed " + right_symname)

    for symname in left_s_map:
        left_address = left_s_map[symname]
        if symname in right_s_map:
            right_address = right_s_map[symname]
            if right_address != left_address:
                files_match = False
                print("symbol " + symname + " moved from " + left_address + " to " + right_address)

    if files_match:
        print("No moved symbols")

if __name__ == '__main__':
    main()
