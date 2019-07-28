#pragma once

#include <memory>
#include <exception>
#include <type_traits>

namespace Utils {
    class NullPointerException : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Got Null Pointer Where None Null-Pointer Expected";
        }
    };

    template <class T, class = typename std::is_pointer<T>::type>
    T RequireNonNull(T pointer) {
        if (pointer) {
            return pointer;
        }
        throw NullPointerException();
    }

    template <class T>
    std::unique_ptr<T> RequireNonNull(std::unique_ptr<T>& pointer) {
        if (pointer) {
            return std::move(pointer);
        }
        throw NullPointerException();
    }

    template <class T>
    std::shared_ptr<T> RequireNonNull(std::shared_ptr<T>& pointer) {
        if (pointer) {
            return std::move(pointer);
        }
        throw NullPointerException();
    }
}
