import sys
import re
import os

def get_terminal_width():
    try:
        return os.get_terminal_size().columns
    except OSError:
        return 80

class Ansi:
    RESET = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    BLUE = "\033[34m"
    MAGENTA = "\033[35m"
    CYAN = "\033[36m"
    REVERSE = "\033[7m"

def render_markdown(content):
    width = get_terminal_width()
    in_code_block = False
    
    for line in content.splitlines():
        # Handle code blocks
        if line.strip() == '```':
            in_code_block = not in_code_block
            print(Ansi.GREEN + '─' * width + Ansi.RESET)
            continue

        if in_code_block:
            print(Ansi.REVERSE + line.ljust(width) + Ansi.RESET)
            continue

        # Headings (e.g., #, ##, ###)
        if line.startswith('#'):
            level = len(line.split(' ')[0])
            text = line[level:].strip()
            if level == 1:
                print("\n" + Ansi.BOLD + Ansi.YELLOW + text.center(width) + Ansi.RESET)
                print((Ansi.YELLOW + '=' * len(text) + Ansi.RESET).center(width))
            else:
                print("\n" + Ansi.BOLD + Ansi.CYAN + text + Ansi.RESET)
            continue

        # Horizontal Rule
        if line.strip() in ('---', '***', '___'):
            print(Ansi.BOLD + '─' * width + Ansi.RESET)
            continue

        # Blockquotes
        if line.startswith('>'):
            print(Ansi.YELLOW + "|" + Ansi.RESET + " " + line[1:].strip())
            continue

        # Unordered Lists
        if line.strip().startswith('* '):
            print("  • " + line.strip()[2:])
            continue
        if line.strip().startswith('- '):
            print("  • " + line.strip()[2:])
            continue
        
        # Ordered Lists
        match = re.match(r'^(\d+)\.\s+(.*)', line.strip())
        if match:
            num, item = match.groups()
            print("  " + num + ". " + item)
            continue

        # Images: ![alt](src)
        line = re.sub(r'!\[(.*?)\]\((.*?)\)', lambda m: Ansi.BOLD + '[Image: ' + m.group(1) + ']' + Ansi.RESET + ' (' + Ansi.UNDERLINE + m.group(2) + Ansi.RESET + ')', line)
        
        # Links: [text](url)
        line = re.sub(r'\[(.*?)\]\((.*?)\)', lambda m: m.group(1) + ' (' + Ansi.UNDERLINE + Ansi.BLUE + m.group(2) + Ansi.RESET + ')', line)

        # Bold: **text** or __text__
        line = re.sub(r'\*\*(.*?)\*\*|__(.*?)__', lambda m: Ansi.BOLD + (m.group(1) or m.group(2)) + Ansi.RESET, line)
        
        # Italics: *text* or _text_ (using underline)
        line = re.sub(r'\*(.*?)\*|_(.*?)_', lambda m: Ansi.UNDERLINE + (m.group(1) or m.group(2)) + Ansi.RESET, line)

        # Inline code: `code`
        line = re.sub(r'`(.*?)`', lambda m: Ansi.REVERSE + m.group(1) + Ansi.RESET, line)

        print(line)

def main():
    if len(sys.argv) != 2:
        print(Ansi.RED + "Usage: python3 " + sys.argv[0] + " <filename.md>" + Ansi.RESET)
        sys.exit(1)

    filepath = sys.argv[1]
    if not os.path.exists(filepath):
        print(Ansi.RED + "Error: File not found at '" + filepath + "'" + Ansi.RESET)
        sys.exit(1)

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        render_markdown(content)
    except Exception as e:
        print(Ansi.RED + "Error reading or rendering file: " + str(e) + Ansi.RESET)
        sys.exit(1)

if __name__ == "__main__":
    main()