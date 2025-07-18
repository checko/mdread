#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sys/ioctl.h>
#include <unistd.h>
#include <clocale>
#include <termios.h>

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

void get_terminal_size(int& rows, int& cols) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    rows = w.ws_row > 0 ? w.ws_row : 24;
    cols = w.ws_col > 0 ? w.ws_col : 80;
}

std::string center_text(const std::string& text, int width) {
    int padding = (width - text.length()) / 2;
    return std::string(padding, ' ') + text + std::string(padding, ' ');
}

std::vector<std::string> render_markdown(const std::string& content) {
    int rows, width;
    get_terminal_size(rows, width);
    bool in_code_block = false;
    std::vector<std::string> rendered_lines;
    std::string line;
    std::istringstream stream(content);

    while (std::getline(stream, line)) {
        if (line.substr(0, 3) == "```") {
            in_code_block = !in_code_block;
            std::string separator;
            for (int i = 0; i < width; ++i) {
                separator += "─";
            }
            rendered_lines.push_back(Ansi::GREEN + separator + Ansi::RESET);
            continue;
        }

        if (in_code_block) {
            std::string spaces = (line.length() < width) ? std::string(width - line.length(), ' ') : "";
            rendered_lines.push_back(Ansi::REVERSE + line + spaces + Ansi::RESET);
            continue;
        }

        if (line[0] == '#') {
            size_t level = line.find_first_not_of('#');
            std::string text = line.substr(level + 1);
            if (level == 1) {
                rendered_lines.push_back("");
                rendered_lines.push_back(Ansi::BOLD + Ansi::YELLOW + center_text(text, width) + Ansi::RESET);
                rendered_lines.push_back(center_text(std::string(text.length(), '='), width));
            } else {
                rendered_lines.push_back("");
                rendered_lines.push_back(Ansi::BOLD + Ansi::CYAN + text + Ansi::RESET);
            }
            continue;
        }

        if (line == "---" || line == "***" || line == "___") {
            std::string separator;
            for (int i = 0; i < width; ++i) {
                separator += "─";
            }
            rendered_lines.push_back(Ansi::BOLD + separator + Ansi::RESET);
            continue;
        }

        if (line[0] == '>') {
            rendered_lines.push_back(Ansi::YELLOW + "|" + Ansi::RESET + " " + line.substr(1));
            continue;
        }

        if (line.substr(0, 2) == "* " || line.substr(0, 2) == "- ") {
            rendered_lines.push_back("  • " + line.substr(2));
            continue;
        }

        std::regex ordered_list_regex(R"(^\d+\.\s+.*)");
        if (std::regex_match(line, ordered_list_regex)) {
            size_t dot_pos = line.find('.');
            rendered_lines.push_back("  " + line.substr(0, dot_pos + 1) + " " + line.substr(dot_pos + 2));
            continue;
        }

        std::string processed = line;
        processed = std::regex_replace(processed, std::regex(R"(!\[(.*?)\]\((.*?)\))"), Ansi::BOLD + std::string("[Image: $1]") + Ansi::RESET + " (" + Ansi::UNDERLINE + "$2" + Ansi::RESET + ")");
        processed = std::regex_replace(processed, std::regex(R"(\[(.*?)\]\((.*?)\))"), "$1 (" + Ansi::UNDERLINE + Ansi::BLUE + "$2" + Ansi::RESET + ")");
        processed = std::regex_replace(processed, std::regex(R"(\*\*(.*?)\*\*|__(.*?)__)"), Ansi::BOLD + "$1$2" + Ansi::RESET);
        processed = std::regex_replace(processed, std::regex(R"(\*(.*?)\*|_(.*?)_)"), Ansi::UNDERLINE + "$1$2" + Ansi::RESET);
        processed = std::regex_replace(processed, std::regex(R"(`(.*?)`)"), Ansi::REVERSE + "$1" + Ansi::RESET);
        rendered_lines.push_back(processed);
    }
    return rendered_lines;
}

int get_key() {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    
    // Handle arrow keys (escape sequences)
    if (ch == 27) { // ESC
        ch = getchar();
        if (ch == 91) { // [
            ch = getchar();
            if (ch == 65) { // Up arrow
                tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
                return 1000; // Custom code for up arrow
            } else if (ch == 66) { // Down arrow
                tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
                return 1001; // Custom code for down arrow
            }
        }
        tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
        return 27; // Return ESC if not an arrow key
    }
    
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

void pager(const std::vector<std::string>& lines) {
    int height, width;
    get_terminal_size(height, width);
    int top = 0;
    const int page_size = height > 1 ? height - 1 : 1;
    const int max_top = lines.size() > page_size ? lines.size() - page_size : 0;

    if (lines.empty()) {
        system("clear");
        std::string status = "File is empty. (q: quit)";
        std::cout << Ansi::REVERSE << status << std::string(width - status.length(), ' ') << Ansi::RESET << std::flush;
        while(get_key() != 'q');
        system("clear");
        return;
    }

    while (true) {
        system("clear");
        for (int i = 0; i < page_size; ++i) {
            if (top + i < lines.size()) {
                std::cout << lines[top + i] << std::endl;
            }
        }

        std::string status;
        if (top == 0 && max_top == 0) {
            status = "(TOP/END)";
        } else if (top == 0) {
            status = "(TOP)";
        } else if (top == max_top) {
            status = "(END)";
        } else {
            status = "Line " + std::to_string(top + 1) + "/" + std::to_string(lines.size());
        }
        status += " (q:quit, ↑↓/n/p:scroll, f/b:page)";
        if (status.length() > width) {
            status = status.substr(0, width);
        }
        std::cout << Ansi::REVERSE << status << std::string(width - status.length(), ' ') << Ansi::RESET << std::flush;

        int ch = get_key();

        if (ch == 'q') {
            system("clear");
            break;
        } else if (ch == 'n' || ch == 1001) { // 'n' or down arrow
            if (top < max_top) {
                top++;
            }
        } else if (ch == 'p' || ch == 1000) { // 'p' or up arrow
            if (top > 0) {
                top--;
            }
        } else if (ch == 'f' || ch == ' ') {
            if (top < max_top) {
                top += page_size;
                if (top > max_top) {
                    top = max_top;
                }
            }
        } else if (ch == 'b') {
            if (top > 0) {
                top -= page_size;
                if (top < 0) {
                    top = 0;
                }
            }
        }
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
        std::vector<std::string> lines = render_markdown(content);
        pager(lines);
    } catch (const std::exception& e) {
        std::cerr << Ansi::RED << "Error reading or rendering file: " << e.what() << Ansi::RESET << std::endl;
        return 1;
    }

    return 0;
}