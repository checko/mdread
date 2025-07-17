#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sys/ioctl.h>
#include <unistd.h>
#include <clocale>

class Ansi {
public:
    static const std::string RESET;
    static const std::string BOLD;
    static const std::string UNDERLINE;
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string REVERSE;
};

const std::string Ansi::RESET = "\033[0m";
const std::string Ansi::BOLD = "\033[1m";
const std::string Ansi::UNDERLINE = "\033[4m";
const std::string Ansi::RED = "\033[31m";
const std::string Ansi::GREEN = "\033[32m";
const std::string Ansi::YELLOW = "\033[33m";
const std::string Ansi::BLUE = "\033[34m";
const std::string Ansi::MAGENTA = "\033[35m";
const std::string Ansi::CYAN = "\033[36m";
const std::string Ansi::REVERSE = "\033[7m";

int get_terminal_width() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col > 0 ? w.ws_col : 80;
}

std::string center_text(const std::string& text, int width) {
    int padding = (width - text.length()) / 2;
    return std::string(padding, ' ') + text + std::string(padding, ' ');
}

std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

void render_markdown(const std::string& content) {
    try {
        int width = get_terminal_width();
        bool in_code_block = false;
        std::string line;
        std::istringstream stream(content);

    while (std::getline(stream, line)) {
        // Handle code blocks
        if (line.substr(0, 3) == "```") {
            in_code_block = !in_code_block;
            std::cout << Ansi::GREEN;
            for(int i = 0; i < width; i++) std::cout << "─";
            std::cout << Ansi::RESET << std::endl;
            continue;
        }

        if (in_code_block) {
            // In code blocks, print the line directly without any processing
            std::string spaces = (line.length() < width) ? std::string(width - line.length(), ' ') : "";
            std::cout << Ansi::REVERSE << line << spaces << Ansi::RESET << std::endl;
            continue;
        }

        // Headings
        if (line[0] == '#') {
            size_t level = line.find_first_not_of('#');
            std::string text = line.substr(level + 1);
            if (level == 1) {
                std::cout << "\n" << Ansi::BOLD << Ansi::YELLOW << center_text(text, width) << Ansi::RESET << std::endl;
                std::cout << center_text(std::string(text.length(), '='), width) << std::endl;
            } else {
                std::cout << "\n" << Ansi::BOLD << Ansi::CYAN << text << Ansi::RESET << std::endl;
            }
            continue;
        }

        // Horizontal Rule
        if (line == "---" || line == "***" || line == "___") {
            std::cout << Ansi::BOLD;
            for(int i = 0; i < width; i++) std::cout << "─";
            std::cout << Ansi::RESET << std::endl;
            continue;
        }

        // Blockquotes
        if (line[0] == '>') {
            std::cout << Ansi::YELLOW << "|" << Ansi::RESET << " " << line.substr(1) << std::endl;
            continue;
        }

        // Lists
        if (line.substr(0, 2) == "* " || line.substr(0, 2) == "- ") {
            std::cout << "  • " << line.substr(2) << std::endl;
            continue;
        }

        // Ordered Lists
        std::regex ordered_list_regex(R"(^\d+\.\s+.*)");
        if (std::regex_match(line, ordered_list_regex)) {
            size_t dot_pos = line.find('.');
            std::cout << "  " << line.substr(0, dot_pos + 1) << " " 
                     << line.substr(dot_pos + 2) << std::endl;
            continue;
        }

        // Inline formatting
        std::string processed = line;

        // Images
        processed = std::regex_replace(processed, 
            std::regex(R"(!\[(.*?)\]\((.*?)\))"),
            Ansi::BOLD + std::string("[Image: $1]") + Ansi::RESET + 
            " (" + Ansi::UNDERLINE + "$2" + Ansi::RESET + ")");

        // Links
        processed = std::regex_replace(processed, 
            std::regex(R"(\[(.*?)\]\((.*?)\))"),
            "$1 (" + Ansi::UNDERLINE + Ansi::BLUE + "$2" + Ansi::RESET + ")");

        // Bold
        processed = std::regex_replace(processed, 
            std::regex(R"(\*\*(.*?)\*\*|__(.*?)__)"),
            Ansi::BOLD + "$1$2" + Ansi::RESET);

        // Italics
        processed = std::regex_replace(processed, 
            std::regex(R"(\*(.*?)\*|_(.*?)_)"),
            Ansi::UNDERLINE + "$1$2" + Ansi::RESET);

        // Inline code
        processed = std::regex_replace(processed, 
            std::regex(R"(`(.*?)`)"),
            Ansi::REVERSE + "$1" + Ansi::RESET);

        std::cout << processed << std::endl;
    }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error processing markdown content: " + std::string(e.what()));
    }
}

int main(int argc, char* argv[]) {
    std::setlocale(LC_ALL, "en_US.UTF-8");
    if (argc != 2) {
        std::cerr << Ansi::RED << "Usage: " << argv[0] << " <filename.md>" << Ansi::RESET << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << Ansi::RED << "Error: File not found at '" << filepath << "'" << Ansi::RESET << std::endl;
        return 1;
    }

    try {
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        render_markdown(content);
    } catch (const std::exception& e) {
        std::cerr << Ansi::RED << "Error reading or rendering file: " << e.what() << Ansi::RESET << std::endl;
        return 1;
    }

    return 0;
}
