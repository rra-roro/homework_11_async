#pragma once
#include "queue.h"
#include "counters.h"
#include "exception_list.h"
#include <functional>
#include <thread>
#include <list>

namespace roro_lib
{
      template <typename... Args>
      class thread_mgr
      {
            queue<Args...>& queue_thread;

        public:
            template <typename F,
                      typename std::enable_if_t<std::is_invocable_v<F, queue<Args...>&,
                                                                       counters_thread_mgr&,
                                                                       std::exception_ptr&>>* Facke = nullptr
            >
            thread_mgr(std::size_t count_threads,
                       queue<Args...>& q,
                       F fn) : queue_thread(q)
            {
                  thread_mgr_internal(count_threads, q, fn);
            }

            template <typename T, typename MF,
                      typename std::enable_if_t<std::is_member_function_pointer_v<MF>>* Facke = nullptr
            >
            thread_mgr(std::size_t count_threads,
                       queue<Args...>& q,
                       T&& obj, MF mfn) : queue_thread(q)
            {
                  using namespace std::placeholders;
                  thread_mgr_internal(count_threads, q, bind(mfn, std::forward<T>(obj), _1, _2, _3));
            }

            void finalize_threads()
            {
                  queue_thread.push_wait_exit();

                  for (auto& thr : list_threads)
                  {
                        if (thr.joinable())
                              thr.join();
                  }
            }

            ~thread_mgr()
            {
                  try
                  {
                        finalize_threads();
                  }
                  catch (...)
                  {
                        for (auto& thr : list_threads)
                        {
                              if (thr.joinable())
                                    thr.detach();
                        }
                  }
            }

            auto get_list_counters() const
            {
                  return list_counters;
            }

            auto get_threads_exceptions() const
            {
                  return list_exception_ptr;
            }

        private:
            std::list<std::thread> list_threads;
            std::list<counters_thread_mgr> list_counters;
            exception_ptr_list list_exception_ptr;

            template <typename T>
            void thread_mgr_internal(std::size_t count_threads,
                                     queue<Args...>& q,
                                     T fn)
            {
                  for (size_t i = 0; i < count_threads; i++)
                  {
                        list_counters.emplace_back(i);
                        list_exception_ptr.add_back_exception_ptr();

                        list_threads.push_back(std::thread(fn,
                                                           std::ref(q),
                                                           std::ref(list_counters.back()),
                                                           std::ref(list_exception_ptr.back())));
                  }
            }
      };
}
