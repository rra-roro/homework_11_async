#pragma once
#include <exception>
#include <list>
#include <iostream>
#include <string>

namespace roro_lib
{
      void print_nested_exception(const std::exception&, int = 0);

      class exception_ptr_list
      {
        private:
            std::list<std::exception_ptr> list_exception_ptr;           

        public:
            void add_back_exception_ptr()
            {
                  list_exception_ptr.emplace_back();
            }

            void add_back_exception_ptr(const std::exception_ptr& ex_ptr)
            {
                  list_exception_ptr.emplace_back(ex_ptr);
            }

            auto& back()
            {
                  return list_exception_ptr.back();
            }

            auto& back() const
            {
                  return list_exception_ptr.back();
            }

            void clear() noexcept
            {
                  list_exception_ptr.clear();
            }

            std::size_t size() const noexcept
            {
                  return list_exception_ptr.size();
            }

            void rethrow_if_exist()
            {
                  list_exception_ptr.remove(nullptr);
                  if (!list_exception_ptr.empty()) throw *this;
            }

            exception_ptr_list& operator+=(exception_ptr_list rhs)
            {
                  this->list_exception_ptr.splice(this->list_exception_ptr.end(), rhs.list_exception_ptr);
                  return *this; 
            }

            friend exception_ptr_list operator+(exception_ptr_list lhs, 
                                                const exception_ptr_list& rhs)
            {
                  lhs += rhs;
                  return lhs;
            }           

            static void print(const exception_ptr_list& ex_list, int level = 0)
            {
                  for (auto& ex_ptr : ex_list.list_exception_ptr)
                  {
                        try
                        {
                              if (ex_ptr)
                              {
                                    std::rethrow_exception(ex_ptr);
                              }
                        }
                        catch (const std::exception& e)
                        {
                              print_nested_exception(e, level);
                        }
                  }
            }
      };

      void print_nested_exception(const std::exception& e, int level)
      {
            std::cerr << std::string(level, ' ') << "exception: " << e.what() << '\n';
            try
            {
                  rethrow_if_nested(e);
            }
            catch (const exception_ptr_list& ex_list)
            {
                  exception_ptr_list::print(ex_list, level + 1);
            }
            catch (const std::exception& ex)
            {
                  print_nested_exception(ex, level + 1);
            }
            catch (...)
            {
            }
      }
}
