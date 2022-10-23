import sys
import re

def main():
    args = sys.argv[1:]
    input_filename = args[0]
    input_text = ''
    with open(input_filename, 'r') as input_file:
        input_text = input_file.read()

    input_text = '.org $7000; Initialize offset to beginning of process ram\n' + input_text
    # input_text = re.sub(r"\t\.fopt.*?\n", "", input_text)
    # input_text = re.sub(r"\t\.export.*?\n", "", input_text)

    with open(input_filename, 'w') as output_file:
        output_file.write(input_text)

if __name__ == '__main__':
    main()
