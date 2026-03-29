#include <gtest/gtest.h>
#include "model/hashtable.h"

class HashTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        table = new HashTable<std::string, int>();
    }

    void TearDown() override {
        delete table;
    }

    HashTable<std::string, int>* table;
};

TEST_F(HashTableTest, EmptyTable) {
    EXPECT_TRUE(table->empty());
    EXPECT_EQ(table->size(), 0);
    EXPECT_EQ(table->getCapacity(), 32); 
}

TEST_F(HashTableTest, InsertAndFind) {
    auto result1 = table->insert("key1", 100);
    EXPECT_TRUE(result1.second);
    EXPECT_EQ((*result1.first).second, 100);

    auto result2 = table->insert("key2", 200);
    EXPECT_TRUE(result2.second);

    EXPECT_FALSE(table->empty());
    EXPECT_EQ(table->size(), 2);

    auto it1 = table->find("key1");
    EXPECT_NE(it1, table->end());
    EXPECT_EQ((*it1).second, 100);

    auto it2 = table->find("key2");
    EXPECT_NE(it2, table->end());
    EXPECT_EQ((*it2).second, 200);

    auto it3 = table->find("key3");
    EXPECT_EQ(it3, table->end());
}

TEST_F(HashTableTest, InsertDuplicate) {
    table->insert("key1", 100);
    auto result = table->insert("key1", 200);

    EXPECT_FALSE(result.second); 
    EXPECT_EQ(table->size(), 1);

    auto it = table->find("key1");
    EXPECT_EQ((*it).second, 100); 
}

TEST_F(HashTableTest, ContainsAndErase) {
    table->insert("key1", 100);
    table->insert("key2", 200);
    table->insert("key3", 300);

    EXPECT_TRUE(table->contains("key1"));
    EXPECT_TRUE(table->contains("key2"));
    EXPECT_FALSE(table->contains("key4"));

    size_t erased = table->erase("key2");
    EXPECT_EQ(erased, 1);
    EXPECT_EQ(table->size(), 2);
    EXPECT_FALSE(table->contains("key2"));

    erased = table->erase("key4");
    EXPECT_EQ(erased, 0);
    EXPECT_EQ(table->size(), 2);
}

TEST_F(HashTableTest, IteratorOperations) {
    table->insert("a", 1);
    table->insert("b", 2);
    table->insert("c", 3);

    int sum = 0;
    for (auto it = table->begin(); it != table->end(); ++it) {
        sum += (*it).second;
    }
    EXPECT_EQ(sum, 6);

    auto it = table->begin();
    auto firstKey = (*it).first;
    ++it;
    EXPECT_NE((*it).first, firstKey);

    it = table->begin();
    auto copy = it++;
    EXPECT_EQ((*copy).first, firstKey);
}

TEST_F(HashTableTest, ClearTable) {
    table->insert("key1", 100);
    table->insert("key2", 200);

    EXPECT_EQ(table->size(), 2);
    table->clear();

    EXPECT_TRUE(table->empty());
    EXPECT_EQ(table->size(), 0);
    EXPECT_EQ(table->find("key1"), table->end());
}

TEST_F(HashTableTest, Rehashing) {
    for (int i = 0; i < 100; ++i) {
        table->insert("key" + std::to_string(i), i);
    }

    EXPECT_EQ(table->size(), 100);
    EXPECT_GT(table->getCapacity(), 32); 

    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(table->contains("key" + std::to_string(i)));
    }
}

TEST_F(HashTableTest, MoveSemantics) {
    table->insert("key1", 100);
    table->insert("key2", 200);

    HashTable<std::string, int> movedTable = std::move(*table);

    EXPECT_TRUE(table->empty()); 
    EXPECT_EQ(movedTable.size(), 2);
    EXPECT_TRUE(movedTable.contains("key1"));
    EXPECT_TRUE(movedTable.contains("key2"));
}

TEST_F(HashTableTest, CopySemantics) {
    table->insert("key1", 100);
    table->insert("key2", 200);

    HashTable<std::string, int> copiedTable(*table);

    EXPECT_EQ(table->size(), 2);
    EXPECT_EQ(copiedTable.size(), 2);

    table->erase("key1");
    EXPECT_EQ(table->size(), 1);
    EXPECT_EQ(copiedTable.size(), 2);
    EXPECT_TRUE(copiedTable.contains("key1"));
}