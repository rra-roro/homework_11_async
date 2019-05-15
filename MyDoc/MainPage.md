# Otus homework 11: async

## Описание libasync.so
Пакете async устанавливает следующие файлы:<br>
<b>/usr/lib/libasync.so</b>  -  библиотека async<br>
<b>/usr/include/async.h</b>  -  интерфейс библиотеки async<br>
<b>/usr/include/shared_EXPORTS.h</b> - необходим для кроскомпиляции с библиотекой async<br>
<b>/usr/include/exception_list.h</b> - вспомогательный файл для обработки исключений библиотеки async

## Описание особенностей моей реализации:

1)  За основу взята многопоточная реализация из Задания 10

2)  Проект собирает под Windows .dll и под Linux .so  файл библиотеки

2)  Вызовы  connect(), receive() и disconnect() могут осуществляться из разных потоков.
    При этом, между вызовами connect() и disconnect() можно вызывать receive() из разных потоков, с одним и тем же контекстом выполнения.
    Т.е. receive() потокобезопасен.
    Вызов disconnect() так же можно осуществить из любого потока, но пользователь билиотеки libasync.so должен гарантировать, что после вызова disconnect()
    ни один из потоков не вызовет receive().

3)  Вызовы  connect(), receive() и disconnect() могут бросать исключения.
    Если receive() бросает исклчение, пользователь должен гарантировать вызов disconnect(), чтобы освободить ресурсы.
    Если disconnect() бросает исключение, то перед возбуждеием исключения, ресурсы гарантиовано освобождются.<br><br>
    Для безопасной обработки исключений возуждаемых connect(), receive() и disconnect() рекомедуетя такой код:
~~~cpp
#include <iostream>
#include <memory>

#include <async.h>
#include <exception_list.h>

using namespace roro_lib;
using namespace std;

int main(int, char*[])
{
      try
      {
            using async_t = unique_ptr<void, void (*)(async::handle_t)>;

            std::size_t bulk = 5;

            async_t h2(async::connect(bulk), async::disconnect);
            async_t h(async::connect(bulk), async::disconnect);

            async::receive(h.get(), "1", 1);
            async::receive(h2.get(), "1\n", 2);
            async::receive(h.get(), "\n2\n3\n4\n5\n6\n{\na\n", 15);
            async::receive(h.get(), "b\nc\nd\n}\n89\n", 11);

      }
      catch (const exception_ptr_list& ex_list)
      {
            exception_ptr_list::print(ex_list);
            return EXIT_FAILURE;
      }
      catch (const exception& ex)
      {
            print_nested_exception(ex);
            return EXIT_FAILURE;
      }
      catch (...)
      {
            cerr << "Error: unknown exception" << endl;
            return EXIT_FAILURE;
      }
      return 0;
}
~~~

## Использование libasync.so

После установки пакета async он готов к использованию.<br>
Для работы с библиотекой рекомендуется взять пример выше, и если сохранить его в файл main.cpp, то собрать его можно так:
~~~cpp
g++ main.cpp -lasync -lpthread
~~~

