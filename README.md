# mdread - Terminal Markdown Renderer

A lightweight, fast terminal-based Markdown renderer with ANSI color support, written in C++. Transform your markdown files into beautifully formatted terminal output with syntax highlighting and proper Unicode support.

![License](https://img.shields.io/badge/license-MIT-blue.svg)

## Features

- 🎨 Rich ANSI color formatting
- 📑 Supports common Markdown elements:
  - Headers (levels 1-6)
  - Code blocks with syntax highlighting
  - Lists (ordered and unordered)
  - Blockquotes
  - Bold and italic text
  - Links and images
  - Horizontal rules
- 📏 Auto-adjusts to terminal width
- 🌈 UTF-8 support for special characters
- ⚡ Fast and lightweight

## Installation

### Prerequisites

- CMake (3.10 or higher)
- C++ compiler with C++17 support
- Make

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/mdread.git
cd mdread

# Create build directory and build
mkdir build
cd build
cmake ..
make
```

## Usage

```bash
./mdread <filename.md>
```

Example:
```bash
./mdread README.md
```

## Example Output

When you render a markdown file, the output will be colorized and formatted:

- Headers are highlighted in yellow (level 1) or cyan (levels 2-6)
- Code blocks are displayed with a green border and reversed text
- Links are shown in blue and underlined
- Blockquotes are prefixed with a yellow vertical bar
- Lists are properly indented with bullets or numbers
- Bold text is highlighted
- Italic text is underlined

## Contributing

Contributions are welcome! Feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Originally started as a Python project, now rewritten in C++ for better performance
- Inspired by terminal-based markdown viewers and the need for a lightweight solution
- Developed with the assistance of GitHub Copilot 🤖
