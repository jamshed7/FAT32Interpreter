#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <csignal>
#include <fstream>
#include <cstdint>

#define maxCommandSize 255
#define maxNumArguments 5
#define whitespace " \t\n"

std::ifstream FAT32Img;



void signalHandler(int signum);


int main(){
    std::string commandLineInput;
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTSTP, signalHandler);

    while(true){
        std::cout << "mfs>";
        std::getline(std::cin,commandLineInput);
        std::vector<std::string> tokenizedInput;
        boost::split(tokenizedInput, commandLineInput, boost::is_any_of(whitespace));
        if(tokenizedInput[0] == "exit" || tokenizedInput[0] == "quit"){
            exit(0);
        }
        if(tokenizedInput[0] == "open"){
            FAT32Img.open(tokenizedInput[1]);
        }
    }
    return 0;
}

void signalHandler(int signum){}
