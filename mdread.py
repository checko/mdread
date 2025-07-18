import sys
import re
import os
import tty
import termios

def get_terminal_size():
    try:
        return os.get_terminal_size().lines, os.get_terminal_size().columns
    except OSError:
        return 24, 80

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
    _, width = get_terminal_size()
    in_code_block = False
    rendered_lines = []
    
    for line in content.splitlines():
        if line.strip() == '```':
            in_code_block = not in_code_block
            rendered_lines.append(Ansi.GREEN + '─' * width + Ansi.RESET)
            continue

        if in_code_block:
            rendered_lines.append(Ansi.REVERSE + line.ljust(width) + Ansi.RESET)
            continue

        if line.startswith('#'):
            level = len(line.split(' ')[0])
            text = line[level:].strip()
            if level == 1:
                rendered_lines.append("")
                rendered_lines.append(Ansi.BOLD + Ansi.YELLOW + text.center(width) + Ansi.RESET)
                rendered_lines.append((Ansi.YELLOW + '=' * len(text) + Ansi.RESET).center(width))
            else:
                rendered_lines.append("")
                rendered_lines.append(Ansi.BOLD + Ansi.CYAN + text + Ansi.RESET)
            continue

        if line.strip() in ('---', '***', '___'):
            rendered_lines.append(Ansi.BOLD + '─' * width + Ansi.RESET)
            continue

        if line.startswith('>'):
            rendered_lines.append(Ansi.YELLOW + "|" + Ansi.RESET + " " + line[1:].strip())
            continue

        if line.strip().startswith('* '):
            rendered_lines.append("  • " + line.strip()[2:])
            continue
        if line.strip().startswith('- '):
            rendered_lines.append("  • " + line.strip()[2:])
            continue
        
        match = re.match(r'^(\d+)\.\s+(.*)', line.strip())
        if match:
            num, item = match.groups()
            rendered_lines.append("  " + num + ". " + item)
            continue

        line = re.sub(r'!\[(.*?)\]\((.*?)\)', lambda m: Ansi.BOLD + '[Image: ' + m.group(1) + ']' + Ansi.RESET + ' (' + Ansi.UNDERLINE + m.group(2) + Ansi.RESET + ')', line)
        line = re.sub(r'\[(.*?)\]\((.*?)\)', lambda m: m.group(1) + ' (' + Ansi.UNDERLINE + Ansi.BLUE + m.group(2) + Ansi.RESET + ')', line)
        line = re.sub(r'\*\*(.*?)\*\*|__(.*?)__', lambda m: Ansi.BOLD + (m.group(1) or m.group(2)) + Ansi.RESET, line)
        line = re.sub(r'\*(.*?)\*|_(.*?)_', lambda m: Ansi.UNDERLINE + (m.group(1) or m.group(2)) + Ansi.RESET, line)
        line = re.sub(r'`(.*?)`', lambda m: Ansi.REVERSE + m.group(1) + Ansi.RESET, line)

        rendered_lines.append(line)
    return rendered_lines

def get_char():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setcbreak(fd)
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

def pager(lines):
    height, _ = get_terminal_size()
    top = 0
    
    while True:
        os.system('clear')
        
        for i in range(height -1):
            if top + i < len(lines):
                print(lines[top + i])
        
        print(Ansi.REVERSE + f"Line {top+1}/{len(lines)} (q: quit, j/k: scroll, f/b: page)".ljust(_) + Ansi.RESET, end="")

        ch = get_char()

        if ch == 'q':
            os.system('clear')
            break
        elif ch == 'j':
            if top + height < len(lines) + 1:
                top += 1
        elif ch == 'k':
            if top > 0:
                top -= 1
        elif ch == 'f' or ch == ' ':
            top += height - 1
            if top >= len(lines):
                top = len(lines) - (height - 1)
            if top < 0:
                top = 0

        elif ch == 'b':
            top -= height - 1
            if top < 0:
                top = 0

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
        
        lines = render_markdown(content)
        pager(lines)

    except Exception as e:
        print(Ansi.RED + "Error reading or rendering file: " + str(e) + Ansi.RESET)
        sys.exit(1)

if __name__ == "__main__":
    main()
