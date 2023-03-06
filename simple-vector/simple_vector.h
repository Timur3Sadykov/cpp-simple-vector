#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <iostream>
#include <utility>

using namespace std::string_literals;

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve)
            :capacity_to_reserve(capacity_to_reserve) {}

    size_t capacity_to_reserve;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
            :items_(ArrayPtr<Type>(size)), capacity_(size) {
        std::fill(&items_[0], &items_[size], Type());
        size_ = size;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
            :items_(ArrayPtr<Type>(size)), capacity_(size) {
        std::fill(&items_[0], &items_[size], value);
        size_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
            :items_(ArrayPtr<Type>(init.size())),  capacity_(init.size()) {
        std::copy(std::make_move_iterator(init.begin()), std::make_move_iterator(init.end()), begin());
        size_ = init.size();
    }

    explicit SimpleVector(ReserveProxyObj proxy_obj)
            :items_(ArrayPtr<Type>(proxy_obj.capacity_to_reserve)),
             capacity_(proxy_obj.capacity_to_reserve) {}

    SimpleVector(const SimpleVector& other)
            :items_(ArrayPtr<Type>(other.GetSize())),  capacity_(other.GetCapacity()) {
        std::copy(other.begin(), other.end(), begin());
        size_ = other.GetSize();
    }

    SimpleVector(SimpleVector&& other)
            :items_(ArrayPtr<Type>(other.GetSize())),  capacity_(other.GetCapacity()) {
        std::copy(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), begin());
        size_ = std::exchange(other.size_, 0);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            ArrayPtr<Type> new_items(rhs.GetCapacity());
            std::copy(rhs.begin(), rhs.end(), new_items.Get());

            items_.swap(new_items);
            capacity_ = rhs.GetCapacity();
            size_ = rhs.GetSize();
        }

        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            ArrayPtr<Type> new_items(rhs.GetCapacity());
            std::copy(std::make_move_iterator(rhs.begin()), std::make_move_iterator(rhs.end()), new_items.Get());

            items_.swap(new_items);
            capacity_ = rhs.GetCapacity();
            size_ = std::exchange(rhs.size_, 0);
        }

        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index >= size"s);
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index >= size"s);
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size <= capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                items_[i] = Type();
            }
            size_ = new_size;
        }
        else {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_items(new_capacity);

            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_items.Get());

            for (size_t i = size_; i < new_size; ++i) {
                new_items[i] = Type();
            }

            items_.swap(new_items);
            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        if (items_) {
            return &items_[0];
        }

        return nullptr;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        if (items_) {
            return &items_[size_];
        }

        return nullptr;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        if (items_) {
            return &items_[0];
        }

        return nullptr;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        if (items_) {
            return &items_[size_];
        }

        return nullptr;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        if (items_) {
            return &items_[0];
        }

        return nullptr;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        if (items_) {
            return &items_[size_];
        }

        return nullptr;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            items_[size_] = item;
            ++size_;
        }
        else {
            size_t new_capacity;
            capacity_ == 0 ? new_capacity = 1 : new_capacity = capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);

            std::copy(begin(), end(), new_items.Get());
            new_items[size_] = item;

            items_.swap(new_items);
            ++size_;
            capacity_ = new_capacity;
        }
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            items_[size_] = std::move(item);
            ++size_;
        }
        else {
            size_t new_capacity;
            capacity_ == 0 ? new_capacity = 1 : new_capacity = capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);

            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_items.Get());
            new_items[size_] = std::move(item);

            items_.swap(new_items);
            ++size_;
            capacity_ = new_capacity;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        auto index = pos - begin();

        if (size_ < capacity_) {
            std::copy_backward(&items_[index], end(), end() + 1);
            items_[index] = value;
            ++size_;
        }
        else {
            size_t new_capacity;
            capacity_ == 0 ? new_capacity = 1 : new_capacity = capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);

            std::copy(begin(), &items_[index], new_items.Get());
            new_items[index] = value;
            std::copy(&items_[index], end(), &new_items[index + 1]);

            items_.swap(new_items);
            ++size_;
            capacity_ = new_capacity;
        }

        return &items_[index];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        auto index = pos - begin();

        if (size_ < capacity_) {
            std::copy_backward(std::make_move_iterator(&items_[index]), std::make_move_iterator(end()), end() + 1);
            items_[index] = std::move(value);
            ++size_;
        }
        else {
            size_t new_capacity;
            capacity_ == 0 ? new_capacity = 1 : new_capacity = capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);

            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(&items_[index]), new_items.Get());
            new_items[index] = std::move(value);
            std::copy(std::make_move_iterator(&items_[index]), std::make_move_iterator(end()), &new_items[index + 1]);

            items_.swap(new_items);
            ++size_;
            capacity_ = new_capacity;
        }

        return &items_[index];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        auto index = pos - begin();
        std::copy(std::make_move_iterator(&items_[index + 1]), std::make_move_iterator(end()), &items_[index]);
        --size_;

        return &items_[index];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_items.Get());

            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> items_;

    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
