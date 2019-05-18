#pragma once
#include "queue.h"
#include "counters.h"
#include "platform.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>
#include <tuple>
#include <exception>
#include <random>
#include <thread>

namespace roro_lib
{
      void output_to_console(queue<std::vector<std::string>, std::time_t>& que,
          counters_thread_mgr& counters_thread,
          std::exception_ptr& ex_ptr)
      {
            try
            {
                  try
                  {
                        using queue_thread_t = std::remove_reference_t<decltype(que)>;

                        while (true)
                        {
                              auto item = que.pop_wait();
                              if (std::get<0>(item) == queue_thread_t::exit)
                                    return;

                              counters_thread.count_block++;


                              std::string bulk = "bulk:";
                              for (auto cmd : std::get<1>(item))
                              {
                                    bulk += " " + cmd + ",";
                                    counters_thread.count_all_cmds++;
                              };

                              bulk.back() = '\n';

                              std::cout << bulk;
                        }
                  }
                  catch (std::bad_alloc&)
                  {
                        std::throw_with_nested(std::runtime_error("output_to_console() failed."));
                  }
                  catch (...)
                  {
                        std::stringstream ss;
                        ss << std::this_thread::get_id();
                        std::throw_with_nested(std::runtime_error("output_to_console() failed. Thread ID:" + ss.str()));
                  }
            }
            catch (...)
            {
                  ex_ptr = std::current_exception();
            }
      }


      struct save_log_file
      {
#ifdef OS64
            using rnd_gen_t = std::mt19937_64;
#else
            using rnd_gen_t = std::mt19937;
#endif // OS64

            std::string get_filename(std::time_t time_first_cmd, std::size_t number_thread, rnd_gen_t& gen)
            try
            {
                  std::uniform_int_distribution<> dis(1000, 9999);
                  std::stringstream sstr;
                  sstr << "./bulk" << time_first_cmd <<
                          "_tid"   << std::this_thread::get_id() <<
                          "_file"   << number_thread <<
                          "_uid" << dis(gen) << ".log";
                  return sstr.str();
            }
            catch (...)
            {
                  std::throw_with_nested(std::runtime_error("save_log_file::get_filename() failed."));
            }

            size_t get_tid()
            {
                  size_t id;
                  std::stringstream sstr;
                  sstr << std::this_thread::get_id();
                  sstr >> id;
                  return id;
            }

            void save(queue<std::vector<std::string>, std::time_t>& que,
                counters_thread_mgr& counters_thread,
                std::exception_ptr& ex_ptr)
            {
                  try
                  {
                        try
                        {
                              thread_local rnd_gen_t gen(get_tid());

                              using queue_thread_t = std::remove_reference_t<decltype(que)>;

                              while (true)
                              {
                                    auto item = que.pop_wait();
                                    if (std::get<0>(item) == queue_thread_t::exit)
                                          return;

                                    counters_thread.count_block++;

                                    std::fstream fout(get_filename(std::get<2>(item), counters_thread.number_thread, gen), std::fstream::out);
                                    for (auto cmd : std::get<1>(item))
                                    {
                                          fout << cmd << "\n";
                                          counters_thread.count_all_cmds++;
                                    }
                                    fout.flush();
                                    fout.close();
                              }
                        }
                        catch (std::bad_alloc&)
                        {
                              std::throw_with_nested(std::runtime_error("save_log_file::save() failed."));
                        }
                        catch (...)
                        {
                              std::stringstream ss;
                              ss << std::this_thread::get_id();
                              std::throw_with_nested(std::runtime_error("save_log_file::save() failed. Thread ID:" + ss.str()));
                        }
                  }
                  catch (...)
                  {
                        ex_ptr = std::current_exception();
                  }
            }
      };
}
