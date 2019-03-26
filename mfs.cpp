#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <csignal>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define maxCommandSize 255
#define maxNumArguments 5
#define whitespace " \t\n"


//std::ifstream FAT32Img;
std::FILE* fp;

uint16_t BPB_RsvdSecCnt;
uint16_t BPB_BytsPerSec;
uint8_t BPB_SecPerClus;
uint32_t BPB_FATSz32;
uint8_t BPB_NumFATs;


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


        //Exit command
        if(tokenizedInput[0] == "exit" || tokenizedInput[0] == "quit"){
            exit(0);
        }

        //Open file command
        if(tokenizedInput[0] == "open")
        {
          int n = tokenizedInput[1].length();
          char inputFilename[n + 1];
          strcpy(inputFilename, tokenizedInput[1].c_str());

          fp = std::fopen(inputFilename, "r");

          if(fp == NULL)
          {
            printf("Error: FAT32 image file not found!\n");
          }
          else
          {
            printf("FAT32 image file sucessfully opened.\n");
          }
        }

        //Close file command
        if (tokenizedInput[0] == "close")
        {
          std::fclose(fp);
        }

        //Info Command
        if( tokenizedInput[0] == "info")
        {
          fseek(fp, 11, SEEK_SET);
          fread(&BPB_BytsPerSec, 2, 1, fp);

          printf("BPB_BytsPerSec: %d \t %x\n", BPB_BytsPerSec, BPB_BytsPerSec);
        }

    }
    return 0;
}

void signalHandler(int signum){}
