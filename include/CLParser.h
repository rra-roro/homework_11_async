#pragma once
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//    Файл CLParser.h   - содержит объявление класса ParserСommandLine, который инкапсулирует
//                        функциональность разбора командной строки.
//
//    Версия файла 1.01  - внесены незначительные изменения
//
//

#include <vector>
#include <algorithm>
#include "tchar_dummy.h"

using std::vector;

/*
      ParserCommandLine работает с опциями, заданными по следующим правилам:

      1)        Опции и не опции могут задаваться в любом порядке.

      2)        “-- ” если встречается этот символ, то дальнейшая обработка строки прекращается
                и все что указано после него считается не именованными параметрами

      3)        “--option=abc” или “--option abc” задает  длинную опцию с параметром

      4)        Короткие опции могут быть заданы или так “-o -p –t –i” или так “-opti”,
                “-opti”    - будет списком коротких опций, если нет аналогичной длинной
                                    опции начинающейся на “-opti” например “-option”
                “-o”       - будет проанализирована, как длинная опция только, если нет
                             такой же короткой опции. Если короткая опция “-o” существует,
                             то анализ этой опции, как длинной не будет выполнен никогда.

      6)        Длинные опции могут быть сокращены, пока такое сокращение однозначно идентифицирует
                опцию. Например, если задан только список длинных опций или если задан список
                        длинных опций и коротких опций то:
                        a.        Если есть длинная опция "fubar", и ARGV-элемент "-fu",
                                Тогда "-fu" распознается, как опция "fubar"
                        b.        Если длинной опции, начинающейся на "-fu", нет, это
                                список коротких опций.

      5)        “-option=abc” или “-option abc”
                a.        если задан только список длинных опций, то это длинная опция с параметром.
                b.        если задан только список коротких опций, то это последовательность коротких опций.
                        И опция n имеет параметр “abc”
                c.        если задан список длинных опций и коротких опций, то:
                        i.        Если существует длинная опция “-option” - Это длинная опция с параметром.
                        ii.        Иначе это список коротких опций, и опция “n“ имеет параметр “abc”

      6)        Длинные опции могут быть сокращены, пока такое сокращение однозначно идентифицирует
                опцию. Например, если задан только список длинных опций или если задан список
                        длинных опций и коротких опций то:
                        a.        Если есть длинная опция "fubar", и ARGV-элемент "-fu",
                                Тогда "-fu" распознается, как опция "fubar"
                        b.        Если длинной опции, начинающейся на "-fu", нет, это
                            список коротких опций.

      7)        Если задана “-o” -короткая опция с обязательным параметром, то следующая строка
                    “-option” ошибочна,  для задания параметра требуется ставить равно или пробел.
                    “-o=ption” или “-o ption” верное задание параметра.

      Символы '-' не являются символами опции, а только разделителями опции, поэтому для задания
          формата опции этот символ не используется.

  Короткие опции задаются строкой следующего формата:
  ParserCommandLine::SetShortFormatOfArg(“ab:c::”);

  Где    а   – имя опции без параметра
         b:  – имя опции с обязательным параметром
         с:: – имя опции с не обязательным параметром

  Формат длинно-именованных опции задается функцией
  ParserCommandLine::AddFormatOfArg(tstring name,              //- длинное имя опции
                                    _argtype has_arg,
                                    TCHAR val);                //-сокращенное имя опции из одного символа

  Где  _argtype это перечисление, и
  'has_arg' может принимать следующие значения:
*/

enum _argtype
{
      no_argument,       //  (или 0), если опция без параметра,
      required_argument, //  (или 1), если опция требует параметр,
      optional_argument  //  (или 2), если опция может быть с не обязательным параметром.
};


// В случае если ParserCommandLine в процессе работы не смог распознать опцию
// Не распознанные опции помещаются в массив опций  ParserCommandLine::ErrorOption[];
// Где каждый элемент этого массива представляет из себя следующую структуру
struct ErrorOpt
{
      enum ErrorCode
      {
            unrecognized_opt, //Опция не распознана
            ambiguous_opt,    //Не однозначная опция
            not_need_arg,     //Параметр для опции не нужен
            requires_arg      //Параметр для опции нужен
      };
      ErrorCode ErrorID;       // Код ошибки
      Tstring::tstring optopt; // Имя опции которая была не распознана
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//        Здесь объявляем вспомогательные классы для представления
//        распознанных опций класса ParserCommandLine

class OptionArray;
class ParserCommandLine;

struct ItemArg //Пользовательское представление одной распознаной опции
{
      friend ParserCommandLine;
      friend OptionArray;

  private:
      bool IsSet;
      TCHAR NameOption;
      ItemArg() : IsSet(false), ParamOption(){};

  public:
      vector<Tstring::tstring> ParamOption;
      operator bool() const
      {
            return IsSet;
      }
      TCHAR GetName() const
      {
            return NameOption;
      }
};

class OptionArray //Эмуляция массива, представляющего список распознанных опций
{
      friend ParserCommandLine;
      vector<ItemArg> ListArg;
      ItemArg ItemArgEmpty;

  public:
      auto begin()
      {
            return ListArg.cbegin();
      }

      auto end()
      {
            return ListArg.cend();
      }

      const ItemArg& operator[](TCHAR Ch)
      {
            auto ItrListItemArg = find_if(ListArg.begin(), ListArg.end(),
                [=](const ItemArg& op) {
                      return op.NameOption == Ch;
                });

            if (ItrListItemArg != ListArg.end())
            {
                  return *ItrListItemArg;
            }
            else
            {
                  ItemArgEmpty.NameOption = Ch;
                  ItemArgEmpty.ParamOption.push_back(_T(""));
                  return ItemArgEmpty;
            }
      }
      size_t size() const
      {
            return ListArg.size();
      }
      bool empty() const
      {
            return ListArg.empty();
      }
};


template <class Type>
class TOptionArray //Шаблон обертка, для списка не распознанных и ошибочных опций
{
      friend ParserCommandLine;
      vector<Type> TOptionArg;

  public:
      const Type& operator[](size_t Index) const
      {
            return TOptionArg[Index];
      }
      size_t size() const
      {
            return TOptionArg.size();
      }
      bool empty() const
      {
            return TOptionArg.empty();
      }

      auto begin()
      {
            return TOptionArg.cbegin();
      }

      auto end()
      {
            return TOptionArg.cend();
      }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//        Объявляем основной пользовательский класс ParserCommandLine,
//        который парсит аргументы командной строки
//

class GetOpt;
class ParserCommandLine
{
      GetOpt* GetOptObject;

  public:
      //Список не опций
      TOptionArray<Tstring::tstring> NonOption;
      //Список ошибочных опций
      TOptionArray<ErrorOpt> ErrorOption;
      //Список опций
      OptionArray Option;

      //Конструктор и деструктор
      ParserCommandLine(void);
      ~ParserCommandLine(void);

      // Установим флаг вывода ошибок
      void SetShowError(bool ShowErrorFl);

      // Установим дополнительный символ разделитель опций
      void SetSeparatorChar(TCHAR Ch);

      // Установим формат коротких опций
      void SetShortFormatOfArg(Tstring::tstring Str);

      // Добавим формат для очередной длинной опции
      void AddFormatOfArg(Tstring::tstring name, _argtype has_arg, TCHAR val);

      // Две функции Parser() с разными входными параметрами
      template <class _Elem>
      void Parser(int argc, _Elem* argv[]);

      void Parser(vector<Tstring::tstring>& ArgV_p);

      // Еще одна функция Parser(), которая принимает для разбора  строку
      // bProgramName, этот параметр задает правила интерпретации первой опции в строке ArgV_str:
      // Если  bProgramName == true, то первая опция в строке это имя программы,
      //                             эта опция не будет проанализирована
      // Если  bProgramName == false, то в начале строки отсутствует путь и имя программы,
      //                              и первый аргумент строки будет включен в анализ опций
      //void Parser(Tstring::tstring& ArgV_str, bool bProgramName=true);
      template <class _Elem>
      void Parser(const std::basic_string<_Elem>& ArgV_str, bool bProgramName = true);
};

extern template void ParserCommandLine::Parser(int argc, char* argv[]);
extern template void ParserCommandLine::Parser(const std::string& ArgV_str, bool bProgramName);
