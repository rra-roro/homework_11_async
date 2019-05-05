#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <iterator>
#include <regex>

typedef char TCHAR;

#define __T(x) x
#define _T(x) __T(x)
#define _TEXT(x) __T(x)

namespace Tstring
{
      extern const size_t npos;
      typedef std::string tstring;
      typedef std::string_view tstring_view;

      using std::endl;

      extern std::ostream& ucout;
      extern std::istream& ucin;

      extern std::ostream& tcout;
      extern std::istream& tcin;
      extern std::ostream& tcerr;
      extern std::ostream& tclog;

      typedef std::ostream tostream;
      typedef std::istream tistream;
      typedef std::ifstream tifstream;
      typedef std::ofstream tofstream;
      typedef std::fstream tfstream;
      typedef std::ostringstream tostringstream;
      typedef std::istringstream tistringstream;
      typedef std::stringstream tstringstream;
      typedef std::stringbuf tstringbuf;

      typedef std::basic_regex<char> tregex;                           // regex
      typedef std::match_results<const char*> tcmatch;                 // cmatch
      typedef std::match_results<std::string::const_iterator> tsmatch; // smatch

      typedef std::regex_token_iterator<const char*> tcregex_token_iterator;                 // cregex_token_iterator
      typedef std::regex_token_iterator<std::string::const_iterator> tsregex_token_iterator; // sregex_token_iterator
      typedef std::regex_iterator<const char*> tcregex_iterator;
      typedef std::regex_iterator<std::string::const_iterator> tsregex_iterator;

}
