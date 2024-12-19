#include "../include/networks.h"

int countingLine (string& fin) { // ф-ия подсчёта строк в файле
    ifstream file;
    file.open(fin);
    int countline = 0;
    string line;
    while(getline(file, line)) {
        countline++;
    }
    file.close();
    return countline;
}

string fileread (string& filename) { // чтение из файла
    string result, str;
    ifstream fileinput;
    fileinput.open(filename);
    while (getline(fileinput, str)) {
        result += str + '\n';
    }
    result.pop_back();
    fileinput.close();
    return result;
}

void filerec (string& filename, string data) { // запись в файл
    ofstream fileoutput;
    fileoutput.open(filename);
    fileoutput << data;
    fileoutput.close();
}

string BaseDate::checkcommand(string& command) {
    if (command.substr(0, 11) == "INSERT INTO") {
        command.erase(0, 12);
        return Insert(command);
    } else if (command.substr(0, 11) == "DELETE FROM") {
        command.erase(0, 12);
        return Delete(command); 
    } else if (command.substr(0, 6) == "SELECT") {
        command.erase(0, 7);
        return isValidSelect(command);
    } else if (command == "STOP") {
        exit(0);
    } else {
        return "Ошибка, неизвестная команда!";
    }
}

int main() {

    BaseDate airport;

    airport.parser();
    airport.createdirect();

    createServer(airport);
    
    return 0;
}