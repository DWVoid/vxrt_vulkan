#pragma once

#include <any>
#include <map>
#include <string>
#include <memory>
#include <vector>
#include "../util/exceptions.h"

namespace Vulkan {
    class Builder;

    class IBuilder {
    public:
        virtual ~IBuilder() noexcept = default;
        virtual void Build(Builder& builder) = 0;
    };

    class Builder {
    public:
        template <class T>
        auto& Push(const std::string& name, const T& object) {
           _results.insert_or_assign(name, std::any(object));
           return *this;
        }

        auto& Use(std::unique_ptr<IBuilder> builder) {
            _builders.push_back(std::move(builder));
            return *this;
        }

        void Build() {
            for (auto&& x : _builders) {
                x->Build(*this);
            }
        }

        std::any& Fetch(const std::string& name) {
            return _results[name];
        }

        template <class T>
        T& Fetch(const std::string& name) {
            return *Utils::RequireNonNull(std::any_cast<T>(&Fetch(name)));
        }
    private:
        std::map<std::string, std::any> _results;
        std::vector<std::unique_ptr<IBuilder>> _builders;
    };
}
