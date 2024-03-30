//
// Created by ben on 3/30/24.
//

#ifndef BOOKSIM2_OUTPUT_COLOR_H
#define BOOKSIM2_OUTPUT_COLOR_H


#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <random>

#if __cplusplus >= 201703L
#define COLOR_CONSTEXPR_FOR_IF constexpr
#else
#define COLOR_CONSTEXPR_FOR_IF
#endif

namespace color_ostream {
    enum class clr : uint8_t {
        grey = 30,
        red = 31,
        green = 32,
        yellow = 33,
        blue = 34,
        magenta = 35,
        cyan = 36,
        white = 37,
        on_grey = 40,
        on_red = 41,
        on_green = 42,
        on_yellow = 43,
        on_blue = 44,
        on_magenta = 45,
        on_cyan = 46,
        on_white = 47,
        reset
    };

    template<class CharT, class Traits>
    std::basic_ostream <CharT, Traits> &operator<<(std::basic_ostream <CharT, Traits> &ostream, const clr color) {
        if (color == clr::reset) {
            if COLOR_CONSTEXPR_FOR_IF (std::is_same<CharT, char>::value)
                ostream << "\033[m";
            else
                ostream << L"\033[m";
        } else {
            if COLOR_CONSTEXPR_FOR_IF (std::is_same<CharT, char>::value)
                ostream << "\033[" << static_cast<uint32_t>(color) << "m";
            else
                ostream << L"\033[" << static_cast<uint32_t>(color) << L"m";

        }
        return ostream;
    }

    constexpr std::array<clr, 8>
    clrs {
    clr::red, clr::blue, clr::cyan, clr::green, clr::grey, clr::magenta, clr::white,
    clr::yellow
};

template<typename T>
using io_manipulator = std::integral_constant<bool,
        std::is_same<T, decltype(std::showpoint)>::value ||
        std::is_same<T, decltype(std::setw(1))>::value ||
        std::is_same<T, decltype(std::setbase(1))>::value ||
        std::is_same<T, decltype(std::setfill(1))>::value ||
        std::is_same<T, decltype(std::setprecision(1))>::value ||
        std::is_same<T, decltype(std::get_time(nullptr, "1"))>::value ||
        std::is_same<T, decltype(std::get_time(nullptr, L"1"))>::value ||
        std::is_same<T, decltype(std::quoted("1"))>::value ||
        std::is_same<T, decltype(std::quoted(L"1"))>::value ||
        std::is_same<T, decltype(std::resetiosflags(std::ios_base::dec))>::value>;

template<typename char_type, typename traits_type, typename generator>
class color_ostream : public std::basic_ostream<char_type, traits_type> {
    generator generator_;
public:
    inline explicit color_ostream(std::basic_streambuf <char_type, traits_type> *_sb)
            : std::basic_ostream<char_type, traits_type>(_sb) {}

    using ostream = std::basic_ostream<char_type, traits_type>;

    template<typename T>
#if __cplusplus > 201703L
    requires
#if defined(__cpp_concepts) && __cpp_concepts
                 requires (T a, ostream os){os << a;} &&
#endif
                                                         (!io_manipulator<T>::value)
#endif
    inline color_ostream &operator<<(T t) {
#if __cplusplus <= 201703L
        if COLOR_CONSTEXPR_FOR_IF(!io_manipulator<T>::value)
#endif
            static_cast<ostream &>(*this) << generator_.get_color();
        static_cast<ostream &>(*this) << t;
        return *this;
    }

    inline color_ostream &operator<<(const char_type *str) {
        for (size_t i{}; i < std::char_traits<char_type>::length(str); ++i)
            operator<<(str[i]);
        return *this;
    }

    inline color_ostream &operator<<(ostream &(*_pf)(ostream &)

    ) {
        return static_cast<color_ostream &>(_pf(static_cast<ostream &>(*this)));
    }
};

class random_generator {
    std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution <size_t> dis{0, 7};
public:
    [[nodiscard]] inline clr get_color() { return clrs[dis(gen)]; }
};

class circle_generator {
    size_t i{};
public:
    [[nodiscard]] inline clr get_color() { return clrs[i = i == 7 ? 0 : i + 1]; }
};

template<typename CharT>
class random_generator_truecolor {
    std::ranlux24_base gen{std::random_device{}()};
    std::uniform_int_distribution <size_t> dis{0, 255};
public:
    [[nodiscard]] std::basic_string <CharT> get_color() {
        std::basic_ostringstream <CharT> buffer;
        if COLOR_CONSTEXPR_FOR_IF (std::is_same<CharT, char>::value)
            buffer << "\x1b[38;2";
        else
            buffer << L"\x1b[38;2";
        for (size_t i{}; i < 3; ++i)
            buffer << static_cast<CharT>(';') << std::setw(3)
                   << std::setfill(static_cast<CharT>('0')) << dis(gen);

        buffer << static_cast<CharT>('m');
        return move(*buffer.rdbuf()).str();
    }
};

template<typename CharT>
class random_generator_256color {
    std::ranlux24_base gen{std::random_device{}()};
    std::uniform_int_distribution <size_t> dis{0, 255};
public:
    [[nodiscard]] inline std::basic_string <CharT> get_color() {
        std::basic_ostringstream <CharT> buffer;
        if COLOR_CONSTEXPR_FOR_IF (std::is_same<CharT, char>::value)
            buffer << "\x1b[38;5";
        else
            buffer << L"\x1b[38;5";
        buffer << static_cast<CharT>(';') << std::setw(3)
               << std::setfill(static_cast<CharT>('0')) << dis(gen) << static_cast<CharT>('m');
        return move(*buffer.rdbuf()).str();
    }
};

[[maybe_unused]]random_generator random_color; // NOLINT(cert-err58-cpp)

#define COLORFUL(x) \
    using os##x = decltype(std::x);\
    color_ostream<typename os##x::char_type, typename os##x::traits_type, circle_generator> cc_##x(std::x.rdbuf());\
    color_ostream<typename os##x::char_type, typename os##x::traits_type, random_generator> rd_##x(std::x.rdbuf());\
    color_ostream<typename os##x::char_type, typename os##x::traits_type, random_generator_truecolor<typename os##x::char_type>> rdtrue_##x(std::x.rdbuf());\
    color_ostream<typename os##x::char_type, typename os##x::traits_type, random_generator_256color<typename os##x::char_type>> rd256_##x(std::x.rdbuf());

#define W_COLORFUL(x) COLORFUL(x) COLORFUL(w##x)

W_COLORFUL(cout) // NOLINT(cert-err58-cpp)
W_COLORFUL(cerr) // NOLINT(cert-err58-cpp)
W_COLORFUL(clog) // NOLINT(cert-err58-cpp)
}


#endif //BOOKSIM2_OUTPUT_COLOR_H
