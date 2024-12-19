#ifndef commands_h
#define commands_h

#include "utility.h"
#include "list.h"
#include "json.hpp"

int countingLine (string& fin); // ф-ия подсчёта строк в файле
string fileread (string& filename); // Производим чтение из файла // чтение из файла
void filerec (string& filename, string data); // запись в файл

struct BaseDate {
    string BD; // название БД
    int rowLimits; // лимит строк
    SinglyLinkedList<string> nametables; // названия таблиц
    SinglyLinkedList<string> stlb; // столбцы таблиц
    SinglyLinkedList<int> fileindex; // количество файлов таблиц

    struct Filter { // структура для фильтрации
        string table;
        string colona;
        string value;
        string logicOP;
        bool check;
    };

    void parser();
    void createdirect();

    /// Функии для INSERT ///
    string checkInsert(string& table, string& values); // Проверка ввода команды инсерта
    string Insert(string& command); // Функция инсерта

    /// Функции для DELETE ///
    string delAll(string& table); // Функция очистки всей таблицы
    string delZnach(string& table, string& stolbec, string& values); // Функция удаления строк по значению
    string delYslov(SinglyLinkedList<Filter>& conditions, string& table); //Функция удаления по условию
    string Delete(string& command); // Поверка синтаксиса команды
    bool parseConditions(string& conditions, string& table, SinglyLinkedList<Filter>& cond);
    bool isValidColumn(string& table, string& colona);

    /// Функции для SELECT ///
    string isValidSelect(string& command); // ф-ия проверки ввода команды select
    string select(SinglyLinkedList<Filter>& conditions); // ф-ия обычного селекта
    string selectWithValue(SinglyLinkedList<Filter>& conditions, string& table, string& stolbec, struct Filter value); // ф-ия селекта с where для обычного условия
    string selectWithLogic(SinglyLinkedList<Filter>& conditions, SinglyLinkedList<string>& table, SinglyLinkedList<string>& stolbec, SinglyLinkedList<Filter>& value);

    // Вспомогательные ф-ии, чтобы избежать повтора кода в основных ф-иях
    bool checkLockTable(string table); // ф-ия проверки, закрыта ли таблица
    string lockTable(string& table, bool open);
    string checkcommand(string& command);  // Функция ввода команд
    SinglyLinkedList<int> findIndexStlb(SinglyLinkedList<Filter>& conditions); // ф-ия нахождения индекса столбцов(для select)
    int findIndexStlbCond(string table, string stolbec); // ф-ия нахождения индекса столбца условия(для select)
    SinglyLinkedList<string> textInFile(SinglyLinkedList<Filter>& conditions); // ф-ия инпута текста из таблиц(для select)
    SinglyLinkedList<string> findStlbTable(SinglyLinkedList<Filter>& conditions, SinglyLinkedList<string>& tables, int stlbindexvalnext, string table); // ф-ия инпута нужных колонок из таблиц для условиястолбец(для select)
    string sample(SinglyLinkedList<int>& stlbindex, SinglyLinkedList<string>& tables); // ф-ия выборки(для select)
};

#include "../src/commands.cpp"

#endif // COMMANDS_H