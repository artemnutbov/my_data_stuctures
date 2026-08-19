
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
        using ConstT = std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>,
                                          const T, T>;
        using CastType =
            std::conditional_t<std::is_lvalue_reference_v<decltype(self)>, ConstT&, ConstT&&>;

        return static_cast<CastType>(self.value);
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

template <typename T>
struct my_tuple_size;

template <typename... Types>
struct my_tuple_size<my_tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template <typename T>
constexpr size_t my_tuple_size_v = my_tuple_size<std::remove_cvref_t<T>>::value;

template <size_t I, typename T>
struct my_tuple_element;

template <size_t I, typename... Types>
struct my_tuple_element<I, my_tuple<Types...>> {
    using WrapperType = decltype(deduce_base_type<I>(std::declval<my_tuple<Types...>*>()));
    using type = typename WrapperType::element_type;
};

template <size_t I, typename T>
struct my_tuple_element<I, const T> {
    using type = const typename my_tuple_element<I, T>::type;
};

template <size_t I, typename T>
using my_tuple_element_t = typename my_tuple_element<I, T>::type;

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
constexpr my_tuple<Types&&...> forward_as_my_tuple(Types&&... args) noexcept {
    return my_tuple<Types&&...>(std::forward<Types>(args)...);
}

template <typename... Types>
constexpr my_tuple<std::unwrap_ref_decay_t<Types>...> make_my_tuple(Types&&... args) {
    return my_tuple<std::unwrap_ref_decay_t<Types>...>(std::forward<Types>(args)...);
}

template <size_t I, typename IndexSeq>
struct repeat_index;

template <size_t I, size_t... Js>
struct repeat_index<I, std::index_sequence<Js...>> {
    using type = std::index_sequence<((void)Js, I)...>;
};

template <size_t I, typename IndexSeq>
using repeat_index_t = repeat_index<I, IndexSeq>::type;

template <typename... Seqs>
struct concat_seq;

template <>
struct concat_seq<> {
    using type = std::index_sequence<>;
};

template <size_t... A>
struct concat_seq<std::index_sequence<A...>> {
    using type = std::index_sequence<A...>;
};

template <size_t... A, size_t... B, typename... Rest>
struct concat_seq<std::index_sequence<A...>, std::index_sequence<B...>, Rest...> {
    using type = typename concat_seq<std::index_sequence<A..., B...>, Rest...>::type;
};

template <typename... Seqs>
using concat_seq_t = typename concat_seq<Seqs...>::type;

template <size_t... I, size_t... J, typename Tuple>
constexpr auto my_tuple_cat_impl_2(std::index_sequence<I...>, std::index_sequence<J...>,
                                   Tuple&& all_tuples) {
    return my_tuple<my_tuple_element_t<
        J, std::remove_reference_t<decltype(get<I>(std::forward<Tuple>(all_tuples)))>>...>(
        get<J>(get<I>(std::forward<Tuple>(all_tuples)))...);
}

template <size_t... Indices, typename... Tuples>
constexpr auto my_tuple_cat_impl_1(std::index_sequence<Indices...>, Tuples&&... args) {
    auto all_tuples = forward_as_my_tuple(std::forward<Tuples>(args)...);
    return my_tuple_cat_impl_2(
        concat_seq_t<
            repeat_index_t<Indices, std::make_index_sequence<my_tuple_size_v<Tuples>>>...>{},
        concat_seq_t<std::make_index_sequence<my_tuple_size_v<Tuples>>...>{},
        std::move(all_tuples));
}

template <typename... Tuples>
constexpr auto my_tuple_cat(Tuples&&... args) {
    return my_tuple_cat_impl_1(std::make_index_sequence<sizeof...(Tuples)>{},
                               std::forward<Tuples>(args)...);
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

    constexpr my_tuple(const my_tuple& other)
        requires(std::is_copy_constructible_v<Types> && ...)
    = default;

    constexpr my_tuple(my_tuple&& other)
        requires(std::is_move_constructible_v<Types> && ...)
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
                 !(sizeof...(Types) == 1 &&
                   (std::is_same_v<std::remove_cvref_t<UTypes>, my_tuple> && ...)) &&
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

    constexpr my_tuple& operator=(const my_tuple& other)
        requires(std::is_copy_assignable_v<Types> && ...)
    = default;
    constexpr my_tuple& operator=(my_tuple&& other)
        requires(std::is_move_assignable_v<Types> && ...)
    = default;

    template <typename... UTypes>
    constexpr my_tuple& operator=(const my_tuple<UTypes...>& other)
        requires(sizeof...(Types) == sizeof...(UTypes) &&
                 (std::is_assignable_v<Types&, const UTypes&> && ...))
    {
        [&]<size_t... I>(std::index_sequence<I...>) {
            ((get<I>(*this) = get<I>(other)), ...);
        }(std::make_index_sequence<sizeof...(Types)>{});

        return *this;
    }
};
