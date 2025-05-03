#include <iostream>
// #include <cstring>      // strlen
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // inet_pton
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // close
// #include <vector>


int main() {
  int server_fd, new_socket;          // Файловый дескриптор сервера и сокет для клиента
  struct sockaddr_in address;         // Структура для хранения информации о сетевом адресе
  const int opt = 1;                  // Переменная для настройки опций сокета, позволит дать истинное значение для функций SO_REUSEADDR | SO_REUSEPORT
  int addrlen = sizeof(address);
  const int PORT = 8080;

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

  while(true) { // Сервер в этом цикле вечно принимает подключения
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,    // Сервер постоянно принимает подключение от клиентов. accept блокирует выполнение программы, пока не поступит новое подключение. Возвращает новый дескриптор сокета для общения с клиентом.
                             (socklen_t*)&addrlen))<0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    char buffer[1024] = {0};                                            // Создание буфера для хранения данных клиента
    // std::vector<char> buffer[1024];

    while (true) {  // Сервер в этом цикле вечно читает и вечно отвечает
      // Чтение данных от клиента
      int bytes_read = read(new_socket, buffer, sizeof(buffer) - 1);   // -1 нужен для нуль терминатора
      if (bytes_read <= 0) {
        std::cout << "Client disconnected or error occurred" << std::endl;
        break; // Выход из внутреннего цикла при отключении клиента или ошибке
      }

      buffer[bytes_read] = '\0'; // Добавляем нуль-терминатор для корректного вывода строки
      std::cout << "Received: " << buffer << std::endl;

      // Отправка ответа клиенту
      std::string response = "Message received: ";
      response += buffer;
      send(new_socket, response.c_str(), response.size(), 0);
    }


    close(new_socket);
  }

  return 0;
}
