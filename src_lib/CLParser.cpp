//////////////////////////////////////////////////////////////////////////////////////////////////
//
//    Файл CLParser.cpp - содержит реализацию класса ParserCommandLine, который инкапсулирует
//                        функциональность разбора командной строки. Так же в этом файле
//                        содержится объявление и реализация класса GetOpt, который является оберткой
//                        вокруг GNU ф-ии _getopt_internal(), которая и выполняет всю черновую работу по
//                        разбору командной строки.
//
//    Версия файла 1.01  - внесены незначительные изменения
//
//

#include <CLParser.h>
#include "tchar_dummy.h"

using namespace std;
using namespace Tstring;

//=====================================================================================================
/*         class GetOpt   - является оберткой вокруг переделанной GNU ф-ии _getopt_internal().
 *                          Эта функция была тщательно переделана. На текущий момент ф-ия _getopt_internal()
 *                          и класс GetOpt, реализуют PERMUTE ф-иональность прежней GNU ф-ии.
 *                          Это значит, что в процессе анализа ARGV мы переставляем содержимое ARGV,
 *                          так, что бы все не опции оказались в конце списка ARGV. Это позволяет задавать опции
 *                          в любом порядке.
 */
class GetOpt
{
  private:
      /////////////////////////////////////////////////////////////////////////////////////////
      //Если процесс сканирование коротких опций еще не закончен, то true

      bool StatusScanShortOption;

      /////////////////////////////////////////////////////////////////////////////////////////
      // struct Option    - задает формат длинной опции

      struct Option
      {
            Tstring::tstring name; //Полное имя опции
            _argtype has_arg;      //Задаем необходимость параметров
            TCHAR ChArg;           //Символьный псевдоним опции
      };

      // long_opts_array   - список форматов длинных опций
      vector<Option> long_opts_array;

      /////////////////////////////////////////////////////////////////////////////////////////
      // Если не опции предшествуют опциям, меняем их местами
      // Это индексы параметров не опций которые были пропущены и нуждаются в перестановке.

      size_t first_nonopt; // Index в ARGV первого из этих параметров
      size_t last_nonopt;  // Index в ARGV последнего из этих параметров

      /////////////////////////////////////////////////////////////////////////////////////////
      // Функция, которая меняет местами опции и не опции

      void exchange(vector<tstring>& argv);

      /////////////////////////////////////////////////////////////////////////////////////////
      // Введем функцию, которая определяет переданная строка содержит опцию или нет
      // Возвращает, true если строка - это не опция

      bool IsNoneOption(tstring& ArgStr)
      {
            return (!IsSeparatorTChar(ArgStr[0]) || ArgStr[1] == _T('\0'));
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      // Введем функцию, которая определяет начинается ли опция с "--" или  другого двойного разделителя
      // Возвращает true если опция имеет длинный разделитель

      bool IsLoongSeparator(tstring& ArgStr)
      {
            return IsSeparatorTChar(ArgStr[0]) && IsSeparatorTChar(ArgStr[1]);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      // Введем функцию, которая определяет является ли переданный символ, символом разделителем
      // Возвращает true если переданный символ, это символ разделитель

      bool IsSeparatorTChar(TCHAR Ch)
      {
            return (Ch == _T('-') || Ch == this->SeparatorChar);
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      // Группа функция анализирующая опции
      // AnalizInitialisation() - предварительный анализ ARGV и вызов exchange() при необходимости
      // AnalizLongOption()     - распознавание длинных опций
      // AnalizShortOption()    - распознавание коротких опций

      int AnalizInitialisation(vector<tstring>& argv);
      int AnalizLongOption(tstring& OptionStr, tstring_view ParamStr, vector<Option>::iterator* ptrLongOptInd);
      int AnalizShortOption(tstring& OptionStr, tstring_view ParamStr);


  public:
      /////////////////////////////////////////////////////////////////////////////////////////
      //   Если  ShowErrorFlag == true, то выводим в cerr сообщение об ошибках (по умолчанию)
      //   Если  ShowErrorFlag == false, то не выводим

      bool ShowErrorFlag;

      /////////////////////////////////////////////////////////////////////////////////////////
      //   Основной символ разделитель опций '-'. В эту переменную можно записать дополнительный
      //   символ разделитель, который отделяет одну опцию от другой. По умолчанию конструктор
      //   задает дополнительный символ разделитель равным '/'.

      TCHAR SeparatorChar;

      /////////////////////////////////////////////////////////////////////////////////////////
      //  Содержит строку, в которой описан формат коротких опций

      tstring short_opts_str;

      /////////////////////////////////////////////////////////////////////////////////////////
      // В эту переменную помещается распознанный параметр опции, если _getopt_internal() его находит

      tstring optarg;

      /////////////////////////////////////////////////////////////////////////////////////////
      // Если мы не можем распознать опцию, или у опции нет обязательного параметра
      // То код ошибки или имя проблемной опции возвращается в эту структуру,
      // в момент когда _getopt_internal() возвращает 0;

      ErrorOpt OptOpt;

      /////////////////////////////////////////////////////////////////////////////////////////
      //    Индекс в ARGV следующего элемента, который сканируется.
      //    Когда cсканирование закончено и _getopt_internal() возвращает EOF
      //    optindex - содержит значение индекса первого из элементов, которые не являются опцией,
      //    после сканирования все не опции будут размещены в конце ARGV
      //    Не опции вызывающая программа должна сканировать самостоятельно.

      size_t optindex;

      /////////////////////////////////////////////////////////////////////////////////////////
      //    Конструктор класса. Инициализируем данные необходимые для работы ф-ии_getopt_internal()
      //    Начинаем обрабатывать опции с ARGV-элемента 1. так как ARGV-элемент 0 является именем программы

      GetOpt()
      {
            first_nonopt = last_nonopt = optindex = 1;
            StatusScanShortOption = false;
            ShowErrorFlag = true;
            SeparatorChar = _T('/');
            short_opts_str = _T("");
      }

      /////////////////////////////////////////////////////////////////////////////////////////
      //    Основная функция анализирующая ARGV элементы. При каждом вызове возвращает либо символьный
      //    Псевдоним короткой опции, либо 0 в случае ошибки распознавания, либо EOF в случае если все распознано

      int _getopt_internal(vector<tstring>& argv, vector<Option>::iterator* ptrLongOptInd);

      /////////////////////////////////////////////////////////////////////////////////////////
      //    Функия, которая инициализирует список содержащий формат длинных опций.

      void AddFormatOfArg(Tstring::tstring name, _argtype has_arg, TCHAR val)
      {
            Option Temp_Otion;
            Temp_Otion.name = name;
            Temp_Otion.has_arg = has_arg;
            Temp_Otion.ChArg = val;
            long_opts_array.push_back(Temp_Otion);
      }
};

//=====================================================================================================
/*                               Реализация класса-       class GetOpt
*/

void GetOpt::exchange(vector<tstring>& argv)
{
      /* Поменять две смежных под последовательности ARGV.
        Одна под последовательность - множество элементов [first_nonopt,last_nonopt),
        который содержит все не опции, которые были пока пропущены.
        Другое множество элементов - это [last_nonopt, optindex), которое содержит все
        обработанные опции, так как выше описанные не опции были пропущены.

        Перемещение осуществляется так, что после перемещения 'first_nonopt' и 'last_nonopt'
        содержат новые значения индексов не опций в последовательности ARGV, полученных после
        перемещения. */
      size_t nonopts_size = last_nonopt - first_nonopt;

      /*Interchange the two blocks of data in ARGV.  */
      tstring* temp = new tstring[nonopts_size];

      for (size_t i = 0; i < nonopts_size; i++)
      {
            temp[i] = argv[first_nonopt + i];
      }

      for (size_t i = 0; i < (optindex - last_nonopt); i++)
      {
            argv[first_nonopt + i] = argv[last_nonopt + i];
      }

      for (size_t i = 0; i < nonopts_size; i++)
      {
            argv[first_nonopt + optindex - last_nonopt + i] = temp[i];
      }


      /* Update records for the slots the non-options now occupy.  */

      first_nonopt += (optindex - last_nonopt);
      last_nonopt = optindex;
}

//----------------------------------------------------------------------------------------------------

int GetOpt::AnalizInitialisation(vector<tstring>& argv)
{

      // Если  у нас есть не опции в начале argv, то их нужно переставить после всех опций в argv
      {
            // Если мы только что обработали некоторые опции, которые располагаются
            // после некоторых не опций, поменяем их местами так, чтобы опции были
            // на первом месте.

            if (this->first_nonopt != this->last_nonopt && this->last_nonopt != this->optindex)
                  exchange(argv);
            else if (this->last_nonopt != this->optindex)
                  first_nonopt = optindex;

            // Теперь пропустим любые дополнительные не опции и расширим диапазон ранее пропущенных не опций.

            while (this->optindex < argv.size() && IsNoneOption(argv[this->optindex]))
                  this->optindex++;

            this->last_nonopt = this->optindex;
      }

      //  Специальный ARGV-элемент '--' означает преждевременный конец опций.
      //  Пропускаем его, как нулевую опцию, затем меняем местами с предыдущими
      //  не опциями, как будто элемент '--' была опция. Затем останавливаем сканирование.
      //  Пропуская все остальное, как не опции. В результате все опции и не опции, что стоят после '--'
      //  попадут в список не распознаных опций NotIdentifyArg, как и все не опции, что стояли до '--'

      if (this->optindex != argv.size() && argv[this->optindex] == _T("--"))
      {
            this->optindex++;

            if (this->first_nonopt != this->last_nonopt && this->last_nonopt != this->optindex)
                  exchange(argv);
            else if (this->first_nonopt == this->last_nonopt)
                  this->first_nonopt = this->optindex;

            this->last_nonopt = argv.size();
            this->optindex = argv.size(); // optindex  на конец списка, что приведет к окончанию сканирования
      }

      // Если мы обработали все ARGV-элементы, остановим сканирование
      // this->optindex будет указывать на первую не опцию

      if (this->optindex == argv.size())
      {
            // Установим optindex на первую  не опцию, которые мы ранее
            // пропустили, таким образом вызывающая программа сможет получить их.
            if (this->first_nonopt != this->last_nonopt)
                  this->optindex = this->first_nonopt;

            return EOF;
      }

      return 0;
}

//----------------------------------------------------------------------------------------------------

int GetOpt::AnalizLongOption(tstring& OptionStr, tstring_view ParamStr, vector<Option>::iterator* ptrLongOptInd)
{
      // Очистим строку опции от лидирующих разделителей "-" или "--" или "/"
      tstring OptionStrWithoutSeparator = OptionStr.substr(IsSeparatorTChar(OptionStr[0]) + IsSeparatorTChar(OptionStr[1]));

      // Найдем позицию символа "=", отделяющего Имя опции от Параметра опции
      size_t PosSeparator = OptionStrWithoutSeparator.find_first_of(_T('='));

      // Получим имя текущей опции
      tstring NameOptionStr = OptionStrWithoutSeparator.substr(0, PosSeparator);

      // Получим параметр текущей опции
      tstring ParamOptionStr = (PosSeparator != npos) ? OptionStrWithoutSeparator.substr(PosSeparator + 1) : tstring();


      bool ambig = false; // Флаг устанавливается в true если найденная опция двусмысленна
      bool exact = false; // Флаг устанавливается в true если найденная опция строго
          // соответствует одной из опций в списке

      vector<Option>::iterator ptrFoundOption = this->long_opts_array.end();

      // Тестируем текущую опцию на точное или на сокращенные соответствие одному из
      // шаблонов длинных опций из long_opts_array
      for (auto ptrOption = this->long_opts_array.begin(); ptrOption != this->long_opts_array.end(); ptrOption++)
      {
            // Если текущая опция NameOptionStr является началом шаблона длинной опции ptrOption
            if (ptrOption->name.find(NameOptionStr) == 0)
            {
                  if (ptrOption->name.length() == NameOptionStr.length()) // и если их длинна совпадает
                  {
                        ptrFoundOption = ptrOption;
                        exact = true; // Строгое соответствие найдено.
                        break;        // Дальше можно не сканировать
                  }
                  else if (ptrFoundOption == this->long_opts_array.end())
                  {
                        ptrFoundOption = ptrOption; // Первое не строгое соответствие найдено.
                  }
                  else
                        ambig = true; // Второе не строгое соответствие найдено.
            }
      }

      //В случае не строго соответствия выведем ошибку
      if (ambig && !exact)
      {
            optindex++;

            ErrorOpt Error;
            Error.ErrorID = ErrorOpt::ambiguous_opt;
            Error.optopt = OptionStr;
            throw Error;
      }

      //Если опция определена, получим для нее параметры.
      if (ptrFoundOption != this->long_opts_array.end())
      {
            //Указываем на следующий элемент, возможно параметр опции
            optindex++;

            //Если у нас остался не обработанный параметр опции, в строке опции
            if (!ParamOptionStr.empty())
            {
                  if (ptrFoundOption->has_arg != no_argument) //Если параметр необходим
                        optarg = ParamOptionStr;
                  else
                  {
                        //Если параметр есть, но он не нужен
                        ErrorOpt Error;
                        Error.ErrorID = ErrorOpt::not_need_arg;
                        Error.optopt = OptionStr.substr(0, OptionStr.find_first_of(_T('=')));
                        throw Error;
                  }
            }
            else
                //Если в строке опции нет параметра, но он необходим для этой опции
                if (ptrFoundOption->has_arg != no_argument)
            {
                  switch (ptrFoundOption->has_arg)
                  {
                  case required_argument: // Параметр обязателен
                        if (ParamStr != _T(""))
                        {
                              optarg = ParamStr; // Получаем параметр
                              optindex++;        // переходим к следующему элементу
                        }
                        else
                        { // Его нет, ошибка
                              ErrorOpt Error;
                              Error.ErrorID = ErrorOpt::requires_arg;
                              Error.optopt = OptionStr;
                              throw Error;
                        }
                        break;

                  case optional_argument: // Параметр не обязателен
                        if (ParamStr != _T(""))
                        {
                              optarg = ParamStr; // Получаем параметр
                              optindex++;        // переходим к следующему элементу
                        }
                  default:
                        break;
                  }
            }

            // Параметры получены успешно, опция определена,
            // вернем в ptrLongOptInd указатель на правило в long_opts_array, которое
            // определило опцию.
            if (ptrLongOptInd != NULL)
                  *ptrLongOptInd = ptrFoundOption;
            // И вернем символ найденной опции
            return ptrFoundOption->ChArg;
      }
      // Опция не найдена, вернем 0, что позволит попытаться
      // проанализировать опцию, как короткую опцию, если short_opts_str определена
      return 0;
}

//----------------------------------------------------------------------------------------------------

int GetOpt::AnalizShortOption(tstring& ArgStr, tstring_view ParamStr)
{
      // Короткие символы могут быть заданы или так “-o –t –i –o –n” или так “-otion”
      // Мы должны пробежаться по строке OptionStr, и проанализировать все
      // входящие в нее символы, не увеличивая optindex пока не дойдем до конца OptionStr
      // Что бы сохранить состояние ф-ии при повторном входе в нее, и
      // возобновить сканирование строки с места, где мы его прервали,
      // зададим указатель nextchar и прочее, как статическии переменные ф-ии.
      // Сканирование продолжаем пока nextchar не укажет на конец OptionStr.

      // Инициализация статических переменных будет выполнена один раз
      // при первом вызове AnalizShortOption()

      static tstring OptionStr = ArgStr;                        // Скопируем в статическую переменную, строку с опцией
      static tstring::iterator BeginArgStr = OptionStr.begin(); // Зададим начало опциональной строки
      static tstring::iterator EndArgStr = OptionStr.end();     // Зададим конец опциональной строки
      static tstring::iterator nextchar = BeginArgStr + 1;      // опции начинающиеся на '--' этой ф-ией мы не обрабатываем.

      // Если нам передали новую группу коротких опций
      if (OptionStr != ArgStr)
      {
            OptionStr = ArgStr;
            BeginArgStr = OptionStr.begin();
            EndArgStr = OptionStr.end();
            nextchar = BeginArgStr + 1;
      }

      StatusScanShortOption = true;

      //Прочитаем новый символ опции из строки опций, и найдем его в short_opts_str
      TCHAR c = *nextchar++;
      size_t IndexShOtion = short_opts_str.find(c);

      // Увеличим значение 'optindex', когда мы дошли до последнего символа OptionStr.
      if (nextchar == EndArgStr)
      {
            ++optindex;
            StatusScanShortOption = false;
      }

      //Еслии символ опции не найден вернем ошибку
      if (IndexShOtion == npos || c == _T(':')) //':' не может быть использована, как короткий символ опции
      {
            ErrorOpt Error;
            Error.ErrorID = ErrorOpt::unrecognized_opt;
            Error.optopt = tstring(_T("-")) + c;
            throw Error;
      }

      // Если опции нужен параметр получим его
      if (short_opts_str[IndexShOtion + 1] == _T(':'))
      {
            if (short_opts_str[IndexShOtion + 2] == _T(':'))
            {
                  /* Эта опция требует не обязательные параметры.  */

                  // Если параметров нет у опции
                  if (!(nextchar != EndArgStr && nextchar[0] == _T('=')) &&
                      !(nextchar == EndArgStr && ParamStr != _T("") && !IsSeparatorTChar(ParamStr[0])))
                        optarg.clear();

                  // Если параметры опции записаны через пробел, то это последний символ опции и следовательно
                  // мы уже увеличивали `optindex'; увеличим его еще раз после получения параметра
                  if (nextchar == EndArgStr && ParamStr != _T("") && !IsSeparatorTChar(ParamStr[0]))
                  {
                        optarg = ParamStr;
                        optindex++;
                  }

                  // Если параметры опции записаны через равно
                  if (nextchar != EndArgStr && nextchar[0] == _T('='))
                  {
                        optarg = OptionStr.substr(nextchar - BeginArgStr + 1);
                        optindex++;
                        nextchar = EndArgStr;
                        StatusScanShortOption = false;
                  }
            }
            else
            {
                  /* Эта опция требует обязательные параметры */

                  //Если параметр задан через символ равно
                  if (nextchar != EndArgStr && nextchar[0] == _T('='))
                  {
                        optarg = OptionStr.substr(nextchar - BeginArgStr + 1);
                        optindex++;
                  }

                  // Если параметры опции записаны через пробел, то это последний символ опции и следовательно
                  // мы уже увеличивали `optindex'; увеличим его еще раз после получения параметра
                  if (nextchar == EndArgStr && ParamStr != _T(""))
                  {
                        optarg = ParamStr;
                        optindex++;
                  }

                  //В случае если у опции нет параметра
                  if (!(nextchar != EndArgStr && nextchar[0] == _T('=')) &&
                      !(nextchar == EndArgStr && ParamStr != _T("")))
                  {
                        ErrorOpt Error;
                        Error.ErrorID = ErrorOpt::requires_arg;
                        Error.optopt = tstring(_T("-")) + c;
                        throw Error;
                  }
                  nextchar = EndArgStr;
                  StatusScanShortOption = false;
            }
      }
      return c; //Вернем найденный символ
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*
   На вход ф-ии _getopt_internal() подается:
      vector<tstring>& argv    - список аргументов, для анализа
      Order ordering           - метод обработки аргументов
          iterator* ptrLongOptInd  - сюда возвращаем указатель на правило
                                     из списка форматов длинных опций,
                                     которое распознало опцию.

   Если '_getopt_internal()' неоднократно вызывается, то при каждом вызове эта ф-ия возвращает
   последовательно, короткие символы опций для каждой найденной опции.

   Функция _getopt_internal() работает с членами класса GetOpt.

   Функция _getopt_internal() вызывает три функции члена класса GetOpt:
      AnalizInitialisation()   - Предварительная обработка argv, и упорядочение ее элементов
          AnalizLongOption()       - Анализ длинных опций
          AnalizShortOption()      - Анализ коротких опций

   Если '_getopt_internal()' находит следующий символ опции, он возвращает этот символ,
   обновляя 'GetOpt::optindex', если обрабатывается короткие опции, то эта ф-ия возвращает
   символ короткой опции, и 'GetOpt::optindex' не изменяется между вызовами '_getopt_internal()'
   пока все короткие опции в одном элементе  argv, не будут проанализированы.

   Если нет больше символов опции, '_getopt_internal()' возвращает `EOF'.
   'GetOpt::optindex' - при этом будет указывать на первый символ не опцию.
   (ф-ия AnalizInitialisation() переставит argv-элементы так, чтобы все не опции
    оказались в конце списка опций)

   Если найдена опция, которая не может быть распознана, ни как короткая опция, ни как длинная,
   или возникла иная ошибка, то ф-ия '_getopt_internal()' возвращает 0. Информация об ошибке
   помещается в OptOpt. И выводим сообщение об ошибке, если 'ShowErrorFlag' установлен в true.

   Если у опции есть параметры они будут возвращены в 'optarg', иначе 'optarg'
   устанавливается в нуль.

*/

int GetOpt::_getopt_internal(vector<tstring>& argv, vector<Option>::iterator* ptrLongOptInd)

{
      this->optarg.clear();

      //----------------1-й IF----------------------
      //  AnalizInitialisation() анализирует ordering, и выполняет соответствующие действия
      //  Выборка следующего элемента argv для сканирования.
      //  AnalizInitialisation() переставляет местами строки в argv,
      //  в случае если необходимо, что бы все не опции были в конце
      //  Если опция это '--' AnalizInitialisation() прерывает сканирование

      if (!StatusScanShortOption)
      {
            int Ret = AnalizInitialisation(argv);
            if (Ret != 0)
                  return Ret;
      }

      // Если во время анализа опций возникает ошибка, отловим ее
      try
      {
            //----------------2-й IF-----------------------
            // Интерпретируем текущую опцию как длинную опцию.
            // Это возможно если задан long_opts_array, и опция задана, как длинная '--'
            // или если опция не начинается с символа короткой опции.

            if (!StatusScanShortOption && !this->long_opts_array.empty() &&
                (IsLoongSeparator(argv[optindex]) ||
                    argv[optindex].size() > 2 ||
                    short_opts_str.find(argv[optindex][1]) == npos))
            {
                  tstring empty_param(_T(""));
                  // Проверим существует ли вероятный параметр опции,
                  // или у нас последняя опция в списке или параметр пропущен и далее идет другая опция
                  tstring_view ParamStr = ((optindex + 1) < argv.size() &&
                                              !IsSeparatorTChar(argv[this->optindex + 1][0]))
                                              ? argv[this->optindex + 1]
                                              : empty_param;

                  int Ret = AnalizLongOption(argv[this->optindex], ParamStr, ptrLongOptInd);
                  // AnalizLongOption() возвращает 0, если опция не была найдена среди длинных опций
                  // Если 0 значит попытаемся искать среди коротких опций, иначе опция определена,
                  // И мы возвращаем символ найденной опции.
                  if (Ret != 0)
                        return Ret;
            }

            //-----------------3-й IF ----------------------
            // Интерпретируем опцию, как короткую опцию.
            // Мы последовательно пройдемся по всем символам опции, на предмет выявления в них коротких опций
            // optindex не будет увеличен пока, мы не дойдем до последнего символа текущей опции

            if (!short_opts_str.empty() &&
                !IsSeparatorTChar(argv[this->optindex][1]) &&
                short_opts_str.find(argv[this->optindex][1]) != npos)
            {
                  tstring empty_param(_T(""));
                  // Если short_opts_str не пустая строка и текущая опция не начинается с '--'
                  // продолжим сканировать argv[this->optindex], на предмет поиска коротких символов
                  tstring_view ParamStr = ((optindex + 1) < argv.size() &&
                                              !IsSeparatorTChar(argv[this->optindex + 1][0]))
                                              ? argv[this->optindex + 1]
                                              : empty_param;
                  // Cмотрим и обработаем следующую символ опции
                  return AnalizShortOption(argv[this->optindex], ParamStr);
            }
            else
            {
                  // Не возможно найти текущую опцию, как длинную опцию и как короткую опцию
                  ErrorOpt Error;
                  Error.ErrorID = ErrorOpt::unrecognized_opt;
                  Error.optopt = argv[this->optindex];
                  optindex++;
                  throw Error;
            }
      }
      catch (ErrorOpt Error)
      {
            // При необходимости выведим сообщение об ошибке
            if (ShowErrorFlag)
            {
                  tstring ErrorMsg;
                  switch (Error.ErrorID)
                  {
                  case ErrorOpt::unrecognized_opt:
                        ErrorMsg = _T("Unrecognized option '") + Error.optopt + _T("'");
                        break;

                  case ErrorOpt::ambiguous_opt:
                        ErrorMsg = _T("Оption '") + Error.optopt + _T("' is ambiguous");
                        break;

                  case ErrorOpt::not_need_arg:
                        ErrorMsg = _T("Option `") + Error.optopt + _T("' doesn't allow an argument");
                        break;

                  case ErrorOpt::requires_arg:
                        ErrorMsg = _T("Option `") + Error.optopt + _T("' requires an argument");
                        break;
                  }

                  tcerr << argv[0] << _T(": ") << ErrorMsg << _T("\n");
            }

            OptOpt = Error;

            return 0; //Вернем ноль в случае ошибки
      }
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//                             Реализация класса     ParserCommandLine
//

ParserCommandLine::ParserCommandLine(void)
{
      GetOptObject = new GetOpt;
}

ParserCommandLine::~ParserCommandLine(void)
{
      delete GetOptObject;
}


void ParserCommandLine::SetShowError(bool ShowErrorFl)
{
      GetOptObject->ShowErrorFlag = ShowErrorFl;
}

void ParserCommandLine::SetSeparatorChar(TCHAR Ch)
{
      GetOptObject->SeparatorChar = Ch;
}

void ParserCommandLine::SetShortFormatOfArg(Tstring::tstring Str)
{
      GetOptObject->short_opts_str = Str;
}

void ParserCommandLine::AddFormatOfArg(Tstring::tstring name, _argtype has_arg, TCHAR val)
{
      GetOptObject->AddFormatOfArg(name, has_arg, val);
}

// Две функции Parser() с разными входными параметрами
template <class _Elem>
void ParserCommandLine::Parser(int argc, _Elem* argv[])
{
      // сконвертируем стандартный вид списка аргументов ф-ии main()
      // в аналогичный список строк tstring
      vector<tstring> ArgV_p;
      ArgV_p.resize(argc, _T(""));

      for (int i = 0; i < argc; i++)
      {
            ArgV_p[i] = argv[i];
      }

      Parser(ArgV_p);
}

template void ParserCommandLine::Parser(int argc, char* argv[]);

template <class _Elem>
void ParserCommandLine::Parser(const std::basic_string<_Elem>& ArgV_str, bool bProgramName)
{
      // 1) Конвертируем строку параметров ArgV_str из произвольного типа в TempStr типа tstring
      // 2) Пробежимся по строке TempStr и разобьем ее на подстроки с помощью tregex
      //    либо подстрока в двойных кавычках \"[^\"]+\"
      //    либо подстрока в одинарных кавычках \'[^\']+\'
      //    либо подстрока без пробельных символов \\S+
      // 3) Из полученной подстроки уберем обрамляющие кавычки с помощью лямбды
      // 4) И вставим результирующую строку в конец вектора ArgV_p

      tstring TempStr = ArgV_str;

      vector<tstring> ArgV_p;

      if (bProgramName == false)
            ArgV_p.push_back(_T(""));

      tregex treg_exp(_T("(\"[^\"]+\"|\'[^\']+\'|[^\\s]+)"));

      transform(tsregex_token_iterator(TempStr.begin(), TempStr.end(), treg_exp, 1),
          tsregex_token_iterator(),
          back_inserter(ArgV_p),
          [](const tstring& str) {
                if ((str[0] == _T('\"') && str[str.size() - 1] == _T('\"')) ||
                    (str[0] == _T('\'') && str[str.size() - 1] == _T('\'')))
                      return str.substr(1, str.size() - 2);
                else
                      return str;
          });

      Parser(ArgV_p);
}

template void ParserCommandLine::Parser(const std::string& ArgV_str, bool bProgramName);

void ParserCommandLine::Parser(vector<tstring>& ArgV_p)
{
      TCHAR EnableOption = 0; //После вызова _getopt_internal() содержит символ псевдоним найденной опции

      //Разбор коммандной строки
      while ((EnableOption = GetOptObject->_getopt_internal(ArgV_p, 0)) != (TCHAR)EOF)
      {
            if (EnableOption != 0) //В случае ошибки возвращается 0
            {
                  //Проверим нет ли в списке опций такой опции
                  auto ItrListItemArg = find_if(Option.ListArg.begin(), Option.ListArg.end(),
                      [=](const ItemArg& op) {
                            return op.NameOption == EnableOption;
                      });

                  if (ItrListItemArg != Option.ListArg.end())
                  {
                        //Если опция встретилась повторно, добавим новый параметр опции если он есть
                        if (GetOptObject->optarg != _T(""))
                              ItrListItemArg->ParamOption.push_back(GetOptObject->optarg);
                  }
                  else
                  {
                        //Опция встретилась в первый раз, сохраним ее в списке
                        ItemArg TempItem;
                        TempItem.NameOption = EnableOption;
                        TempItem.IsSet = true;
                        if (GetOptObject->optarg != _T(""))
                              TempItem.ParamOption.push_back(GetOptObject->optarg);
                        Option.ListArg.push_back(TempItem);
                  }
            }
            else
            {
                  //Сохраним ошибочные опции в списке ошибочных опций
                  ErrorOption.TOptionArg.push_back(GetOptObject->OptOpt);
            }
      }

      //Сохраним не опции в списке неопций
      if (GetOptObject->optindex < ArgV_p.size())
      {
            NonOption.TOptionArg.clear();
            NonOption.TOptionArg.resize(ArgV_p.size() - GetOptObject->optindex, tstring());
            for (size_t i = 0; (GetOptObject->optindex + i) < ArgV_p.size(); i++)
            {
                  NonOption.TOptionArg[i] = ArgV_p[GetOptObject->optindex + i];
            }
      }
}
