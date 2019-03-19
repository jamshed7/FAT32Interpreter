#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <csignal>

#define maxCommandSize 255
#define maxNumArguments 5
#define whitespace " \t\n"

using namespace std;

void signalHandler(int signum);


int main(){
    std::string commandLineInput;
    signal(SIGINT, signalHandler);
    signal(SIGTSTP, signalHandler);
    
    while(true){
        std::cout << "mfs>";
        std::getline(std::cin,commandLineInput);
        vector<std::string> tokenizedInput;
        boost::split(tokenizedInput, commandLineInput, boost::is_any_of(whitespace));

        for(int i = 0; i < tokenizedInput.size(); ++i){
            std::cout << tokenizedInput[i] << std::endl;
        }


    }
    return 0;
}

void signalHandler(int signum){
    switch(signum){
        case SIGINT:
            //std::cout << "SIGINT" << std::endl;
            break;
        case SIGTSTP:
            //std::cout << "SIGTSTP" << std::endl;
            break;
        default:
            //std::cout << "Unrecognized" << std::endl;
            break;
    }
}