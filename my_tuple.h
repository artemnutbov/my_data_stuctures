
template <size_t I, typename T>
struct derive_element : T {
    using element_type = T;

    using T::T;  // inherit any constructor

    decltype(auto) elem(this auto&& self) {
        using CastType = decltype(std::forward_like<decltype(self)>(std::declval<T>()));
        return static_cast<CastType>(self);
    }
};

template <size_t I, typename T>
struct wrap_element {
    using element_type = T;

    T value;

    template <typename... Types>  // fix for narrowing
    wrap_element(Types&&... args) : value(std::forward<Types>(args)...) {}

    decltype(auto) elem(this auto&& self) {
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
    template <typename... Ua>
    constexpr base(Ua&&... args) : wrap<Index, Types>(std::forward<Ua>(args))... {}

    constexpr base() = default;
};

template <typename... Types>
class my_tuple : public base<std::make_index_sequence<sizeof...(Types)>, Types...> {
public:
    using base = base<std::make_index_sequence<sizeof...(Types)>, Types...>;

    constexpr my_tuple()
        requires(std::is_default_constructible_v<Types> && ...)
        : base() {}

    template <typename... Ua>
    constexpr my_tuple(Ua&&... args) : base(std::forward<Ua>(args)...) {}
};

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
