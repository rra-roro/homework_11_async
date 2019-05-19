#pragma once
#include <cstdlib>
#include <iostream>

namespace roro_lib
{
      struct counters_thread_mgr
      {
            std::size_t number_thread = 0;
            std::size_t count_block = 0;
            std::size_t count_all_cmds = 0;

            counters_thread_mgr() noexcept = default;
            counters_thread_mgr(std::size_t number_thread) noexcept : number_thread(number_thread){};
            counters_thread_mgr(const counters_thread_mgr&) noexcept = default;
            counters_thread_mgr(counters_thread_mgr&&) noexcept = default;
            counters_thread_mgr& operator=(const counters_thread_mgr&) noexcept = default;
            counters_thread_mgr& operator=(counters_thread_mgr&&) noexcept = default;
      };

      std::ostream& operator<<(std::ostream& out, const counters_thread_mgr& counters)
      {
            out << counters.count_block << " bloks; " << counters.count_all_cmds << " cmds";
            return out;
      }

      struct counters_command_reader
      {
            std::size_t count_string = 0;
            std::size_t count_block = 0;
            std::size_t count_all_cmds = 0;
      };

      std::ostream& operator<<(std::ostream& out, const counters_command_reader& counters)
      {
            out << counters.count_string << " strings; " << counters.count_block << " bloks; " << counters.count_all_cmds << " cmds";
            return out;
      }
}
