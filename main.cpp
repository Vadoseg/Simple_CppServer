#include <iostream>
#include <cstring>      // strlen
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // inet_pton
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // close

#define PORT 8080

int main() {
  int server_fd, new_socket;          // Файловый дескриптор сервера и сокет для клиента
  struct sockaddr_in address;         // Структура для хранения информации о сетевом адресе
  const int opt = 1;                  // Переменная для настройки опций сокета, позволит дать истинное значение для функций SO_REUSEADDR | SO_REUSEPORT
  int addrlen = sizeof(address);

  // Создаём сокет
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {   // AF_INET - значит IPv4, SOCK_STREAM - указывает на TCP (Потоковый протокол), 0 - используем стандартный протокол для данного сокета
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Настраиваем опции сокета (чтобы можно было быстро перезапускать сервер)
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,    // SO_REUSEADDR | SO_REUSEPORT позволяют сразу после закрытия сервера использовать тот же адрес и порт
                 &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // Заполняем структуру адреса
  address.sin_family = AF_INET;                           // Привязываем семейство IPv4 для адресов
  inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);     // Укажите ваш IP-адрес, использование inet_pton является более безопасным, т.к. преобразует ip-адрес в бинарный формат и записывает в поле структуры
  address.sin_port = htons(PORT);                         // Привязываем порт

  if (bind(server_fd, (struct sockaddr *)&address,        // Привязываем сокет к адресу и порту
           sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 1) < 0) {                         // Начинаем слушать входящие подключения, второй параметр указывает максмальное количество ожидающих подключений в очереди
    perror("listen");
    exit(EXIT_FAILURE);
  }

  std::cout << "Server listening on port " << PORT << std::endl;

  char buffer[1024] = {0};                                            // Создание буфера для хранения данных клиента

  while (true) {
    // Принятие входящего соединения
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
      std::cerr << "Connection error" << std::endl;
    }

    std::cout << "Client connected." << std::endl;

           // Используем select для отслеживания состояния сокета
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(new_socket, &readfds);

    // Установка тайм-аута на select (например, 10 секунд)
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    while (true){

      int activity = select(new_socket + 1, &readfds, NULL, NULL, NULL);  // Ожидание данных от клиента

      if (activity < 0) {
        std::cerr << "Error select()" << std::endl;
        close(new_socket);
        break;
      } else if (activity == 0) {
        std::cout << "Timeout." << std::endl;
        close(new_socket);
        break;
      }


      // Чтение сообщения от клиента
      if (FD_ISSET(new_socket, &readfds)) {
        int valread = read(new_socket, buffer, sizeof(buffer));
        if (valread > 0) {
          std::cout << "Message recieved: " << buffer << std::endl;
          // Здесь можно обработать полученное сообщение по необходимости
        } else {
          std::cout << "Client disconnected." << std::endl;
          close(new_socket);
          break;
        }
      }

      // Очистка буфера для следующего сообщения
      memset(buffer, 0, sizeof(buffer));
    }

    std::cout << "Waiting for new connection..." << std::endl;
  }

  close(server_fd);
  return 0;
}
