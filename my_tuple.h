
template <size_t I, typename T>
struct derive_element : T {
    using element_type = T;

    using T::T;  // inherit any constructor
    template <typename U>
        requires(!std::is_same_v<std::remove_cvref_t<U>, derive_element>)
    constexpr derive_element(U&& other) : T(std::forward<U>(other)) {}

    constexpr decltype(auto) elem(this auto&& self) {
        using CastType = decltype(std::forward_like<decltype(self)>(std::declval<T>()));
        return static_cast<CastType>(self);
    }
};

template <size_t I, typename T>
struct wrap_element {
    using element_type = T;

    T value;

    template <typename... Types>  // fix for narrowing
    constexpr wrap_element(Types&&... args) : value(std::forward<Types>(args)...) {}

    constexpr decltype(auto) elem(this auto&& self) {
        return std::forward_like<decltype(self)>(self.value);
    }
};

template <size_t I, typename T>
using wrap = std::conditional_t<std::is_empty<T>() && !std::is_final<T>() && !std::is_union<T>(),
                                derive_element<I, T>, wrap_element<I, T>>;

template <typename I, typename... Types>
class base;

template <size_t... Index, typename... Types>
class base<std::index_sequence<Index...>, Types...> : public wrap<Index, Types>... {
public:
    template <typename... UTypes>
    constexpr base(UTypes&&... args)
        requires(sizeof...(Types) > 0)
        : wrap<Index, Types>(std::forward<UTypes>(args))... {}

    constexpr base(const Types&... args)
        requires(sizeof...(Types) > 0)
        : wrap<Index, Types>(args)... {}

    constexpr base() = default;
    constexpr base(const base& other) = default;
    constexpr base(base&& other) = default;
};

template <typename T>
void test_implicit_default(T);

template <typename T>
concept implicitly_default_constructible = requires { test_implicit_default<T>({}); };

template <typename... Types>
class my_tuple;

template <size_t I, typename T>
derive_element<I, T> deduce_base_type(const derive_element<I, T>*);

template <size_t I, typename T>
wrap_element<I, T> deduce_base_type(const wrap_element<I, T>*);

template <typename T, size_t I>
derive_element<I, T> deduce_base_type(const derive_element<I, T>*);

template <typename T, size_t I>
wrap_element<I, T> deduce_base_type(const wrap_element<I, T>*);

template <typename T>
struct is_my_tuple : std::false_type {};

template <typename... Types>
struct is_my_tuple<my_tuple<Types...>> : std::true_type {};

template <typename T>
constexpr bool is_my_tuple_v = is_my_tuple<T>::value;

template <typename T>
concept MyTuple = is_my_tuple_v<std::remove_cvref_t<T>>;

template <size_t I>
constexpr decltype(auto) get(MyTuple auto&& t) noexcept {
    using BaseType = decltype(deduce_base_type<I>(&t));
    using Type = std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(t)>>,
                                    const BaseType&, BaseType&>;

    auto& ref = static_cast<Type>(t);
    return std::forward_like<decltype(t)>(ref).elem();
}

template <typename T>
constexpr decltype(auto) get(MyTuple auto&& t) noexcept {
    using BaseType = decltype(deduce_base_type<T>(&t));
    using Type = std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(t)>>,
                                    const BaseType&, BaseType&>;

    auto& ref = static_cast<Type>(t);
    return std::forward_like<decltype(t)>(ref).elem();
}

template <typename... Types>
constexpr my_tuple<Types&...> tie(Types&... args) noexcept {
    return {args...};
}

template <typename... Types>
constexpr my_tuple<Types&&...> forward_as_tuple(Types&&... args) noexcept {
    return my_tuple<Types&&...>(std::forward<Types>(args)...);
}

template <typename... Types>
class my_tuple : public base<std::make_index_sequence<sizeof...(Types)>, Types...> {
private:
    template <typename SourceTuple, typename... FWDUTypes>
    static constexpr bool is_valid_conversion() {
        return (
            (sizeof...(Types) == sizeof...(FWDUTypes)) &&
            (std::is_constructible_v<Types, FWDUTypes> && ...) &&
            ((sizeof...(Types) != 1) || !((std::is_convertible_v<SourceTuple, Types> ||
                                           std::is_constructible_v<Types, SourceTuple> ||
                                           std::is_same_v<Types, std::remove_cvref_t<FWDUTypes>>) &&
                                          ...)) &&
            !(std::reference_constructs_from_temporary_v<Types, FWDUTypes> || ...));
    }

    template <typename... UTypes, size_t... Index>
    constexpr my_tuple(const my_tuple<UTypes...>& other, std::index_sequence<Index...>)
        : base_type(get<Index>(other)...) {}

    template <typename... UTypes, size_t... Index>
    constexpr my_tuple(my_tuple<UTypes...>& other, std::index_sequence<Index...>)
        : base_type(get<Index>(other)...) {}

    template <typename... UTypes, size_t... Index>
    constexpr my_tuple(my_tuple<UTypes...>&& other, std::index_sequence<Index...>)
        : base_type(get<Index>(std::move(other))...) {}

    template <typename... UTypes, size_t... Index>
    constexpr my_tuple(const my_tuple<UTypes...>&& other, std::index_sequence<Index...>)
        : base_type(get<Index>(std::move(other))...) {}

public:
    using base_type = base<std::make_index_sequence<sizeof...(Types)>, Types...>;

    constexpr my_tuple()
        requires(sizeof...(Types) == 0)
    = default;

    explicit(!(implicitly_default_constructible<Types> && ...)) constexpr my_tuple()
        requires(sizeof...(Types) > 0 && (std::is_default_constructible_v<Types> && ...))
        : base_type() {}

    explicit(!(std::is_convertible_v<const Types&, Types> &&
               ...)) constexpr my_tuple(const Types&... args)
        requires(sizeof...(Types) >= 1 && (std::is_copy_constructible_v<Types> && ...))
        : base_type(args...) {}

    template <typename... UTypes>
    explicit(!(std::is_convertible_v<UTypes, Types> && ...)) constexpr my_tuple(UTypes&&... args)
        requires((sizeof...(Types) == sizeof...(UTypes)) && (sizeof...(Types) >= 1) &&
                 (std::is_constructible_v<Types, UTypes> && ...) &&
                 !(sizeof...(Types) == 1 && (MyTuple<UTypes> && ...)) &&
                 !(std::reference_constructs_from_temporary_v<Types, UTypes &&> || ...))
        : base_type(std::forward<UTypes>(args)...) {}

    template <typename... UTypes>
    explicit(!(std::is_convertible_v<const UTypes&, Types> &&
               ...)) constexpr my_tuple(const my_tuple<UTypes...>& other)
        requires(is_valid_conversion<const my_tuple<UTypes...>&, const UTypes&...>())
        : my_tuple(other, std::make_index_sequence<sizeof...(Types)>{}) {}

    template <typename... UTypes>
    explicit(!(std::is_convertible_v<UTypes&, Types> &&
               ...)) constexpr my_tuple(my_tuple<UTypes...>& other)
        requires(is_valid_conversion<my_tuple<UTypes...>&, UTypes&...>())
        : my_tuple(other, std::make_index_sequence<sizeof...(Types)>{}) {}

    template <typename... UTypes>
    explicit(!(std::is_convertible_v<UTypes&&, Types> &&
               ...)) constexpr my_tuple(my_tuple<UTypes...>&& other)
        requires(is_valid_conversion<my_tuple<UTypes...> &&, UTypes && ...>())
        : my_tuple(std::move(other), std::make_index_sequence<sizeof...(Types)>{}) {}

    template <typename... UTypes>
    explicit(!(std::is_convertible_v<const UTypes&&, Types> &&
               ...)) constexpr my_tuple(const my_tuple<UTypes...>&& other)
        requires(is_valid_conversion<const my_tuple<UTypes...> &&, const UTypes && ...>())
        : my_tuple(std::move(other), std::make_index_sequence<sizeof...(Types)>{}) {}
};
