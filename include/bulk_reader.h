#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <exception>
#include "publisher.h"
#include "counters.h"

namespace roro_lib
{
      class command_reader : public publisher_mixin<void(const std::vector<std::string>&, std::time_t)>
      {
            struct cmd_reader_internal_status
            {
                  std::vector<std::string> command_list;
                  size_t count_cmd_bulk = 0;
                  std::time_t time_first_cmd = 0;
                  long long count_bracket = 0;
            };

            cmd_reader_internal_status cmd_ris;
            counters_command_reader reader_counters;
            size_t size_bulk;

        public:
            command_reader(size_t size_bulk) : size_bulk(size_bulk){};

            void read(std::istream& in = std::cin)
            {
                  try
                  {
                        for (std::string line; getline(in, line);) // For exit by EOF. (Console Linux Ctrl+D. Console Windows Ctrl+Z)
                        {
                              reader_counters.count_string++;

                              if (cmd_ris.count_cmd_bulk == 0)
                                    cmd_ris.time_first_cmd = get_time();

                              if (line == "{")
                              {
                                    if (cmd_ris.count_bracket == 0)
                                    {
                                          notify_subscribers(cmd_ris.command_list, cmd_ris.time_first_cmd);
                                          cmd_ris.count_cmd_bulk = 0;
                                    }
                                    cmd_ris.count_bracket++;
                                    continue;
                              }

                              if (line == "}")
                              {
                                    cmd_ris.count_bracket--;
                                    if (cmd_ris.count_bracket < 0)
                                          throw std::runtime_error("found not a pair bracket");

                                    if (cmd_ris.count_bracket == 0)
                                    {
                                          notify_subscribers(cmd_ris.command_list, cmd_ris.time_first_cmd);
                                          cmd_ris.count_cmd_bulk = 0;
                                    }
                                    continue;
                              }

                              cmd_ris.command_list.push_back(line);
                              reader_counters.count_all_cmds++;

                              if (cmd_ris.count_cmd_bulk == size_bulk - 1 && cmd_ris.count_bracket == 0)
                              {
                                    notify_subscribers(cmd_ris.command_list, cmd_ris.time_first_cmd);
                                    cmd_ris.count_cmd_bulk = 0;
                                    continue;
                              }
                              cmd_ris.count_cmd_bulk++;
                        }
                  }
                  catch (...)
                  {
                        std::throw_with_nested(std::runtime_error("command_reader::read() failed."));
                  }
            };

            counters_command_reader get_counters() const noexcept
            {
                  return reader_counters;
            }

            void flush()
            {
                  if (cmd_ris.count_bracket == 0)
                  {
                        notify_subscribers(cmd_ris.command_list, cmd_ris.time_first_cmd);
                        cmd_ris.count_cmd_bulk = 0;
                  }
            }

        private:
            std::time_t get_time() const noexcept
            {
                  return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            }

            void notify_subscribers(std::vector<std::string>& command_list, std::time_t time_first_cmd)
            {
                  try
                  {
                        if (!command_list.empty())
                        {
                              reader_counters.count_block++;
                              notify(command_list, time_first_cmd);
                              command_list.clear();
                        }

                        get_last_notify_exception().rethrow_if_exist();
                  }
                  catch (...)
                  {
                        std::throw_with_nested(std::runtime_error("command_reader::notify_subscribers() failed."));
                  }
            }
      };
}
