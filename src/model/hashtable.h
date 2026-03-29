/**
 * @file hashtable.h
 * @brief Заголовочный файл для шаблонного класса HashTable
 *
 * @details
 * Реализация хэш-таблицы с методом цепочек (chaining) для разрешения коллизий.
 * Поддерживает итераторы, автоматическое рехэширование и семантику перемещения.
 *
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <functional>
#include <iterator>
#include <utility>
#include <cstddef>
#include <concepts>
#include <string>
#include <exception>
#include <cmath>


/**
* Концепция для проверки хешируемости
*/

template<typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

/**
* Концепция для проверки, что тип поддерживает оператор ==
*/

template<typename T>
concept EqualityComparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
    { a != b } -> std::convertible_to<bool>;
};

 /**
  * @class HashTable
  * @brief Шаблонный класс хэш-таблицы с открытой адресацией методом цепочек
  *
  * @tparam Key Тип ключа (должен поддерживать std::hash и оператор ==)
  * @tparam Value Тип значения
  *
  * @ingroup Utils
  *
  * Реализует ассоциативный контейнер с постоянным временем доступа в среднем случае.
  * Особенности:
  * - Разрешение коллизий методом цепочек
  * - Автоматическое рехэширование при достижении коэффициента загрузки 0.75
  * - Поддержка итераторов (forward iteration)
  * - Поддержка семантики перемещения (move semantics)
  * - Потокобезопасность не гарантируется
  *
  * @note Для типа Key должен быть определен std::hash<Key> и оператор ==
  * @note Коэффициент загрузки по умолчанию: 0.75
  * @note Начальная емкость по умолчанию: 32
  *
  * @see Iterator
  * @see ConstIterator
  */
template <typename Key, typename Value>

requires Hashable<Key> && EqualityComparable<Key>

class HashTable {
public:

    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = class Iterator;
    using const_iterator = class ConstIterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;



private:
    /**
     * @class Node
     * @brief Внутренний класс узла для цепочек коллизий
     *
     * @private
     * Представляет узел в цепочке коллизий. Содержит ключ, значение
     * и указатель на следующий узел в цепочке.
     */
    class Node {
    public:
        Key key;      ///< Ключ узла
        Value value;  ///< Значение узла
        Node* next;   ///< Указатель на следующий узел в цепочке

        /**
         * @brief Конструктор Node (копирование значения)
         * @param[in] key Ключ узла
         * @param[in] value Значение узла (копируется)
         *
         * @post Создан узел с заданными ключом и значением
         * @post next установлен в nullptr
         */
        Node(const Key& key, const Value& value)
            : key(key), value(value), next(nullptr)
        {
        }

        /**
         * @brief Конструктор Node (перемещение значения)
         * @param[in] key Ключ узла
         * @param[in] value Значение узла (перемещается)
         *
         * @post Создан узел с заданным ключом
         * @post Значение перемещено в узел
         * @post next установлен в nullptr
         */
        Node(const Key& key, Value&& value)
            : key(key), value(std::move(value)), next(nullptr)
        {
        }
    };

    Node** buckets;      ///< Массив указателей на цепочки узлов (бакеты)
    size_t m_size;       ///< Текущее количество элементов в таблице
    size_t m_capacity;   ///< Текущая емкость таблицы (количество бакетов)

    

public:

       //using value_type = ...
    
    class Iterator;
    class ConstIterator;

    /**
     * @brief Конструктор HashTable
     * @param[in] initCapacity Начальная емкость таблицы (по умолчанию 32)
     *
     * @pre initCapacity > 0
     * @post Создана пустая хэш-таблица с заданной емкостью
     * @post m_size = 0
     * @post Все бакеты инициализированы nullptr
     *
     * @throw std::bad_alloc Если не удалось выделить память
     */
    explicit HashTable(size_t initCapacity = 32)
        : m_size(0), m_capacity(initCapacity)
    {
        buckets = new Node * [m_capacity]();
    }

    /**
     * @brief Конструктор копирования
     * @param[in] other Таблица для копирования
     *
     * @post Создана глубокая копия other
     * @post Все элементы скопированы из other
     *
     * @complexity O(n) где n - количество элементов в other
     * @throw std::bad_alloc Если не удалось выделить память
     */
    HashTable(const HashTable& other)
        : m_size(0), m_capacity(other.m_capacity > 0 ? other.m_capacity : 32)
    {
        buckets = new Node * [m_capacity]();
        try {
            for (const auto& [key, value] : other) {
                insert(key, value);
            }
        }
        catch (...) {
            clear();
            delete[] buckets;
            throw;
        }
    }

    /**
     * @brief Конструктор перемещения
     * @param[in] other Другой HashTable для перемещения
     *
     * @post Текущий объект получает ресурсы other
     * @post other остается в валидном, но пустом состоянии
     *
     * @note noexcept гарантирует безопасность при использовании в контейнерах STL
     */
    HashTable(HashTable&& other) noexcept
        : buckets(other.buckets), m_size(other.m_size), m_capacity(other.m_capacity)
    {
        other.buckets = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    /**
     * @brief Оператор присваивания копированием
     * @param[in] other Таблица для копирования
     * @return Ссылка на текущий объект
     *
     * Использует идиому copy-and-swap. Создает временную копию через итераторы,
     * затем обменивается ресурсами.
     *
     * @post Текущий объект становится копией other
     * @post Старые ресурсы освобождены
     *
     * @complexity O(n + m) где n - размер this, m - размер other
     * @throw std::bad_alloc Если не удалось выделить память
     */
    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            HashTable temp(other);
            swap(temp);
        }
        return *this;
    }

    /**
     * @brief Оператор присваивания перемещением
     * @param[in] other Другой HashTable для перемещения
     * @return Ссылка на текущий объект
     *
     * @post Текущий объект получает ресурсы other
     * @post other остается в валидном, но пустом состоянии
     *
     */
    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            clear();
            delete[] buckets;

            buckets = other.buckets;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            other.buckets = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    /**
     * @brief Деструктор HashTable
     *
     * Удаляет все узлы и освобождает память бакетов
     *
     * @complexity O(n) где n - количество элементов
     */
    ~HashTable() {
        clear();
        delete[] buckets;
    }
 

    void rehash(size_t count) {

        const size_type min_buckets = std::max( count, static_cast<size_t>(std::ceil(m_size / max_load_factor())));
       
        if (m_capacity >= min_buckets) { return; }

        size_t newCapacity = (m_capacity == 0) ? 1 : m_capacity;

        while (newCapacity < min_buckets) {
            newCapacity *= 2;
        }

        Node** newBuckets = new Node * [newCapacity]();

        for (size_t i = 0; i < m_capacity; ++i) {
            Node* ptr = buckets[i];
            while (ptr) {
                Node* nextPtr = ptr->next;
                size_t newIndex = std::hash<Key>{}(ptr->key) % newCapacity;
                ptr->next = newBuckets[newIndex];
                newBuckets[newIndex] = ptr;
                ptr = nextPtr;
            }
        }

        delete[] buckets;
        buckets = newBuckets;
        m_capacity = newCapacity;
    }

           /**
     * @brief Получает коэффициент максимальной загрузки
     * @return Текущий коэффициент максимальной загрузки (по умолчанию 0.75f или 1.0f)
     */
    float max_load_factor() const {
        return 0.75f;  
    }



    /**
     * @brief Резервирует место для указанного количества элементов
     *
     * @param count Минимальное количество элементов для резервирования
     *
     * Гарантирует, что после вызова может быть вставлено не менее count
     * элементов без необходимости рехэширования.
     *
     * Фактически вызывает rehash(std::ceil(count / max_load_factor()))
     */
    void reserve(size_type count) {
        if (count == 0) {
            return;
        }
        size_type requiredBuckets = static_cast<size_type>(
            std::ceil(count / max_load_factor())
            );

        rehash(requiredBuckets);
    }

    /**
 * @brief Вычисляет хэш-индекс для ключа
 * @param[in] key Ключ для хэширования
 * @return Индекс бакета в диапазоне [0, m_capacity-1]
 *
 * Использует std::hash<Key> с последующим взятием остатка от деления на m_capacity.
 *
 * @note Гарантирует возврат корректного индекса даже при нулевой емкости
 */
    size_t hash(const Key& key) const {
        if (m_capacity == 0) return 0;
        return std::hash<Key>{}(key) % m_capacity;
    }



    /**
     * @brief Обменивает содержимое двух таблиц
     * @param[in] other Таблица для обмена
     *
     * @post Содержимое this и other поменялось местами
     *
     * @note noexcept гарантирует безопасность операции
     */
    void swap(HashTable& other) noexcept {
        std::swap(buckets, other.buckets);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    /**
     * @brief Очищает таблицу
     *
     * Удаляет все элементы из таблицы
     *
     * @post m_size = 0
     * @post Все бакеты равны nullptr
     *
     * @complexity O(n) где n - количество элементов
     */
    void clear() {
        for (size_t i = 0; i < m_capacity; ++i) {
            Node* current = buckets[i];
            while (current) {
                Node* next = current->next;
                delete current;
                current = next;
            }
            buckets[i] = nullptr;
        }
        m_size = 0;
    }

    /**
     * @brief Вставляет элемент с копированием значения
     * @param[in] key Ключ элемента
     * @param[in] value Значение элемента (копируется)
     * @return std::pair<Iterator, bool> где:
     *         - first: итератор на вставленный или существующий элемент
     *         - second: true если элемент был вставлен, false если ключ уже существовал
     *
     * @note Автоматически вызывает rehash() если коэффициент загрузки ≥ 0.75
     * @note Если ключ уже существует, значение не обновляется
     *
     * @complexity O(1) в среднем случае, O(n) в худшем
     * @throw std::bad_alloc Если не удалось выделить память
     */
    std::pair<Iterator, bool> insert(const Key& key, const Value& value) {
        Iterator it = find(key);
        if (it != end()) {
            return { it, false };
        }

        rehash(m_capacity);

        size_t index = hash(key);
        Node* newNode = new Node(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
        ++m_size;

        it = Iterator(this, index, newNode);
        return { it, true };
    }


    /**
     * @brief Вставляет элемент с перемещением значения
     * @param[in] key Ключ элемента
     * @param[in] value Значение элемента (перемещается)
     * @return std::pair<Iterator, bool> где:
     *         - first: итератор на вставленный или существующий элемент
     *         - second: true если элемент был вставлен, false если ключ уже существовал
     *
     * @note Аналогично insert(const Key&, const Value&), но значение перемещается
     */
    std::pair<Iterator, bool> insert(const Key& key, Value&& value) {
        Iterator it = find(key);
        if (it != end()) {
            return { it, false };
        }

        rehash(m_capacity);
        
        size_t index = hash(key);
        Node* newNode = new Node(key, std::move(value));

        newNode->next = buckets[index];
        buckets[index] = newNode;
        ++m_size;

        it = Iterator(this, index, newNode);
        return { it, true };
    }

    /**
     * @brief Вставляет элемент из пары (копирование)
     * @param[in] pair Пара ключ-значение для вставки
     * @return std::pair<Iterator, bool> (см. insert(const Key&, const Value&))
     */
    std::pair<Iterator, bool> insert(const std::pair<Key, Value>& pair) {
        return insert(pair.first, pair.second);
    }

    /**
     * @brief Вставляет элемент из пары (перемещение)
     * @param[in] pair Пара ключ-значение для вставки (перемещается)
     * @return std::pair<Iterator, bool> (см. insert(const Key&, Value&&))
     */
    std::pair<Iterator, bool> insert(std::pair<Key, Value>&& pair) {
        return insert(pair.first, std::move(pair.second));
    }

    /**
     * @brief Ищет элемент по ключу и возвращает итератор
     * @param[in] key Ключ для поиска
     * @return Iterator к найденному элементу или end() если не найден
     *
     * @complexity O(1) в среднем случае, O(n) в худшем
     */
    Iterator find(const Key& key) {
        if (m_capacity == 0) {
            return end();
        }

        size_t index = hash(key);
        Node* current = buckets[index];

        while (current) {
            if (current->key == key) {
                return Iterator(this, index, current);
            }
            current = current->next;
        }

        return end();
    }

    /**
     * @brief Ищет элемент по ключу и возвращает константный итератор
     * @param[in] key Ключ для поиска
     * @return ConstIterator к найденному элементу или end() если не найден
     *
     * @complexity O(1) в среднем случае, O(n) в худшем
     */
    ConstIterator find(const Key& key) const {
        if (m_capacity == 0) {
            return end();
        }

        size_t index = hash(key);
        const Node* current = buckets[index];

        while (current) {
            if (current->key == key) {
                return ConstIterator(this, index, current);
            }
            current = current->next;
        }

        return end();
    }

    /**
     * @brief Проверяет наличие элемента по ключу
     * @param[in] key Ключ для проверки
     * @return true если элемент существует, false в противном случае
     *
     * @complexity O(1) в среднем случае, O(n) в худшем
     */
    bool contains(const Key& key) const {
        return find(key) != end();
    }

    /**
     * @brief Проверяет, пуста ли таблица
     * @return true если таблица пуста (m_size == 0), false в противном случае
     *
     * @complexity O(1)
     */
    bool empty() const {
        return m_size == 0;
    }

    /**
     * @brief Удаляет элемент по ключу
     * @param[in] key Ключ элемента для удаления
     * @return size_t Количество удаленных элементов (0 или 1)
     *
     * @note Аналогично std::map::erase(const key_type& key)
     * @note В std::map возвращает количество удаленных элементов (0 или 1)
     *
     * @complexity O(1) в среднем случае, O(n) в худшем
     */
    size_t erase(const Key& key) {
        if (empty() || m_capacity == 0) {
            return 0;
        }

        size_t index = hash(key);
        Node* current = buckets[index];
        Node* prev = nullptr;

        while (current) {
            if (current->key == key) {
                if (prev == nullptr) {
                    buckets[index] = current->next;
                }
                else {
                    prev->next = current->next;
                }

                delete current;
                --m_size;
                return 1;
            }
            prev = current;
            current = current->next;
        }

        return 0;
    }

    /**
     * @brief Удаляет элемент по итератору
     * @param[in] pos Итератор на элемент для удаления
     * @return Iterator на элемент, следующий за удаленным
     *
     * @note Аналогично std::map::erase(iterator pos)
     * @warning Поведение не определено если pos == end()
     *
     * @complexity O(1) в среднем случае
     */
    Iterator erase(Iterator pos) {
        if (pos == end() || empty()) {
            return end();
        }

        const Key& key = (*pos).first;

        size_t index = hash(key);
        Node* current = buckets[index];
        Node* prev = nullptr;

        while (current && current->key != key) {
            prev = current;
            current = current->next;
        }

        if (!current) {
            return end();
        }
        if (!prev) {
            buckets[index] = current->next;
        }
        else {
            prev->next = current->next;
        }

        Node* nextNode = current->next;

        delete current;
        --m_size;

        if (nextNode) {
            return Iterator(this, index, nextNode);
        }
        else {
            for (size_t i = index + 1; i < m_capacity; ++i) {
                if (buckets[i]) {
                    return Iterator(this, i, buckets[i]);
                }
            }
            return end();
        }
    }

    /**
     * @brief Удаляет элемент по константному итератору
     * @param[in] pos Константный итератор на элемент для удаления
     * @return Iterator на элемент, следующий за удаленным
     *
     */
    Iterator erase(ConstIterator pos) {
        if (pos == end() || empty()) {
            return end();
        }

        Iterator it = find((*pos).first);
        return erase(it);
    }

    /**
     * @brief Удаляет диапазон элементов [first, last)
     * @param[in] first Начальный итератор диапазона
     * @param[in] last Конечный итератор диапазона (не включается)
     * @return Iterator на элемент, следующий за последним удаленным (т.е. last)
     *
     * @note Аналогично std::map::erase(iterator first, iterator last)
     *
     * @complexity O(m) где m - количество удаляемых элементов
     */
    Iterator erase(Iterator first, Iterator last) {
        if (first == end() || first == last) {
            return last;
        }

        while (first != last) {
            first = erase(first);
        }

        return last;
    }

    /**
     * @brief Получает количество элементов в таблице
     * @return Текущее количество элементов
     *
     * @complexity O(1)
     */
    size_t size() const {
        return m_size;
    }

    /**
     * @brief Получает текущую емкость таблицы
     * @return Количество бакетов в таблице
     *
     * @complexity O(1)
     */
    size_t getCapacity() const {
        return m_capacity;
    }

    /**
     * @class HashTable::Iterator
     * @brief Класс итератора для HashTable
     *
     * Реализует forward iterator для итерации по всем элементам таблицы
     * в произвольном порядке.
     *
     * @note Итератор становится недействительным при изменении таблицы
     *       (кроме удаления элемента через данный итератор)
     */
    class Iterator {
    private:
        const HashTable* table;      ///< Указатель на хэш-таблицу
        size_t bucketIndex;          ///< Текущий индекс бакета
        Node* currentNode;           ///< Текущий узел в цепочке

        /**
         * @brief Переходит к следующему элементу
         *
         * Автоматически переходит к следующему бакету при достижении конца цепочки
         */
        void advance() {
            if (currentNode) {
                currentNode = currentNode->next;
                if (currentNode) return;
            }

            ++bucketIndex;
            while (bucketIndex < table->m_capacity && !table->buckets[bucketIndex]) {
                ++bucketIndex;
            }

            if (bucketIndex < table->m_capacity) {
                currentNode = table->buckets[bucketIndex];
            }
            else {
                currentNode = nullptr;
            }
        }

    public:


        Iterator() : table(nullptr), bucketIndex(0), currentNode(nullptr) {}

        /// @name Типы итератора
        /// @{
        using iterator_category = std::forward_iterator_tag; ///< Категория итератора
        using value_type = std::pair<Key, Value>;      ///< Тип значения
        using difference_type = std::ptrdiff_t;              ///< Тип разности
        using pointer = value_type*;                         ///< Тип указателя
        using reference = value_type&;                       ///< Тип ссылки
        /// @}

        /**
         * @brief Конструктор Iterator
         * @param[in] table Указатель на хэш-таблицу
         * @param[in] bucketIndex Начальный индекс бакета
         * @param[in] node Начальный узел
         */
        Iterator(const HashTable* table, size_t bucketIndex, Node* node)
            : table(table), bucketIndex(bucketIndex), currentNode(node)
        {
        }

        /**
         * @brief Префиксный инкремент
         * @return Ссылка на текущий итератор
         */
        Iterator& operator++() {
            advance();
            return *this;
        }

        /**
         * @brief Постфиксный инкремент
         * @return Копия итератора до инкремента
         */
        Iterator operator++(int) {
            Iterator temp = *this;
            advance();
            return temp;
        }

        /**
         * @brief Оператор разыменования
         * @return Пару ключ-значение для текущего элемента
         */
        value_type operator*() const {
            return { currentNode->key, currentNode->value };
        }

        /**
         * @brief Оператор доступа к члену
         * @return Указатель на пару ключ-значение
         */
        value_type* operator->() {
            static value_type temp;
            temp = { currentNode->key, currentNode->value };
            return &temp;
        }

        /**
         * @brief Оператор сравнения на равенство
         * @param[in] other Другой итератор
         * @return true если итераторы указывают на один узел
         */
        bool operator==(const Iterator& other) const {
            return currentNode == other.currentNode;
        }

        /**
         * @brief Оператор сравнения на неравенство
         * @param[in] other Другой итератор
         * @return true если итераторы указывают на разные узлы
         */
        bool operator!=(const Iterator& other) const {
            return currentNode != other.currentNode;
        }

        // Дружественный класс для доступа к приватным членам
        friend class HashTable::ConstIterator;
    };

    /**
     * @class HashTable::ConstIterator
     * @brief Константный класс итератора для HashTable
     *
     * Аналогичен Iterator, но предоставляет доступ только для чтения.
     */
    class ConstIterator {
    private:
        const HashTable* table;      ///< Указатель на хэш-таблицу
        size_t bucketIndex;          ///< Текущий индекс бакета
        const Node* currentNode;     ///< Текущий узел в цепочке

        /**
         * @brief Переходит к следующему элементу
         */
        void advance() {
            if (currentNode) {
                currentNode = currentNode->next;
                if (currentNode) return;
            }

            ++bucketIndex;
            while (bucketIndex < table->m_capacity && !table->buckets[bucketIndex]) {
                ++bucketIndex;
            }

            if (bucketIndex < table->m_capacity) {
                currentNode = table->buckets[bucketIndex];
            }
            else {
                currentNode = nullptr;
            }
        }

    public:

        ConstIterator() : table(nullptr), bucketIndex(0), currentNode(nullptr) {}

        /// @name Типы итератора (STL совместимость)
        /// @{
        using iterator_category = std::forward_iterator_tag; ///< Категория итератора
        using value_type = std::pair<const Key, const Value>; ///< Тип значения
        using difference_type = std::ptrdiff_t;              ///< Тип разности
        using pointer = const value_type*;                   ///< Тип указателя
        using reference = const value_type&;                 ///< Тип ссылки
        /// @}

        /**
         * @brief Конструктор ConstIterator
         * @param[in] table Указатель на хэш-таблицу
         * @param[in] bucketIndex Начальный индекс бакета
         * @param[in] node Начальный узел
         */
        ConstIterator(const HashTable* table, size_t bucketIndex, const Node* node)
            : table(table), bucketIndex(bucketIndex), currentNode(node)
        {
        }

        /**
         * @brief Конструктор преобразования из Iterator
         * @param[in] it Итератор для преобразования
         */
        ConstIterator(const Iterator& it)
            : table(it.table), bucketIndex(it.bucketIndex), currentNode(it.currentNode)
        {
        }

        /**
         * @brief Префиксный инкремент
         * @return Ссылка на текущий итератор
         */
        ConstIterator& operator++() {
            advance();
            return *this;
        }

        /**
         * @brief Постфиксный инкремент
         * @return Копия итератора до инкремента
         */
        ConstIterator operator++(int) {
            ConstIterator temp = *this;
            advance();
            return temp;
        }

        /**
         * @brief Оператор разыменования
         * @return Константную пару ключ-значение для текущего элемента
         */
        value_type operator*() const {
            return { currentNode->key, currentNode->value };
        }

        /**
         * @brief Оператор доступа к члену
         * @return Константный указатель на пару ключ-значение
         */
        const value_type* operator->() const {
            static value_type temp;
            temp = { currentNode->key, currentNode->value };
            return &temp;
        }

        /**
         * @brief Оператор сравнения на равенство
         * @param[in] other Другой итератор
         * @return true если итераторы указывают на один узел
         */
        bool operator==(const ConstIterator& other) const {
            return currentNode == other.currentNode;
        }

        /**
         * @brief Оператор сравнения на неравенство
         * @param[in] other Другой итератор
         * @return true если итераторы указывают на разные узлы
         */
        bool operator!=(const ConstIterator& other) const {
            return currentNode != other.currentNode;
        }
    };

    /**
     * @brief Возвращает итератор на первый элемент таблицы
     * @return Iterator на первый элемент или end() если таблица пуста
     *
     * @complexity O(m) где m - индекс первого непустого бакета
     */
    Iterator begin() {
        for (size_t i = 0; i < m_capacity; ++i) {
            if (buckets[i]) {
                return Iterator(this, i, buckets[i]);
            }
        }
        return end();
    }

    /**
     * @brief Возвращает итератор на конец таблицы
     * @return Iterator за последним элементом
     *
     * @complexity O(1)
     */
    Iterator end() {
        return Iterator(this, m_capacity, nullptr);
    }

    /**
     * @brief Возвращает константный итератор на первый элемент таблицы
     * @return ConstIterator на первый элемент или end() если таблица пуста
     */
    ConstIterator begin() const {
        for (size_t i = 0; i < m_capacity; ++i) {
            if (buckets[i]) {
                return ConstIterator(this, i, buckets[i]);
            }
        }
        return end();
    }

    /**
     * @brief Возвращает константный итератор на конец таблицы
     * @return ConstIterator за последним элементом
     */
    ConstIterator end() const {
        return ConstIterator(this, m_capacity, nullptr);
    }

    /**
     * @brief Возвращает константный итератор на первый элемент таблицы
     * @return ConstIterator на первый элемент или cend() если таблица пуста
     */
    ConstIterator cbegin() const {
        return begin();
    }

    /**
     * @brief Возвращает константный итератор на конец таблицы
     * @return ConstIterator за последним элементом
     */
    ConstIterator cend() const {
        return end();
    }
};


namespace hash_table_minimal_tests {
    using TestTable = HashTable<int, int>;

    static_assert(std::is_same_v<TestTable::key_type, int>);
    static_assert(std::is_same_v<TestTable::mapped_type, int>);
    static_assert(std::is_same_v<TestTable::value_type, std::pair<const int, int>>);

}

namespace hash_table_string_minimal {
    using StringTable = HashTable<std::string, int>;
    static_assert(std::is_same_v<StringTable::key_type, std::string>);
}



static_assert(std::forward_iterator<HashTable<int, int>::Iterator>,
    "HashTable::Iterator должен удовлетворять концепции std::forward_iterator");

static_assert(std::forward_iterator<HashTable<int, int>::ConstIterator>,
    "HashTable::ConstIterator должен удовлетворять концепции std::forward_iterator");

static_assert(std::is_same_v<
    typename HashTable<int, int>::Iterator::iterator_category,
    std::forward_iterator_tag>,
    "Iterator должен иметь iterator_category = std::forward_iterator_tag");

static_assert(std::is_same_v<
    typename HashTable<int, int>::Iterator::value_type,
    std::pair<int, int>>,
    "Iterator::value_type должен быть std::pair<Key, Value>");

static_assert(std::is_same_v<
    typename HashTable<int, int>::Iterator::difference_type,
    std::ptrdiff_t>,
    "Iterator::difference_type должен быть std::ptrdiff_t");

static_assert(std::is_same_v<
    typename HashTable<int, int>::ConstIterator::iterator_category,
    std::forward_iterator_tag>,
    "ConstIterator должен иметь iterator_category = std::forward_iterator_tag");

static_assert(std::is_same_v<
    typename HashTable<int, int>::ConstIterator::value_type,
    std::pair<const int, const int>>,
    "ConstIterator::value_type должен быть std::pair<const Key, const Value>");

static_assert(std::is_convertible_v<
    HashTable<int, int>::Iterator,
    HashTable<int, int>::ConstIterator>,
    "Iterator должен быть конвертируем в ConstIterator");


namespace hash_table_iterator_tests {
    template<typename T>
    constexpr bool has_pre_increment() {
        return requires(T it) { ++it; };
    }

    template<typename T>
    constexpr bool has_post_increment() {
        return requires(T it) { it++; };
    }

    template<typename T>
    constexpr bool has_dereference() {
        return requires(T it) { *it; };
    }

    template<typename T>
    constexpr bool has_arrow() {
        return requires(T it) { it.operator->(); };
    }

    template<typename T>
    constexpr bool has_equality() {
        return requires(T a, T b) { a == b; };
    }

    template<typename T>
    constexpr bool has_inequality() {
        return requires(T a, T b) { a != b; };
    }
}

static_assert(hash_table_iterator_tests::has_pre_increment<HashTable<int, int>::Iterator>(),
    "Iterator должен иметь префиксный operator++");

static_assert(hash_table_iterator_tests::has_post_increment<HashTable<int, int>::Iterator>(),
    "Iterator должен иметь постфиксный operator++");

static_assert(hash_table_iterator_tests::has_dereference<HashTable<int, int>::Iterator>(),
    "Iterator должен иметь operator*");

static_assert(hash_table_iterator_tests::has_arrow<HashTable<int, int>::Iterator>(),
    "Iterator должен иметь operator->");

static_assert(hash_table_iterator_tests::has_equality<HashTable<int, int>::Iterator>(),
    "Iterator должен иметь operator==");

static_assert(hash_table_iterator_tests::has_inequality<HashTable<int, int>::Iterator>(),
    "Iterator должен иметь operator!=");


static_assert(std::is_same_v<
    decltype(std::declval<HashTable<int, int>>().begin()),
    HashTable<int, int>::Iterator>,
    "begin() должен возвращать Iterator");

static_assert(std::is_same_v<
    decltype(std::declval<const HashTable<int, int>>().begin()),
    HashTable<int, int>::ConstIterator>,
    "const begin() должен возвращать ConstIterator");

static_assert(std::is_same_v<
    decltype(std::declval<HashTable<int, int>>().end()),
    HashTable<int, int>::Iterator>,
    "end() должен возвращать Iterator");

static_assert(std::is_same_v<
    decltype(std::declval<const HashTable<int, int>>().end()),
    HashTable<int, int>::ConstIterator>,
    "const end() должен возвращать ConstIterator");


static_assert(std::is_copy_constructible_v<HashTable<int, int>::Iterator>,
    "Iterator должен быть копируемым");

static_assert(std::is_copy_assignable_v<HashTable<int, int>::Iterator>,
    "Iterator должен быть копируемо присваиваемым");

static_assert(std::is_destructible_v<HashTable<int, int>::Iterator>,
    "Iterator должен быть уничтожаемым");


namespace {
    template<typename T>
    constexpr bool test_increment_end() {
        T table;
        auto it = table.end();
        auto it2 = ++it;
        return it == table.end() && it2 == table.end();
    }
}

static_assert(requires(HashTable<int, int>::Iterator it) { ++it; },
    "Iterator должен поддерживать инкремент");


#endif // HASHTABLE_H

