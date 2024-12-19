#include "../include/networks.h"

void createServer(BaseDate& airport) {
    createSocket();
    connectClient(airport);
}

void createSocket() {
    // создание TCP-сокета
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        cout << "Ошибка создания сокета!" << endl;
        exit(-1);
    }

    // Настройка опции сокета
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        cout << "Ошибка установки опции сокета" << endl;
        exit(-1);
    }

    // установка адреса и порта
    server_address.sin_port = htons(PORT); // преобразование порта в сетевой порядок байт
    server_address.sin_family = AF_INET; // семейство адресов (IPv 4)
    server_address.sin_addr.s_addr = INADDR_ANY; // привязка к любому адресу

    // привязка сокета к адресу
    if ((bind(server, (struct sockaddr*)&server_address, sizeof(server_address))) < 0) {
        cout << "Ошибка привязки сокета!" << endl;
        exit(-1);
    }
}

void connectClient(BaseDate& airport) {
    // прослушивание входящих соединений
    if (listen(server, 2) < 0) {
        cout << "Ошибка прослушивания!" << endl;
        exit(-1);
    }
    cout << "Сервер запущен. Ожидание подключения на порту " << PORT << "..." << endl;

    // принятие входящего соединения
    while (true) {
        new_socket = accept(server, (struct sockaddr*)&server_address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            cout << "Ошибка принятия соединения!" << endl;
            exit(-1);
        }
        cout << "Клиент подключен" << endl;

        static mutex mx;
        thread(procOfReq, new_socket, ref(airport), ref(mx)).detach(); // обработка запроса клиента в отдельном потоке
    }
}

void procOfReq(int client_socket, BaseDate& airport, mutex& mx) {
    while (true) {
        char buffer[1024] = {0};
        int check = read(client_socket, buffer, 1024);
        if (check <= 0 ) { // клиент отключился
            cout << "Клиент отключился, ожидание нового подключения..." << endl;
            break; 
        }
        string conditions(buffer);
        lock_guard<mutex> lock(mx);
        this_thread::sleep_for(chrono::milliseconds(5000));
        string message = airport.checkcommand(conditions);
        // отправка ответа клиенту
        send(client_socket, message.c_str(), message.size(), 0);
    }
    close(client_socket);
}