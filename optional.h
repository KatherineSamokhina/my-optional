#include <stdexcept>
#include <utility>

class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:

    //---------------------------------------
    Optional() = default;
    
    Optional(const T& value) {
        new (&data_[0]) T(value);
        is_initialized_ = true;
    }
    
    Optional(T&& value) {
        new (&data_[0]) T(std::move(value));
        is_initialized_ = true;
    }
    
    Optional(const Optional& other) {
        if (other.is_initialized_) {
            new (&data_[0]) T(*reinterpret_cast<const T*>(other.data_));
            is_initialized_ = true;
        }
    }

    Optional(Optional&& other) {
        if (other.is_initialized_) {
            new (&data_[0]) T(std::move(*reinterpret_cast<T*>(other.data_)));
            is_initialized_ = true;
        }
    }
    //---------------------------------------

    //---------------------------------------
    Optional& operator=(const T& value) {
        if (!is_initialized_) {
            new (&data_[0]) T(value);
            is_initialized_ = true;
            return *this;
        }
        *reinterpret_cast<T*>(data_) = value;
        return *this;
    }
    Optional& operator=(T&& rhs) {
        if (!is_initialized_) {
            new (&data_[0]) T(std::move(rhs));
            is_initialized_ = true;
            return *this;
        }
        *reinterpret_cast<T*>(data_) = std::move(rhs);
        return *this;
    }

    Optional& operator=(const Optional& rhs) {
        if (rhs.is_initialized_) {
            if (is_initialized_) {
                *reinterpret_cast<T*>(data_) = *reinterpret_cast<const T*>(rhs.data_);
                return *this;
            }
            else {
                new (&data_[0]) T(*reinterpret_cast<const T*>(rhs.data_));
                is_initialized_ = true;
                return *this;
            }
        }
        Reset();
        return *this;
    }
    Optional& operator=(Optional&& rhs) {
        if (rhs.is_initialized_) {
            if (is_initialized_) {
                *reinterpret_cast<T*>(data_) = std::move(*reinterpret_cast<T*>(rhs.data_));
                return *this;
            }
            else {
                new (&data_[0]) T(std::move(*reinterpret_cast<T*>(rhs.data_)));
                is_initialized_ = true;
                return *this;
            }
        }
        Reset();
        return *this;
    }
    //---------------------------------------

    ~Optional() {
        Reset();
    }

    bool HasValue() const {
        return is_initialized_;
    }

    T& operator*() {
        return *reinterpret_cast<T*>(data_);
    }
    const T& operator*() const {
        return *reinterpret_cast<const T*>(data_);
    }
    T* operator->() {
        return reinterpret_cast<T*>(data_);
    }
    const T* operator->() const {
        return reinterpret_cast<const T*>(data_);
    }

    T& Value() {
        if (is_initialized_)
            return *reinterpret_cast<T*>(data_);
        throw BadOptionalAccess();
    }
    const T& Value() const {
        if (is_initialized_)
            return *reinterpret_cast<const T*>(data_);
        throw BadOptionalAccess();
    }

    void Reset() {
        if (is_initialized_) {
            (*reinterpret_cast<T*>(data_)).~T();
            is_initialized_ = false;
        }
    }

    template <typename... Types>
    void Emplace(Types&&... values) {
        if (is_initialized_) {
            Reset();
        }
        new (&data_[0]) T(std::forward<Types>(values)...);
        is_initialized_ = true;
    }

private:
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
};