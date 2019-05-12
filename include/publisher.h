#pragma once
#include "exception_list.h"
#include <unordered_map>
#include <list>
#include <optional>
#include <functional>
#include <type_traits>
#include <variant>

namespace roro_lib
{
      namespace internal
      {
            /*!   \brief  Эта структура является ключом для unordered_map, где хранятся ф-ии подписчиков, принимающие уведомления. 

                          На основании этого ключа можно однозначно идентифицировать исходную ф-ию подписчика. Используется для
                          контроля за тем, чтобы не добавлять уже имеющиеся ф-ии в список подписчиков.
            */
            struct key_subscriber
            {
                  using Facke = struct
                  {
                  };
                  using fn_mem_t = void (Facke::*)(void);

                  enum pointer_t
                  {
                        data_pointer,
                        fun_pointer
                  };

                  bool rvalue;
                  std::pair<std::variant<const void*, void (*)(void)>, fn_mem_t> key_value;
                  pointer_t key_type;

                  key_subscriber() = delete;

#if __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
                  template <typename T, typename FM,
                            typename FMx = void (Facke::*)(void),
                            typename std::enable_if_t<std::is_rvalue_reference_v<T&&>>* tmp = nullptr
                  >
                  key_subscriber(T&& obj, FM fn) : rvalue(true), key_type(pointer_t::data_pointer)
                  {
                        key_value.first = &obj;

                        if constexpr (std::is_same_v<FM, std::nullptr_t>)
                              key_value.second = fn;
                        else
                              key_value.second = reinterpret_cast<FMx>(fn); //  allowed cast C++17 [8.2.10/10]
                  }

                  template <typename T, typename FM,
                            typename FMx = void (Facke::*)(void),
                            typename std::enable_if_t<!std::is_rvalue_reference_v<T&&>>* tmp = nullptr
                  >
                  key_subscriber(T&& obj, FM fn) : rvalue(false), key_type(pointer_t::data_pointer)
                  {
                        key_value.first = &obj;

                        if constexpr (std::is_same_v<FM, std::nullptr_t>)
                              key_value.second = fn;
                        else
                              key_value.second = reinterpret_cast<FMx>(fn); //  allowed cast  C++17 [8.2.10/10]
                  }

#if __GNUG__
#pragma GCC diagnostic pop
#endif
                  template <typename F,
                            typename std::enable_if_t<std::is_pointer_v<F> &&
                                                      std::is_function_v<typename std::remove_pointer_t<F>>>* Facke = nullptr
                  >
                  key_subscriber(F obj) : rvalue(false), key_type(pointer_t::fun_pointer)
                  {
                        key_value.first = reinterpret_cast<void (*)(void)>(obj); // allowed cast  C++17 [8.2.10/6]
                        key_value.second = nullptr;
                  }

                  template <typename T,
                           typename std::enable_if_t<!std::is_pointer_v<T>>* Facke = nullptr
                  >
                  key_subscriber(const T& obj) : rvalue(false), key_type(pointer_t::data_pointer)
                  {
                        key_value.first = &obj;
                        key_value.second = nullptr;
                  }

                  key_subscriber(const key_subscriber& key_arg) noexcept = default;
                  key_subscriber(key_subscriber&& key_arg) noexcept = default;

                  key_subscriber& operator=(const key_subscriber& key_arg) noexcept = default;
                  key_subscriber& operator=(key_subscriber&& key_arg) noexcept = default;

                  ~key_subscriber() = default;
            };

            /*!   \brief  ф-ия сравнения двух ключей key_subscriber.  

                          Позволяет определить, есть ли в нашем unordered_map уже этот подписчик.
            */
            bool operator==(const key_subscriber& arg1, const key_subscriber& arg2) noexcept
            {
                  if (arg1.key_type != arg2.key_type)
                  {
                        return false;
                  }
                  else if (arg1.rvalue == true && arg2.rvalue == true)
                  {
                        return false; // Априори считаем, каждое rvalue уникальным подписчиком
                  }
                  else
                        return (arg1.key_value == arg2.key_value);
            }
      }
}

namespace std
{
      /*!   \brief  Это специализация std::hash для нашей структуры key_subscriber.

      */
      template <>
      struct hash<roro_lib::internal::key_subscriber>
      {
            using argument_t = roro_lib::internal::key_subscriber;
            using result_t = std::intptr_t;

            std::intptr_t intptr_cast(const void* const ptr) const noexcept
            {
                  return reinterpret_cast<std::intptr_t>(ptr); // allowed cast C++17 [8.2.10/4]
            }

            std::intptr_t intptr_cast(void (*ptr)(void)) const noexcept
            {
                  return reinterpret_cast<std::intptr_t>(ptr); //  allowed cast C++17 [8.2.10/4]
            }

            result_t operator()(const argument_t& key_arg) const noexcept
            {
                  using ptr_t = argument_t::pointer_t;

                  if (key_arg.key_type == ptr_t::data_pointer)
                        return std::hash<std::intptr_t>{}(intptr_cast(std::get<ptr_t::data_pointer>(key_arg.key_value.first)));
                  else
                        return std::hash<std::intptr_t>{}(intptr_cast(std::get<ptr_t::fun_pointer>(key_arg.key_value.first)));
            }
      };

}


namespace roro_lib
{
      template <typename>
      class publisher_mixin;

      /*!   \brief  mixin класс, реализующий функциональность паттерна Наблюдатель/Observer

                    Мы можем добавить его, как базовый класс, в любой другой класс, что автоматически добавит в этот класс функциональность паттерна `наблюдатель`
                    Класс *по умолчанию* сохраняет только уникальных подписчиков, т.е. все подписчики получат уведомления только один раз.
                    Класс параметризуется общей сигнатурой ф-ии всех подписиков, т.е. все подписчики должны соответствовать этой сигнатуре.

                     \tparam  R     -тип взвращаемого значения ф-ии подписчика
                     \tparam  Args  -тип аргументов ф-ии подписчика
      */
      template <typename R, typename... Args>
      class publisher_mixin<R(Args...)>
      {
            static_assert(sizeof...(Args) < 20,
                "the subscriber must have less 20 arguments");

            using container_subscribers_t = std::unordered_map<internal::key_subscriber, std::function<R(Args...)>>;

            using subscribers_value_type = typename container_subscribers_t::value_type;
            using subscribers_reference = typename container_subscribers_t::reference;
            using subscribers_iterator = typename container_subscribers_t::iterator;

            /*!   \brief  внутренний вспомогательнй класс, представляет собой дискрептор подписчика добавленного в список

                          Это "чистая выдумка", который однозначно идентифицирует ф-ию подписчика в unordered_map
                          В первую очередь, возвращается ф-ией добавления подписчика subscribe(), чтобы в последствии
                          при необходимости на основе этого дискрептора можно было бы отписаться от уведомления, передав
                          это дискрептор в ф-ию unsubscribe
            */
            class subscriber_handle
            {
                  subscribers_value_type null_pair = { { nullptr, nullptr }, nullptr };
                  subscribers_reference ref;

                  template <typename>
                  friend class publisher_mixin;

              public:
                  subscriber_handle() : ref(null_pair){};
                  subscriber_handle(const std::pair<subscribers_iterator, bool>& arg) : ref(*arg.first){};
                  subscriber_handle(subscribers_reference arg) : ref(arg){};
            };

        public:
            template <typename F,
                      typename std::enable_if_t<std::is_pointer_v<F> &&
                                                std::is_function_v<typename std::remove_pointer_t<F>>>* Facke = nullptr
            >
            subscriber_handle subscribe(F fn)
            {
                  static_assert(std::is_invocable_v<F, Args...>,
                      "the signature of the subscriber function must match the signature declared by the publisher");

                  return subscribe_internal(fn);
            }

            template <typename T, typename MF,
                      typename std::enable_if_t<std::is_member_function_pointer_v<MF>>* Facke = nullptr
            >
            subscriber_handle subscribe(T&& obj, MF mfn)
            {
                  static_assert(std::is_invocable_v<MF, T, Args...>,
                      "the signature of the subscriber member function must match the signature declared by the publisher");

                  return subscribe_internal(std::forward<T>(obj), mfn);
            }

            template <typename Ref_,
                      typename T = std::remove_reference_t<Ref_>,
                      typename std::enable_if_t<!std::is_function_v<typename std::remove_pointer_t<T>>>* Facke = nullptr
            >
            subscriber_handle subscribe(Ref_&& obj)
            {
                  static_assert(std::is_invocable_v<T, Args...>,
                      "the signature of the subscriber functor must match the signature declared by the publisher");

                  if constexpr (std::is_same_v<T, std::function<R(Args...)>>)
                  {
                        if (obj != nullptr) // function != nullptr
                              return subscribe_internal(std::forward<Ref_>(obj));
                        else
                              return {};
                  }
                  else
                  {
                        return subscribe_internal(std::forward<Ref_>(obj));
                  }
            }

            void unsubscribe(const subscriber_handle& handle)
            {
                  for (auto it = subscribers.begin(); it != subscribers.end(); ++it)
                  {
                        if (&*it == &handle.ref)
                        {
                              subscribers.erase(it);
                              return;
                        }
                  }
            }

            void unsubscribe_all()
            {
                  subscribers.clear();
            }

        protected:
            auto notify(Args... args)
            {
                  notify_exception_list.clear();

                  const auto try_call_fn = [&](const subscribers_reference& subscriber) -> std::conditional_t<std::is_void_v<R>, void, std::optional<R>>
                  {
                        try
                        {
                              return subscriber.second(args...);
                        }
                        catch (...)
                        {
                              notify_exception_list.add_back_exception_ptr(std::current_exception());
                        }

                        if constexpr (!std::is_void_v<R>)
                              return {};
                  };

                  if constexpr (!std::is_void_v<R>)
                  {
                        std::list<std::pair<R, subscriber_handle>> subscribers_return_list;

                        for (auto& subscriber : subscribers)
                        {
                              std::optional<R> resoult = try_call_fn(subscriber);
                              if (resoult)
                                    subscribers_return_list.emplace_back(*resoult, subscriber_handle(subscriber));
                        }

                        return subscribers_return_list;
                  }
                  else
                  {
                        for (auto& subscriber : subscribers)
                        {
                              try_call_fn(subscriber);
                        }
                  }
            }

            std::function<R(Args...)> get_fn_by_handle(const subscriber_handle& handle)
            {
                  for (auto it = subscribers.begin(); it != subscribers.end(); ++it)
                  {
                        if (&*it == &handle.ref)
                        {
                              return handle.ref.second;
                        }
                  }
                  return {};
            }

            exception_ptr_list& get_last_notify_exception() noexcept
            {
                  return notify_exception_list;
            }

            // non-virtual protected destructor special for mixin
            ~publisher_mixin(){};

        private:
            container_subscribers_t subscribers;
            exception_ptr_list notify_exception_list;


            // добавляем указатель на ф-ию
            template <typename T,
                      typename std::enable_if_t<std::is_pointer_v<T> &&
                                                std::is_function_v<typename std::remove_pointer_t<T>>>* Facke = nullptr>
            constexpr subscriber_handle subscribe_internal(const T& fn)
            {
                  return subscribers.insert({ { fn }, fn });
            }


            // добавляем функторы
            template <typename Ref_,
                      typename T = std::remove_reference_t<Ref_>,
                      typename std::enable_if_t<std::is_invocable_v<T, Args...> &&
                                                !std::is_function_v<typename std::remove_pointer_t<T>>>* Facke = nullptr>
            constexpr subscriber_handle subscribe_internal(Ref_&& fn)
            {
                  internal::key_subscriber fn_key(fn);

                  if constexpr (std::is_rvalue_reference_v<Ref_&&>)
                  {
                        fn_key.rvalue = true;
                        return subscribers.insert({ fn_key, fn });
                  }
                  else
                  {
                        fn_key.rvalue = false;
                        return subscribers.insert({ fn_key, std::ref(fn) });
                  }                        
            }

            // добавляем указатель на ф-ию член класса
            template <std::size_t I = 0,
                      typename T,
                      typename F = R (T::*)(Args...),
                      std::size_t... PhNumber
            >
            constexpr subscriber_handle subscribe_internal(T&& obj, F fn)
            {
                  if constexpr (I < sizeof...(Args))
                  {
                        return subscribe_internal<I + 1, T, F, PhNumber..., I>(obj, fn);
                  }
                  else if constexpr (sizeof...(Args) == 0)
                  {
                        if constexpr (std::is_rvalue_reference_v<T&&>)
                              return subscribers.insert({ { std::forward<T>(obj), fn }, std::bind(fn, obj) });
                        else
                              return subscribers.insert({ { std::forward<T>(obj), fn }, std::bind(fn, std::ref(obj)) });
                  }
                  else
                  {
                        using namespace std::placeholders;
                        constexpr auto placeholders_tuple = std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,
                            _11, _12, _13, _14, _15, _16, _17, _18, _19, _20);

                        if constexpr (std::is_rvalue_reference_v<T&&>)
                              return subscribers.insert({ { std::forward<T>(obj), fn },
                                                          std::bind(fn, obj, std::get<PhNumber>(placeholders_tuple)...) });
                        else
                              return subscribers.insert({ { std::forward<T>(obj), fn },
                                                          std::bind(fn, std::ref(obj), std::get<PhNumber>(placeholders_tuple)...) });
                  }
            }


#ifdef PRIVATE_TEST
            FRIEND_TEST(PublisherMixinTest, UniqueAddSubscribers1);
            FRIEND_TEST(PublisherMixinTest, UniqueAddSubscribers2);
            FRIEND_TEST(PublisherMixinTest, UniqueAddSubscribers3);
            FRIEND_TEST(PublisherMixinTest, copy);
            FRIEND_TEST(PublisherMixinTest, move);
            FRIEND_TEST(PublisherMixinTest, exeption);
            FRIEND_TEST(PublisherMixinTest, DelSubscribers);
            FRIEND_TEST(PublisherMixinTest, NotifiRetValues);
#endif
      };
}
