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
#define rootAddress 1049600
#define dot "."

std::FILE *fp;

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

int directoryAddress;

//directory struct from FAT32 layout slide
struct __attribute__((__packed__)) DirectoryEntry
{
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstClusterHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize;
};
struct DirectoryEntry dir[16];

void signalHandler(int signum);
//Reference: FAT32.pdf
//	Returns the starting address of a block of data given the sector number
//	corresponding to that data block
int LBAToOffset(int32_t sector);
//Reference: FAT32.pdf
//	Given a logical block address, look up into the first FAT and return the
//	logical block address of the block in the file
//	Return -1 if when no further blocks to return
int16_t NextLB(uint32_t sector);
bool caseInsensitiveCompare(std::string A, std::string B);
std::string removeGarbage(char str[11]);
std::string stringExpand(std::string str);
void getFATToPWD(std::string file);

int main()
{
  std::string commandLineInput;
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTSTP, signalHandler);

  bool file_is_open = false;

  while (true)
  {

    std::cout << "mfs>";
    std::getline(std::cin, commandLineInput);
    std::vector<std::string> tokenizedInput;
    boost::split(tokenizedInput, commandLineInput, boost::is_any_of(whitespace));

    //Exit command
    if (tokenizedInput[0] == "exit" || tokenizedInput[0] == "quit")
    {
      exit(0);
    }

    //Open file command
    if (tokenizedInput[0] == "open")
    {

      if (file_is_open)
      {
        std::cout << "Error: File system image already open." << '\n';
        continue;
      }
      file_is_open = true;

      fp = std::fopen(tokenizedInput[1].c_str(), "rb+");

      if (fp == NULL)
      {
        printf("Error: FAT32 image file not found!\n");
      }
      else
      {
        printf("FAT32 image file sucessfully opened.\n"); //remove later on2
      }

      //Get values for all our Variables
      fseek(fp, 3, SEEK_SET);
      fread(&BS_OEMName, 8, 1, fp);

      fseek(fp, 71, SEEK_SET);
      fread(&BS_VolLab, 11, 1, fp);

      fseek(fp, 17, SEEK_SET);
      fread(&BPB_RootEntCnt, 2, 1, fp); //always 0 for FAT32

      fseek(fp, 44, SEEK_SET);
      fread(&BPB_RootClus, 4, 1, fp); //may not be used

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

      //	initialize a variable to store the address of current working directory
      //	when image is first opened set current directory to root address
      directoryAddress = rootAddress;
      fseek(fp, directoryAddress, SEEK_SET);

      //	read the contents of the cluster
      //	& intialize DirectoryEntry struct
      for (int i = 0; i < 16; i++)
      {
        memset(&dir[i], 0, 32);
        fread(&dir[i], 32, 1, fp);
      }
    }

    //Close file command
    if (tokenizedInput[0] == "close")
    {
      std::fclose(fp);
      file_is_open = false;
    }

    //Info Command
    if (tokenizedInput[0] == "info")
    {
      printf("                 Decimal  Hexadecimal\n");
      printf("BPB_BytsPerSec:  %d \t  %x\n", BPB_BytsPerSec, BPB_BytsPerSec);
      printf("BPB_SecPerClus:  %d \t  %x\n", BPB_SecPerClus, BPB_SecPerClus);
      printf("BPB_RsvdSecCnt:  %d \t  %x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
      printf("BPB_NumFATs:     %d \t  %x\n", BPB_NumFATs, BPB_NumFATs);
      printf("BPB_FATSz32:     %d \t  %x\n", BPB_FATSz32, BPB_FATSz32);
    }

    //ls command
    if (tokenizedInput[0] == "ls")
    {
      int counter = 1;
      for (int i = 0; i < 16; i++)
      {

        if ((dir[i].DIR_Attr == 1 || dir[i].DIR_Attr == 16 ||
             dir[i].DIR_Attr == 32) &&
            (dir[i].DIR_Name[0] != 0))
        {

          char temp[12];
          memset(temp, 0, 12);
          // size 12 to accomodate NULL terminator
          strncpy(temp, dir[i].DIR_Name, 11);
          //convert back to string for printing
          std::string name = temp;

          if (!name.empty())
          {
            std::cout << counter << ": " + name << std::endl;
            ++counter;
          }
        }
      }
    } //ls ends here

    if (tokenizedInput[0] == "cd")
    {

      std::string desiredDirectory = tokenizedInput[1];
      for (int i = 0; i < 16; ++i)
      {
        std::string directoryAtCounter = removeGarbage(dir[i].DIR_Name);
        if (caseInsensitiveCompare(directoryAtCounter, desiredDirectory) == true)
        {
          directoryAddress = LBAToOffset(dir[i].DIR_FirstClusterLow);
        }
      }
      fseek(fp, directoryAddress, SEEK_SET);
      for (int i = 0; i < 16; ++i)
      {
        memset(&dir[i], 0, 32);
        fread(&dir[i], 32, 1, fp);
      }
    }
    if (tokenizedInput[0] == "get")
    {
      getFATToPWD(tokenizedInput[1]);
    }

    if (tokenizedInput[0] == "put")
    {
      int findEmptyFile = -1, freeSectorIndex = -1, counter = 0, pointer = 0;
      for(int i = 0; i < 16; ++i){
        if(dir[i].DIR_Name[0] == '\xe5' || dir[i].DIR_Name[0] == '\x00'){
          findEmptyFile = i;
          break;
        }
      }
      while(1){
        fseek(fp, (counter * 4) + (BPB_BytsPerSec * BPB_RsvdSecCnt), SEEK_SET);
        fread(&pointer, 2, 1, fp);
        if(pointer == 0){
          freeSectorIndex = counter;
          break;
        }
        ++counter;
      }
      std::cout << freeSectorIndex << std::endl;
      char stringRead[512] = "Adarsh";
      int offset = LBAToOffset(freeSectorIndex);
      fseek(fp, offset, SEEK_SET);
      fwrite(stringRead, 512, 1, fp);

      fseek(fp, directoryAddress, SEEK_SET);
      fseek(fp, findEmptyFile * 32, SEEK_CUR);

      strcpy(dir[findEmptyFile].DIR_Name, stringExpand(tokenizedInput[1]).c_str());
      dir[findEmptyFile].DIR_Attr = 16;
      dir[findEmptyFile].DIR_FirstClusterLow = freeSectorIndex;
      fwrite(&dir[findEmptyFile], 512, 1, fp);
      
    }

    //stat command
    if (tokenizedInput[0] == "stat")
    {

      std::string name = tokenizedInput[1];
      char next_name[12];

      fseek(fp, directoryAddress, SEEK_SET);
      for (int i = 0; i < 16; i++)
      {
        memset(&dir[i], 0, 32);
        fread(&dir[i], 32, 1, fp);
      }

      for (int i = 0; i < 16; i++)
      {
        strncpy(next_name, dir[i].DIR_Name, 11);
        if ((dir[i].DIR_Attr == 16 || dir[i].DIR_Attr == 32) && name != next_name)
        {
          printf("Attribute: %d\n", dir[i].DIR_Attr);
          printf("File Size: %d\n", dir[i].DIR_FileSize);
          printf("Starting Cluster Number: %d\n", dir[i].DIR_FirstClusterLow);
        }
      }
    }
  }
  return 0;
}

int LBAToOffset(int32_t sector)
{
  return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

int16_t NextLB(uint32_t sector)
{
  uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector * 4);
  int16_t val;
  fseek(fp, FATAddress, SEEK_SET);
  fread(&val, 2, 1, fp);
  return val;
}

bool caseInsensitiveCompare(std::string A, std::string B)
{
  if (A.size() != B.size())
  {
    return false;
  }
  else
  {
    for (int i = 0; i < A.size(); ++i)
    {
      if (std::tolower(A[i]) != std::tolower(B[i]))
      {
        return false;
      }
    }
  }
  return true;
}

std::string removeGarbage(char str[11])
{
  std::string cleanString;
  char temp[12];
  strncpy(temp, str, 11);
  cleanString = temp;
  boost::trim(cleanString);
  return cleanString;
}

std::string stringExpand(std::string str)
{
  std::vector<std::string> fileNameSplit;
  boost::split(fileNameSplit, str, boost::is_any_of(dot));
  int requiredSpaces = 8 - fileNameSplit[0].size();
  std::string requiredSpaceString;
  for (int i = 0; i < requiredSpaces; ++i)
  {
    requiredSpaceString.append(" ");
  }
  std::string returnString = fileNameSplit[0];
  returnString.append(requiredSpaceString);
  returnString.append(fileNameSplit[1]);
  
  boost::to_upper(returnString);
  return returnString;
}

void getFATToPWD(std::string file)
{
  int size, cluster;
  std::string transformedString = stringExpand(file);
  for (int i = 0; i < 16; ++i)
  {
    std::string directoryAtCounter = removeGarbage(dir[i].DIR_Name);
    if (caseInsensitiveCompare(directoryAtCounter, transformedString) == true && dir[i].DIR_Attr == 32)
    {
      size = dir[i].DIR_FileSize;
      cluster = dir[i].DIR_FirstClusterLow;
    }
  }
  FILE *pwdToFAT32 = fopen(file.c_str(), "wb");

  fseek(fp, LBAToOffset(cluster), SEEK_SET);

  char stringRead[512];
  int nextBlock = cluster;
  while (size > 512)
  {
    fread(stringRead, 512, 1, fp);
    fwrite(stringRead, 512, 1, pwdToFAT32);
    nextBlock = NextLB(nextBlock);
    fseek(fp, LBAToOffset(nextBlock), SEEK_SET);
    size -= 512;
  }

  fclose(pwdToFAT32);
}

void signalHandler(int signum) {}
