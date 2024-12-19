#include "../include/commands.h"

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


void BaseDate::parse() { // ф-ия парсинга
    nlohmann::json objJson;
    ifstream fileinput;
    fileinput.open("../base_date.json");
    fileinput >> objJson;
    fileinput.close();

    if (objJson["name"].is_string()) {
    BD = objJson["name"]; // Парсим каталог 
    } else {
        cout << "Объект каталога не найден!" << endl;
        exit(-1);
    }

    rowLimits = objJson["tuples_limit"];

    // парсим подкаталоги
    if (objJson.contains("structure") && objJson["structure"].is_object()) { // проверяем, существование объекта и является ли он объектом
        for (auto elem : objJson["structure"].items()) {
            nametables.push_back(elem.key());
            
            string kolonki = elem.key() + "_pk_sequence,"; // добавление первичного ключа
            for (auto str : objJson["structure"][elem.key()].items()) {
                kolonki += str.value();
                kolonki += ',';
            }
            kolonki.pop_back(); // удаление последней запятой
            stlb.push_back(kolonki);
            fileindex.push_back(1);
        }
    } else {
        cout << "Объект подкаталогов не найден!" << endl;
        exit(-1);
    }
}

void BaseDate::mkdir() { // ф-ия формирования директории
    string command;
    command = "mkdir ../" + BD; // каталог
    system(command.c_str());

    for (int i = 0; i < nametables.size; ++i) { // подкаталоги и файлы в них
        command = "mkdir ../" + BD + "/" + nametables.getvalue(i);
        system(command.c_str());
        string filepath = "../" + BD + "/" + nametables.getvalue(i) + "/1.csv";
        ofstream file;
        file.open(filepath);
        file << stlb.getvalue(i) << endl;
        file.close();

        // Блокировка таблицы
        filepath = "../" + BD + "/" + nametables.getvalue(i) + "/" + nametables.getvalue(i) + "_lock.txt";
        file.open(filepath);
        file << "open";
        file.close();

        // ключ
        filepath = "../" + BD + "/" + nametables.getvalue(i) + "/" + nametables.getvalue(i) + "_pk_sequence.txt";
        file.open(filepath);
        file << "1";
        file.close();
    }
}

string BaseDate::checkcommand(string& command) { // ф-ия фильтрации команд
    if (command.substr(0, 11) == "insert into") {
        command.erase(0, 12);
        return isValidInsert(command);
    } else if (command.substr(0, 11) == "delete from") {
        command.erase(0, 12);
        return isValidDel(command);
    } else if (command.substr(0, 6) == "select") {
        command.erase(0, 7);
        return isValidSelect(command);
    } else if (command == "exit") {
        exit(0);
    } else return "Ошибка, неизвестная команда!"; 
}


string BaseDate::isValidInsert(string& command) { // ф-ия проверки ввода команды insert
    string table;
    int position = command.find_first_of(' ');
    if (position != -1) { // проверка синтаксиса
        table = command.substr(0, position);
        command.erase(0, position + 1);
        if (nametables.getindex(table) != -1) { // проверка таблицы
            if (command.substr(0, 7) == "values ") { // проверка values
                command.erase(0, 7);
                position = command.find_first_of(' ');
                if (position == -1) { // проверка синтаксиса ///////
                    if (command[0] == '(' && command[command.size()-1] == ')') { // проверка синтаксиса скобок и их удаление
                        command.erase(0, 1);
                        command.pop_back();
                        position = command.find(' ');
                        while (position != -1) { // удаление пробелов
                            command.erase(position);
                            position = command.find(' ');
                        }
                        return insert(table, command);
                    } else return "Ошибка, нарушен синтаксис команды!";
                } else return "Ошибка, нарушен синтаксис команды!";
            } else return "Ошибка, нарушен синтаксис команды!";
        } else return "Ошибка, нет такой таблицы!";
    } else return "Ошибка, нарушен синтаксис команды!";
}

string BaseDate::insert(string& table, string& values) { // ф-ия вставки в таблицу
    string filepath = "../" + BD + "/" + table + "/" + table + "_pk_sequence.txt";
    int index = nametables.getindex(table); // получаем индекс таблицы(aka key)
    string val = fileread(filepath);
    int valint = stoi(val);
    valint++;
    filerec(filepath, to_string(valint));

    if (checkLockTable(table)) {
        filepath = "../" + BD + "/" + table + "/" + table + "_lock.txt";
        filerec(filepath, "close");

        // вставка значений в csv, не забывая про увеличение ключа
        filepath = "../" + BD + "/" + table + "/1.csv";
        int countline = countingLine(filepath);
        int fileid = 1; // номер файла csv
        while (true) {
            if (countline == rowLimits) { // если достигнут лимит, то создаем/открываем другой файл
                fileid++;
                filepath = "../" + BD + "/" + table + "/" + to_string(fileid) + ".csv";
                if (fileindex.getvalue(index) < fileid) {
                    fileindex.replace(index, fileid);
                }
            } else break;
            countline = countingLine(filepath);
        }

        fstream file;
        file.open(filepath, ios::app);
        file << val + ',' + values + '\n';
        file.close();

        filepath = "../" + BD + "/" + table + "/" + table + "_lock.txt";
        filerec(filepath, "open");
        return "Команда выполнена!";
    } else return "Ошибка, таблица используется другим пользователем!";
}


string BaseDate::isValidDel(string& command) { // ф-ия обработки команды DELETE
    string table, conditions;
    int position = command.find_first_of(' ');
    if (position != -1) {
        table = command.substr(0, position);
        conditions = command.substr(position + 1);
    } else table = command;
    if (nametables.getindex(table) != -1) { // проверка таблицы
        if (conditions.empty()) { // если нет условий, удаляем все
            return del(table);
        } else {
            if (conditions.substr(0, 6) == "where ") { // проверка наличия where
                conditions.erase(0, 6);
                SinglyLinkedList<Filter> cond;
                Filter where;
                position = conditions.find_first_of(' '); ////
                if (position != -1) { // проверка синтаксиса
                    where.colona = conditions.substr(0, position);
                    conditions.erase(0, position+1);
                    int index = nametables.getindex(table);
                    string str = stlb.getvalue(index);
                    stringstream ss(str);
                    bool check = false;
                    while (getline(ss, str, ',')) if (str == where.colona) check = true;
                    if (check) { // проверка столбца
                        if (conditions[0] == '=' && conditions[1] == ' ') { // проверка синтаксиса
                            conditions.erase(0, 2);
                            position = conditions.find_first_of(' ');
                            if (position == -1) { // если нет лог. оператора
                                where.value = conditions;
                                return delWithValue(table, where.colona, where.value);
                            } else { // если есть логический оператор
                                where.value = conditions.substr(0, position);
                                conditions.erase(0, position+1);
                                cond.push_back(where);
                                position = conditions.find_first_of(' ');
                                if ((position != -1) && (conditions.substr(0, 2) == "or" || conditions.substr(0, 3) == "and")) {
                                    where.logicOP = conditions.substr(0, position);
                                    conditions.erase(0, position + 1);
                                    position = conditions.find_first_of(' ');
                                    if (position != -1) {
                                        where.colona = conditions.substr(0, position);
                                        conditions.erase(0, position+1);
                                        index = nametables.getindex(table);
                                        str = stlb.getvalue(index);
                                        stringstream iss(str);
                                        bool check = false;
                                        while (getline(iss, str, ',')) if (str == where.colona) check = true;
                                        if (check) { // проверка столбца
                                            if (conditions[0] == '=' && conditions[1] == ' ') { // проверка синтаксиса
                                                conditions.erase(0, 2);
                                                position = conditions.find_first_of(' ');
                                                if (position == -1) {
                                                    where.value = conditions;
                                                    cond.push_back(where);
                                                    return delWithLogic(cond, table);
                                                } else return "Ошибка, нарушен синтаксис команды!";
                                            } else return "Ошибка, нарушен синтаксис команды!";
                                        } else return "Ошибка, нет такого столбца!";
                                    } else return "Ошибка, нарушен синтаксис команды!";
                                } else return "Ошибка, нарушен синтаксис команды!";
                            }
                        } else return "Ошибка, нарушен синтаксис команды!";
                    } else return "Ошибка, нет такого столбца!";
                } else return "Ошибка, нарушен синтаксис команды!";
            } else return "Ошибка, нарушен синтаксис команды!";
        }
    } else return "Ошибка, нет такой таблицы!";
}

string BaseDate::del(string& table) { // ф-ия удаления всех строк таблицы
    string filepath;
    int index = nametables.getindex(table);
    if (checkLockTable(table)) {
        filepath = "../" + BD + "/" + table + "/" + table + "_lock.txt";
        filerec(filepath, "close");
        
        // очищаем все файлы
        int copy = fileindex.getvalue(index);
        while (copy != 0) {
            filepath = "../" + BD + "/" + table + "/" + to_string(copy) + ".csv";
            filerec(filepath, "");
            copy--;
        }

        filerec(filepath, stlb.getvalue(index)+"\n"); // добавляем столбцы в 1.csv

        filepath = "../" + BD + "/" + table + "/" + table + "_lock.txt";
        filerec(filepath, "open");
        return "Команда выполнена!";
    } else return "Ошибка, таблица используется другим пользователем!";
}

string BaseDate::delWithValue(string& table, string& stolbec, string& values) { // ф-ия удаления строк таблицы по значению
    string filepath;
    int index = nametables.getindex(table);
    if (checkLockTable(table)) {
        filepath = "../" + BD + "/" + table + "/" + table + "_lock.txt";
        filerec(filepath, "close");

        // нахождение индекса столбца в файле
        string str = stlb.getvalue(index);
        stringstream ss(str);
        int stolbecindex = 0;
        while (getline(ss, str, ',')) {
            if (str == stolbec) break;
            stolbecindex++;
        }

        // удаление строк
        int copy = fileindex.getvalue(index);
        while (copy != 0) {
            filepath = "../" + BD + "/" + table + "/" + to_string(copy) + ".csv";
            string text = fileread(filepath);
            stringstream stroka(text);
            string filteredlines;
            while (getline(stroka, text)) {
                stringstream iss(text);
                string token;
                int currentIndex = 0;
                bool shouldRemove = false;
                while (getline(iss, token, ',')) {
                    if (currentIndex == stolbecindex && token == values) {
                        shouldRemove = true;
                        break;
                    }
                    currentIndex++;
                }
                if (!shouldRemove) filteredlines += text + "\n"; 
            }
            filerec(filepath, filteredlines);
            copy--;
        }

        filepath = "../" + BD + "/" + table + "/" + table + "_lock.txt";
        filerec(filepath, "open");
        return "Команда выполнена!";
    } else return "Ошибка, таблица используется другим пользователем!";
}

string BaseDate::delWithLogic(SinglyLinkedList<Filter>& conditions, string& table) { // ф-ия удаления строк таблицы с логикой
    string filepath;
    int index = nametables.getindex(table);
    if (checkLockTable(table)) {
        filepath = "../" + BD + "/" + table + "/" + table + "_lock.txt";
        filerec(filepath, "close");

        // нахождение индекса столбцов в файле
        SinglyLinkedList<int> stlbindex;
        for (int i = 0; i < conditions.size; ++i) {
            string str = stlb.getvalue(index);
            stringstream ss(str);
            int stolbecindex = 0;
            while (getline(ss, str, ',')) {
                if (str == conditions.getvalue(i).colona) {
                    stlbindex.push_back(stolbecindex);
                    break;
                }
                stolbecindex++;
            }
        }

        // удаление строк
        int copy = fileindex.getvalue(index);
        while (copy != 0) {
            filepath = "../" + BD + "/" + table + "/" + to_string(copy) + ".csv";
            string text = fileread(filepath);
            stringstream stroka(text);
            string filteredRows;
            while (getline(stroka, text)) {
                SinglyLinkedList<bool> shouldRemove;
                for (int i = 0; i < stlbindex.size; ++i) {
                    stringstream iss(text);
                    string token;
                    int currentIndex = 0;
                    bool check = false;
                    while (getline(iss, token, ',')) { 
                        if (currentIndex == stlbindex.getvalue(i) && token == conditions.getvalue(i).value) {
                            check = true;
                            break;
                        }
                        currentIndex++;
                    }
                    if (check) shouldRemove.push_back(true);
                    else shouldRemove.push_back(false);
                }
                if (conditions.getvalue(1).logicOP == "and") { // Если оператор И
                    if (shouldRemove.getvalue(0) && shouldRemove.getvalue(1));
                    else filteredRows += text + "\n";
                } else { // Если оператор ИЛИ
                    if (!(shouldRemove.getvalue(0)) && !(shouldRemove.getvalue(1))) filteredRows += text + "\n";
                }
            }
            filerec(filepath, filteredRows);
            copy--;
        }

        filepath = "../" + BD + "/" + table + "/" + table + "_lock.txt";
        filerec(filepath, "open");
        return "Команда выполнена!";
    } else return "Ошибка, таблица используется другим пользователем!";
}


string BaseDate::isValidSelect(string& command) { // ф-ия проверки ввода команды select
    Filter conditions;
    SinglyLinkedList<Filter> cond;

    if (command.find_first_of("from") != -1) {
        // работа со столбцами
        while (command.substr(0, 4) != "from") {
            string token = command.substr(0, command.find_first_of(' '));
            if (token.find_first_of(',') != -1) token.pop_back(); // удаляем запятую
            command.erase(0, command.find_first_of(' ') + 1);
            if (token.find_first_of('.') != -1) token.replace(token.find_first_of('.'), 1, " ");
            else {
                return "Ошибка, нарушен синтаксис команды!";
            }
            stringstream ss(token);
            ss >> conditions.table >> conditions.colona;
            bool check = false;
            int i;
            for (i = 0; i < nametables.size; ++i) { // проверка, сущ. ли такая таблица
                if (conditions.table == nametables.getvalue(i)) {
                    check = true;
                    break;
                }
            }
            if (!check) {
                return "Нет такой таблицы!";
            }
            check = false;
            stringstream iss(stlb.getvalue(i));
            while (getline(iss, token, ',')) { // проверка, сущ. ли такой столбец
                if (token == conditions.colona) {
                    check = true;
                    break;
                }
            }
            if (!check) {
                return "Нет такого столбца";
            }
            cond.push_back(conditions);
        }

        command.erase(0, command.find_first_of(' ') + 1); // скип from

        // работа с таблицами
        int iter = 0;
        while (!command.empty()) { // пока строка не пуста
            string token = command.substr(0, command.find_first_of(' '));
            if (token.find_first_of(',') != -1) {
                token.pop_back();
            }
            int position = command.find_first_of(' ');
            if (position != -1) command.erase(0, position + 1);
            else command.erase(0);
            if (iter + 1 > cond.size || token != cond.getvalue(iter).table) {
                return "Ошибка, указаные таблицы не совпадают или их больше!";
            }
            if (command.substr(0, 5) == "where") break; // также заканчиваем цикл если встретился WHERE
            iter++;
        }
        if (command.empty()) {
            return select(cond);
        } else {
            if (command.find_first_of(' ') != -1) {
                command.erase(0, 6);
                int position = command.find_first_of(' ');
                if (position != -1) {
                    string token = command.substr(0, position);
                    command.erase(0, position + 1);
                    if (token.find_first_of('.') != -1) {
                        token.replace(token.find_first_of('.'), 1, " ");
                        stringstream ss(token);
                        string table, column;
                        ss >> table >> column;
                        if (table == cond.getvalue(0).table) { // проверка таблицы в where
                            position = command.find_first_of(' ');
                            if ((position != -1) && (command[0] == '=')) {
                                command.erase(0, position + 1);
                                position = command.find_first_of(' ');
                                if (position == -1) { // если нет лог. операторов
                                    if (command.find_first_of('.') == -1) { // если просто значение
                                        conditions.value = command;
                                        conditions.check = true;
                                        return selectWithValue(cond, table, column, conditions);
                                    } else { // если столбец
                                        command.replace(command.find_first_of('.'), 1, " ");
                                        stringstream iss(command);
                                        iss >> conditions.table >> conditions.colona;
                                        conditions.check = false;
                                        return selectWithValue(cond, table, column, conditions);
                                    }

                                } else { // если есть лог. операторы
                                    SinglyLinkedList<Filter> values;
                                    token = command.substr(0, position);
                                    command.erase(0, position + 1);
                                    if (token.find_first_of('.') == -1) { // если просто значение
                                        conditions.value = token;
                                        conditions.check = true;
                                        values.push_back(conditions);
                                    } else { // если столбец
                                        token.replace(token.find_first_of('.'), 1, " ");
                                        stringstream stream(token);
                                        stream >> conditions.table >> conditions.colona;
                                        conditions.check = false;
                                        values.push_back(conditions);
                                    }
                                    position = command.find_first_of(' ');
                                    if ((position != -1) && (command.substr(0, 2) == "or" || command.substr(0, 3) == "and")) {
                                        conditions.logicOP = command.substr(0, position);
                                        command.erase(0, position + 1);
                                        position = command.find_first_of(' ');
                                        if (position != -1) {
                                            token = command.substr(0, position);
                                            command.erase(0, position + 1);
                                            if (token.find_first_of('.') != -1) {
                                                token.replace(token.find_first_of('.'), 1, " ");
                                                stringstream istream(token);
                                                SinglyLinkedList<string> tables;
                                                SinglyLinkedList<string> columns;
                                                tables.push_back(table);
                                                columns.push_back(column);
                                                istream >> table >> column;
                                                tables.push_back(table);
                                                columns.push_back(column);
                                                if (table == cond.getvalue(0).table) { // проверка таблицы в where
                                                    position = command.find_first_of(' ');
                                                    if ((position != -1) && (command[0] == '=')) {
                                                        command.erase(0, position + 1);
                                                        position = command.find_first_of(' ');
                                                        if (position == -1) { // если нет лог. операторов
                                                            if (command.find_first_of('.') == -1) { // если просто значение
                                                                conditions.value = command.substr(0, position);
                                                                conditions.check = true;
                                                                command.erase(0, position + 1);
                                                                values.push_back(conditions);
                                                                return selectWithLogic(cond, tables, columns, values);
                                                            } else { // если столбец
                                                                token = command.substr(0, position);
                                                                token.replace(token.find_first_of('.'), 1, " ");
                                                                command.erase(0, position + 1);
                                                                stringstream stream(token);
                                                                stream >> conditions.table >> conditions.colona;
                                                                conditions.check = false;
                                                                values.push_back(conditions);
                                                                return selectWithLogic(cond, tables, columns, values);
                                                            }
                                                        } else return "Ошибка, нарушен синтаксис команды!";
                                                    } else return "Ошибка, нарушен синтаксис команды!";
                                                } else return "Ошибка, таблица в where не совпадает с начальной!";
                                            } else return "Ошибка, нарушен синтаксис команды!";
                                        } else return "Ошибка, нарушен синтаксис команды!";
                                    } else return "Ошибка, нарушен синтаксис команды!";
                                }
                            } else return "Ошибка, нарушен синтаксис команды!";
                        } else return "Ошибка, таблица в where не совпадает с начальной!";
                    } else return "Ошибка, нарушен синтаксис команды!";
                } else return "Ошибка, нарушен синтаксис команды!";
            } else return "Ошибка, нарушен синтаксис команды!";
        }
    } else return "Ошибка, нарушен синтаксис команды!";
}

string BaseDate::select(SinglyLinkedList<Filter>& conditions) { // ф-ия обычного селекта
    for (int i = 0; i < conditions.size; ++i) {
        bool check = checkLockTable(conditions.getvalue(i).table);
        if (!check) {
            return "Ошибка, таблица открыта другим пользователем!";
        }
    }
    string filepath;
    for (int i = 0; i < conditions.size; ++i) {
        filepath = "../" + BD + '/' + conditions.getvalue(i).table + '/' + conditions.getvalue(i).table + "_lock.txt";
        filerec(filepath, "close");
    }

    SinglyLinkedList<int> stlbindex = findIndexStlb(conditions); // узнаем индексы столбцов после "select"
    SinglyLinkedList<string> tables = textInFile(conditions); // записываем данные из файла в переменные для дальнейшей работы

    for (int i = 0; i < conditions.size; ++i) {
        filepath = "../" + BD + '/' + conditions.getvalue(i).table + '/' + conditions.getvalue(i).table + "_lock.txt";
        filerec(filepath, "open");
    }

    return sample(stlbindex, tables); // выборка
}

string BaseDate::selectWithValue(SinglyLinkedList<Filter>& conditions, string& table, string& stolbec, struct Filter value) { // ф-ия селекта с where для обычного условия
    for (int i = 0; i < conditions.size; ++i) {
        bool check = checkLockTable(conditions.getvalue(i).table);
        if (!check) {
            return "Ошибка, таблица открыта другим пользователем!";
        }
    }
    string filepath;
    for (int i = 0; i < conditions.size; ++i) {
        filepath = "../" + BD + '/' + conditions.getvalue(i).table + '/' + conditions.getvalue(i).table + "_lock.txt";
        filerec(filepath, "close");
    }

    SinglyLinkedList<int> stlbindex = findIndexStlb(conditions); // узнаем индексы столбцов
    int stlbindexval = findIndexStlbCond(table, stolbec); // узнаем индекс столбца условия
    int stlbindexvalnext = findIndexStlbCond(value.table, value.colona); // узнаем индекс столбца условия после '='(нужно если условиестолбец)
    SinglyLinkedList<string> tables = textInFile(conditions); // записываем данные из файла в переменные для дальнейшей работы
    SinglyLinkedList<string> column = findStlbTable(conditions, tables, stlbindexvalnext, value.table);; // записываем колонки таблицы условия после '='(нужно если условиестолбец)
    
    // фильтруем нужные строки
    for (int i = 0; i < conditions.size; ++i) {
        if (conditions.getvalue(i).table == table) { 
            stringstream stream(tables.getvalue(i));
            string str;
            string filetext;
            int iterator = 0; // нужно для условиястолбец 
            while (getline(stream, str)) {
                stringstream istream(str);
                string token;
                int currentIndex = 0;
                while (getline(istream, token, ',')) {
                    if (value.check) { // для простого условия
                        if (currentIndex == stlbindexval && token == value.value) {
                            filetext += str + '\n';
                            break;
                        }
                        currentIndex++;
                    } else { // для условиястолбец
                        if (currentIndex == stlbindexval && token == column.getvalue(iterator)) {
                        filetext += str + '\n';
                        }
                        currentIndex++;
                    }
                }
                iterator++;
            }
            tables.replace(i, filetext);
        }
    }

    for (int i = 0; i < conditions.size; ++i) {
        filepath = "../" + BD + '/' + conditions.getvalue(i).table + '/' + conditions.getvalue(i).table + "_lock.txt";
        filerec(filepath, "open");
    }

    return sample(stlbindex, tables); // выборка
}

string BaseDate::selectWithLogic(SinglyLinkedList<Filter>& conditions, SinglyLinkedList<string>& table, SinglyLinkedList<string>& stolbec, SinglyLinkedList<Filter>& value) {
    for (int i = 0; i < conditions.size; ++i) {
        bool check = checkLockTable(conditions.getvalue(i).table);
        if (!check) {
            return "Ошибка, таблица открыта другим пользователем!";
        }
    }
    string filepath;
    for (int i = 0; i < conditions.size; ++i) {
        filepath = "../" + BD + '/' + conditions.getvalue(i).table + '/' + conditions.getvalue(i).table + "_lock.txt";
        filerec(filepath, "close");
    }

    SinglyLinkedList<int> stlbindex = findIndexStlb(conditions); // узнаем индексы столбцов после "select"
    SinglyLinkedList<string> tables = textInFile(conditions); // записываем данные из файла в переменные для дальнейшей работы
    SinglyLinkedList<int> stlbindexval;// узнаем индексы столбца условия
    for (int i = 0; i < stolbec.size; ++i) {
        int index = findIndexStlbCond(table.getvalue(i), stolbec.getvalue(i));
        stlbindexval.push_back(index);
    }
    SinglyLinkedList<int> stlbindexvalnext; // узнаем индекс столбца условия после '='(нужно если условиестолбец)
    for (int i = 0; i < value.size; ++i) {
        int index = findIndexStlbCond(value.getvalue(i).table, value.getvalue(i).colona); // узнаем индекс столбца условия после '='(нужно если условиестолбец)
        stlbindexvalnext.push_back(index);
    }
    SinglyLinkedList<string> column;
    for (int j = 0; j < value.size; ++j) {
        if (!value.getvalue(j).check) { // если условие столбец
            column = findStlbTable(conditions, tables, stlbindexvalnext.getvalue(j), value.getvalue(j).table);
        }
    }

    // фильтруем нужные строки
    for (int i = 0; i < conditions.size; ++i) {
        if (conditions.getvalue(i).table == table.getvalue(0)) {
            stringstream stream(tables.getvalue(i));
            string str;
            string filetext;
            int iterator = 0; // нужно для условиястолбец 
            while (getline(stream, str)) {
                SinglyLinkedList<bool> checkstr;
                for (int j = 0; j < value.size; ++j) {
                    stringstream istream(str);
                    string token;
                    int currentIndex = 0;
                    bool check = false;
                    while (getline(istream, token, ',')) {
                        if (value.getvalue(j).check) { // если просто условие
                            if (currentIndex == stlbindexval.getvalue(j) && token == value.getvalue(j).value) {
                                check = true;
                                break;
                            }
                            currentIndex++;
                        } else { // если условие столбец
                            if (currentIndex == stlbindexval.getvalue(j) && token == column.getvalue(iterator)) {
                                check = true;
                                break;
                            }
                            currentIndex++;
                        }
                    }
                    checkstr.push_back(check);
                }
                if (value.getvalue(1).logicOP == "and") { // Если оператор И
                    if (checkstr.getvalue(0) && checkstr.getvalue(1)) filetext += str + "\n";
                } else { // Если оператор ИЛИ
                    if (!checkstr.getvalue(0) && !checkstr.getvalue(1));
                    else filetext += str + "\n";
                }
                iterator++;
            }
            tables.replace(i, filetext);
        }
    }

    for (int i = 0; i < conditions.size; ++i) {
        filepath = "../" + BD + '/' + conditions.getvalue(i).table + '/' + conditions.getvalue(i).table + "_lock.txt";
        filerec(filepath, "open");
    }

    return sample(stlbindex, tables); // выборка
}


bool BaseDate::checkLockTable(string table) { // ф-ия проверки, закрыта ли таблица
    string filepath = "../" + BD + "/" + table + "/" + table + "_lock.txt";
    string check = fileread(filepath);
    if (check == "open") return true;
    else return false;
}

SinglyLinkedList<int> BaseDate::findIndexStlb(SinglyLinkedList<Filter>& conditions) { // ф-ия нахождения индекса столбцов(для select)
    SinglyLinkedList<int> stlbindex;
    for (int i = 0; i < conditions.size; ++i) {
        int index = nametables.getindex(conditions.getvalue(i).table);
        string str = stlb.getvalue(index);
        stringstream ss(str);
        int stolbecindex = 0;
        while (getline(ss, str, ',')) {
            if (str == conditions.getvalue(i).colona) {
                stlbindex.push_back(stolbecindex);
                break;
            }
            stolbecindex++;
        }
    }
    return stlbindex;
}

int BaseDate::findIndexStlbCond(string table, string stolbec) { // ф-ия нахождения индекса столбца условия(для select)
    int index = nametables.getindex(table);
    string str = stlb.getvalue(index);
    stringstream ss(str);
    int stlbindex = 0;
    while (getline(ss, str, ',')) {
        if (str == stolbec) break;
        stlbindex++;
    }
    return stlbindex;
}

SinglyLinkedList<string> BaseDate::textInFile(SinglyLinkedList<Filter>& conditions) { // ф-ия инпута текста из таблиц(для select)
    string filepath;
    SinglyLinkedList<string> tables;
    for (int i = 0; i < conditions.size; ++i) {
        string filetext;
        int index = nametables.getindex(conditions.getvalue(i).table);
        int iter = 0;
        do {
            iter++;
            filepath = "../" + BD + '/' + conditions.getvalue(i).table + '/' + to_string(iter) + ".csv";
            string text = fileread(filepath);
            int position = text.find('\n'); // удаляем названия столбцов
            text.erase(0, position + 1);
            filetext += text + '\n';
        } while (iter != fileindex.getvalue(index));
        tables.push_back(filetext);
    }
    return tables;
}

SinglyLinkedList<string> BaseDate::findStlbTable(SinglyLinkedList<Filter>& conditions, SinglyLinkedList<string>& tables, int stlbindexvalnext, string table) { // ф-ия инпута нужных колонок из таблиц для условиястолбец(для select)
    SinglyLinkedList<string> column;
    for (int i = 0; i < conditions.size; ++i) {
        if (conditions.getvalue(i).table == table) {
            stringstream stream(tables.getvalue(i));
            string str;
            while (getline(stream, str)) {
                stringstream istream(str);
                string token;
                int currentIndex = 0;
                while (getline(istream, token, ',')) {
                    if (currentIndex == stlbindexvalnext) {
                        column.push_back(token);
                        break;
                    }
                    currentIndex++;
                }
            }
        }
    }
    return column;
}

string BaseDate::sample(SinglyLinkedList<int>& stlbindex, SinglyLinkedList<string>& tables) { // ф-ия выборки(для select)
    string result;
    for (int i = 0; i < tables.size - 1; ++i) {
        stringstream onefile(tables.getvalue(i));
        string token;
        while (getline(onefile, token)) {
            string needstlb;
            stringstream ionefile(token);
            int currentIndex = 0;
            while (getline(ionefile, token, ',')) {
                if (currentIndex == stlbindex.getvalue(i)) {
                    needstlb = token;
                    break;
                }
                currentIndex++;
            }
            stringstream twofile(tables.getvalue(i + 1));
            while (getline(twofile, token)) {
                stringstream itwofile(token);
                currentIndex = 0;
                while (getline(itwofile, token, ',')) {
                    if (currentIndex == stlbindex.getvalue(i + 1)) {
                        result += needstlb + ' ' + token + '\n';
                        break;
                    }
                    currentIndex++;
                }
            }
        } 
    }
    return result;
}
