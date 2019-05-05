#include "tchar_dummy.h"

using namespace std;
using namespace Tstring;

namespace Tstring
{
      ostream& tcout = cout;
      istream& tcin = cin;
      ostream& tcerr = cerr;
      ostream& tclog = clog;

      const size_t npos = std::string::npos;
}
