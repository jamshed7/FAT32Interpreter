#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <csignal>
#include <fstream>

#define maxCommandSize 255
#define maxNumArguments 5
#define whitespace " \t\n"


std::ifstream FAT32Img;

void signalHandler(int signum);


int main(){
    std::string commandLineInput;
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTSTP, signalHandler);

    std::ifstream inputFile;

    while(true){
        std::cout << "mfs>";
        std::getline(std::cin,commandLineInput);
        std::vector<std::string> tokenizedInput;
        boost::split(tokenizedInput, commandLineInput, boost::is_any_of(whitespace));

        if(tokenizedInput[0] == "exit" || tokenizedInput[0] == "quit"){
            exit(0);
        }

        if(tokenizedInput[0] == "open")
        {
          inputFile.open(tokenizedInput[1]);

          if(inputFile == NULL)
          {
            printf("Error: FAT32 image file not found!\n");
          }
          else
          {
            printf("FAT32 image file sucessfully opened.\n");
          }
        }

        if (tokenizedInput[0] == "close")
        {
          inputFile.close();
        }

    }
    return 0;
}

void signalHandler(int signum){}
