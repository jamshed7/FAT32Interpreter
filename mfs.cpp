#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <vector>

#define maxCommandSize 255
#define maxNumArguments 5
#define whitespace " \t\n"

using namespace std;

int main(){
    std::string commandLineInput;
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