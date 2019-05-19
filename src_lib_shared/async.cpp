#include "async.h"

#include "queue.h"
#include "thread_mgr.h"
#include "thread_fn.h"
#include "exception_list.h"
#include <mutex>

#include "bulk_reader.h"

using namespace std;
using namespace roro_lib;

namespace async
{
      class async_proxy
      {
            mutex async_proxy_mutex;

            using queue_tread_t = queue<std::vector<std::string>, std::time_t>;
            command_reader cmdr;
            queue_tread_t console_queue;
            thread_mgr<std::vector<std::string>, std::time_t> console_tmgr;
            queue_tread_t file_queue;
            thread_mgr<std::vector<std::string>, std::time_t> file_tmgr;

        public:
            async_proxy(size_t size_bulk) : cmdr(size_bulk),
                                            console_queue(),
                                            console_tmgr(1, console_queue, output_to_console),
                                            file_queue(),
                                            file_tmgr(2, file_queue, save_log_file(), &save_log_file::save)
            {
                  cmdr.subscribe([&](const auto& vec, auto t) {
                        try
                        {
                              console_queue.push_wait(vec, t);
                        }
                        catch (...)
                        {
                              std::throw_with_nested(std::runtime_error("'console' subscriber failed."));
                        }
                  });

                  cmdr.subscribe([&](const auto& vec, auto t) {
                        try
                        {
                              file_queue.push_wait(vec, t);
                        }
                        catch (...)
                        {
                              std::throw_with_nested(std::runtime_error("'file' subscriber failed."));
                        }
                  });
            };

            void read(const char* data, std::size_t size)
            {
                  lock_guard<mutex> async_proxy_guard(async_proxy_mutex);

                  if (size)
                  {
                        if (data[0] == '\n')
                        {
                              data++;
                              size--;
                        }
                        if (size)
                        {
                              std::stringstream ss;
                              ss.write(data, size);
                              cmdr.read(ss);
                        }
                  }
            }

            void finalize()
            {
                  lock_guard<mutex> async_proxy_guard(async_proxy_mutex);

                  cmdr.flush();
                  console_tmgr.finalize_threads();
                  file_tmgr.finalize_threads();

                  // выводим счетчики на консоль

                  cout << "\nmain thread - " << cmdr.get_counters() << std::endl;
                  cout << "log thread - " << console_tmgr.get_list_counters().back() << std::endl;

                  for (auto file_counters : file_tmgr.get_list_counters())
                  {
                        cout << "file" << file_counters.number_thread << " thread - " << file_counters << std::endl;
                  }

                  cout << std::endl;

                  // повторно бросаем исключения потоков

                  exception_ptr_list all_threads_exceptions = console_tmgr.get_threads_exceptions() +
                                                              file_tmgr.get_threads_exceptions();
                  all_threads_exceptions.rethrow_if_exist();
            }

      };

      handle_t connect(std::size_t bulk)
      {
            return new async_proxy(bulk);
      }

      void receive(handle_t handle, const char* data, std::size_t size)
      {
            async_proxy* a_proxy = static_cast<async_proxy*>(handle); 
            a_proxy->read(data, size);
      }

      void disconnect(handle_t handle)
      {
            async_proxy* a_proxy = static_cast<async_proxy*>(handle);
            try
            {
                  a_proxy->finalize();
                  delete a_proxy;
            }
            catch (...)
            {
                  delete a_proxy;
                  throw;
            }
      }

}
