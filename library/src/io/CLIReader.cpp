#include "io/CLIReader.h"
#include <iostream>
#include <string>
#include <memory>

#include "io/FileIO.h"

void print_help_error() {
    std::cout<<"Please write -h or -help to access teh help page\n";
}

// CLIREADER

#define END_READERS(str) {\
    std::cout << "Error! " << str << '\n'; \
    print_help_error();\
    reader->should_start = false;\
    return -1;\
    }

CLIReader::CLIReader(uvec2 default_size, float default_speed, std::string highscore_path)
  : should_start(false), board_size(default_size), speed_seconds(default_speed), highscore_path(highscore_path) {

    // -h / -help: help page
    argument_readers["-h"] = argument_readers["-help"] = [](CLIReader* reader, std::string value) {
        std::cout<<"Terminal Snake Clone made by Mateusz Kubiak and Gosia Komorowska\n"
        << "and further developed by Mateusz Kubiak\n"
        << "@ Lodz University of Technology\n"
        << '\n'
        << "Controls:\n"
        << "WASD - change direction of the snake\n"
        << "Tab - exit the game\n"
        << '\n'
        << "Highscore:\n"
        << "Upon user confirmation, this game will save highscore in a text file ~/.terminal-snake.\n"
        << '\n'
        << "Available options:\n"
        << "-b - change game board dimensions (pattern: 'UINTxUINT' i.e. '-b 40x20').\n"
        << "-d - change difficulty (e - easy, m - medium, h - hard).\n"
        << "-s - change time duration between snake moves (double seconds).\n"
        << '\n'
        << "Options that do not run the game:\n"
        << "-H - show highscore.\n"
        << "-c - clear highscore.\n"
        << "-h (-help) - view this page.\n";
        reader->should_start = false;
        return -1;
    };

    // -H: show highscore
    argument_readers["-H"] = [](CLIReader* reader, std::string value) {
        int score = Files::load_highscore(reader->highscore_path);
        if (score == -1)
            std::cout<<"There is currently no highscore saved for this user.\n";
        else
            std::cout << "Your highscore is: " << score << '\n';
        
        reader->should_start = false;
        return -1;
    };

    // -c: clear highscore
    argument_readers["-c"] = [](CLIReader* reader, std::string value){
        if(Files::load_highscore(reader->highscore_path) == -1) {
            std::cout<<"There is no highscore to clear for current user!\n";
            reader->should_start = false;
            return -1;
        }

        std::cout<<"Do you really want to clear the highscore? Write [yes/no].\n";
        std::string response = "";
        while (response != "yes" && response != "no")
            std::cin >> response;

        if (response == "no") {
            std::cout<<"Highscore has not been cleared.\n";
            reader->should_start = false;
            return -1;
        }
        
        try {
            remove(reader->highscore_path.c_str());
            std::cout << "Highscore cleared!\n";
        }
        catch(std::exception e) {
            std::cout<<"Error! Highscore could not be cleared!\n";
        }
        
        reader->should_start = false;
        return -1;
    };

    // -b: change board dimensions
    argument_readers["-b"] = [](CLIReader* reader, std::string value){
        if (value == "")
            END_READERS("Parameter -b cannot be empty!");
        
        int x_index = value.find('x');
        if (x_index == -1)
            END_READERS("Invalid board size argument. Use pattern UINTxUINT i.e. '-b 40x20'");

        try {
            uvec2 new_board_size((unsigned)std::stoul(value.substr(0, x_index)),
                (unsigned)std::stoul(value.substr(x_index + 1, value.size())));
            
            if (new_board_size.x < 1 || new_board_size.y < 1)
                END_READERS("Invalid board size argument. Board size has to be at least 1x1");

            reader->board_size = new_board_size;
        } catch(std::invalid_argument& e) {
            END_READERS("Invalid board size argument. Use pattern UINTxUINT i.e. '-b 40x20'")
        }
    };

    // -d: change difficulty level
    argument_readers["-d"] = [](CLIReader* reader, std::string value){
        if (value == "")
            END_READERS("Difficulty argument cannot be empty. Use either e, easy, m, medium, h or hard");
        
        if (value == "e" || value == "easy") {
            reader->speed_seconds = .2f;
        }
        else if (value == "m" || value == "medium") {
            reader->speed_seconds = .15f;
        }
        else if (value == "h" || value == "hard") {
            reader->speed_seconds = .1f;
        }
        else
            END_READERS("Invalid difficulty argument. Use either s, small, m, medium, h or hard");

        return 0;
    };

    // -s: speed (float in seconds)
    argument_readers["-s"] = [](CLIReader* reader, std::string value){
        if (value == "")
            END_READERS("Error! Speed argument cannot be empty. Use a positive float fumber");

        float new_speed = 0.f;
        try {
            new_speed = std::stof(value);
            if (new_speed <= 0.f)
                throw std::invalid_argument("");
        }
        catch (std::invalid_argument e){
            END_READERS("Invalid speed argument. Use a positive float fumber");
        }

        reader->speed_seconds = new_speed;
        return 0;
    };
    
}

void CLIReader::analyse_arguments(int argc, char *argv[]) {

    should_start = true;
    bool was_previous_flag = false;
    for (unsigned i = 1; i < argc; i++) {

        // skip values after flags
        if (argv[i][0] != '-' && was_previous_flag) {
            was_previous_flag = false;
            continue;
        }
        
        // incorrect flags and values not following flags
        if (argument_readers.find(argv[i]) == argument_readers.end()) {
            std::cout<<"Error! Unrecognized parameters.\n";
            print_help_error();
            should_start = false;
            return;
        }
        
        // clear values starting with '-'
        std::string value = "";
        if (i+1 < argc && argv[i+1][0] != '-')  
            value = argv[i+1];
        
        was_previous_flag = true;

        // readers can stop further execution
        if (argument_readers.at(argv[i])(this, value) == -1) {
            return;
        }
    }
}



// GETTERS

bool CLIReader::game_should_start() {
    return should_start;
}

uvec2 CLIReader::get_board_size(){
    return board_size;
}

float CLIReader::get_speed_seconds() {
    return speed_seconds;
}
