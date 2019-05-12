#include <iostream>
#include <memory>

#include "async.h"
#include "exception_list.h"

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
