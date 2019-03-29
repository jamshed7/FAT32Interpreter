#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <csignal>
#include <fstream>
#include <cstdint>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define maxCommandSize 255
#define maxNumArguments 5
#define whitespace " \t\n"

#define dot "."

void signalHandler(int signum);

std::FILE* fp;

uint16_t BPB_RsvdSecCnt;
uint16_t BPB_BytsPerSec;
uint8_t BPB_SecPerClus;
uint32_t BPB_FATSz32;
uint8_t BPB_NumFATs;

char BS_OEMName[8];
int16_t BPB_RootEntCnt;
char BS_VolLab[11];
int32_t BPB_RootClus;

int32_t RootDirSectors = 0;
int32_t FirstDataSector = 0;
int32_t FirstSectorofCluster = 0;

//directory struct from FAT32 layout slide
struct  __attribute__((__packed__)) DirectoryEntry
{
	char              DIR_Name[11];
	uint8_t           DIR_Attr;
	uint8_t           Unused1[8];
	uint16_t          DIR_FirstClusterHigh;
	uint8_t           Unused2[4];
	uint16_t          DIR_FirstClusterLow;
	uint32_t          DIR_FileSize;
};
struct DirectoryEntry dir[16];


//Reference: FAT32.pdf
//	Returns the starting address of a block of data given the sector number
//	corresponding to that data block
int LBAToOffset (int32_t sector){
	return (( sector -2 ) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt)
					+ (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}


//Reference: FAT32.pdf
//	Given a logical block address, look up into the first FAT and return the
//	logical block address of the block in the file
//	Return -1 if when no further blocks to return
int16_t NextLB( uint32_t sector){
	uint32_t FATAddress = ( BPB_BytsPerSec * BPB_RsvdSecCnt ) + ( sector * 4 );
	int16_t val;
	fseek(fp, FATAddress, SEEK_SET);
	fread(&val, 2, 1, fp);
	return val;
}


int main(){
    std::string commandLineInput;
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTSTP, signalHandler);

		bool file_is_open = false;


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
        if(tokenizedInput[0] == "open"){

					if (file_is_open){
						std::cout << "Error: A file is already open!" << '\n';
						continue;
					}
					file_is_open = true;

          fp = std::fopen(tokenizedInput[1].c_str(), "r");

          if(fp == NULL){
            printf("Error: FAT32 image file not found!\n");
          }
          else{
            printf("FAT32 image file sucessfully opened.\n");
          }

					//Get values for all our Variables
					fseek(fp, 3, SEEK_SET);
          fread(&BS_OEMName, 8, 1, fp);

					fseek(fp, 71, SEEK_SET);
          fread(&BS_VolLab, 11, 1, fp);

					fseek(fp, 17, SEEK_SET);
          fread(&BPB_RootEntCnt, 2, 1, fp);		//always 0 for FAT32

					fseek(fp, 44, SEEK_SET);
          fread(&BPB_RootClus, 4, 1, fp);			//may not be used

					//The following needs to be displayed in info command
					fseek(fp, 11, SEEK_SET);
          fread(&BPB_BytsPerSec, 2, 1, fp);

					fseek(fp, 11, SEEK_SET);
          fread(&BPB_BytsPerSec, 2, 1, fp);

          fseek(fp, 13, SEEK_SET);
          fread(&BPB_SecPerClus, 1, 1, fp);

          fseek(fp, 14, SEEK_SET);
          fread(&BPB_RsvdSecCnt, 2, 1, fp);

          fseek(fp, 16, SEEK_SET);
          fread(&BPB_NumFATs, 1, 1, fp);

          fseek(fp, 36, SEEK_SET);
          fread(&BPB_FATSz32, 4, 1, fp);
					//info ends here

          //specify root directory address
          int directory_address = 1049600;
          fseek( fp, directory_address, SEEK_SET );

					//	read the contents of the cluster
					//	intialize DirectoryEntry structs
          for( int i = 0 ; i < 16 ; i++ )
  				{
  					memset(&dir[i],0,32);
  					fread(&dir[i],32,1,fp);
  				}

        }

        //Close file command
        if (tokenizedInput[0] == "close"){
          std::fclose(fp);
					file_is_open = false;
        }

        //Info Command
        if(tokenizedInput[0] == "info"){
          printf("                 Decimal  Hexadecimal\n");
          printf("BPB_BytsPerSec:  %d \t  %x\n", BPB_BytsPerSec, BPB_BytsPerSec);
          printf("BPB_SecPerClus:  %d \t  %x\n", BPB_SecPerClus, BPB_SecPerClus);
          printf("BPB_RsvdSecCnt:  %d \t  %x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
          printf("BPB_NumFATs:     %d \t  %x\n", BPB_NumFATs, BPB_NumFATs);
          printf("BPB_FATSz32:     %d \t  %x\n", BPB_FATSz32, BPB_FATSz32);
        }


        //ls command
        if( tokenizedInput[0] == "ls" ){

          for( int i = 0; i < 16; i++ ){

            if( (dir[i].DIR_Attr == 1 || dir[i].DIR_Attr == 16 ||
                dir[i].DIR_Attr == 32) && (dir[i].DIR_Name[0] != 0) ){

									char temp[12];
									memset(temp, 0, 12);
									// size 12 to accomodate NULL terminator
									strncpy(temp, dir[i].DIR_Name, 11);
									//convert back to string for printing
									std::string name = temp;

                  if(!name.empty()){
                    std::cout << i << ": " + name << std::endl;
                  }
                }
          }
        }//ls ends here

    }
    return 0;
}

void signalHandler(int signum){}
