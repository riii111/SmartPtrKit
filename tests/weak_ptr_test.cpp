#include <gtest/gtest.h>
#include "weak_ptr.hpp"

class Resource {
public:
    Resource() : m_id(next_id++) {}
    Resource(int value) : m_id(next_id++), m_value(value) {}
    ~Resource() { destroyed++; }
    
    int id() const { return m_id; }
    int value() const { return m_value; }
    
    static void reset() { next_id = 0; destroyed = 0; }
    static int destroyed;
    
private:
    int m_id;
    int m_value = 0;
    static int next_id;
};

int Resource::next_id = 0;
int Resource::destroyed = 0;

class WeakPointerTests : public ::testing::Test {
protected:
    void SetUp() override {
        Resource::reset();
    }
};

TEST_F(WeakPointerTests, DefaultConstruction) {
    sptr::weak_ptr<Resource> ptr;
    EXPECT_TRUE(ptr.expired());
    EXPECT_EQ(ptr.use_count(), 0);
}

TEST_F(WeakPointerTests, SharedPtrConstruction) {
    sptr::shared_ptr<Resource> shared(new Resource(123));
    sptr::weak_ptr<Resource> weak(shared);
    
    EXPECT_FALSE(weak.expired());
    EXPECT_EQ(weak.use_count(), 1);
    EXPECT_EQ(shared.use_count(), 1);
    
    auto locked = weak.lock();
    EXPECT_TRUE(locked);
    EXPECT_EQ(locked->value(), 123);
    EXPECT_EQ(locked.use_count(), 2);
    EXPECT_EQ(shared.use_count(), 2);
}

TEST_F(WeakPointerTests, CopyConstruction) {
    sptr::shared_ptr<Resource> shared(new Resource(123));
    sptr::weak_ptr<Resource> weak1(shared);
    sptr::weak_ptr<Resource> weak2(weak1);
    
    EXPECT_FALSE(weak1.expired());
    EXPECT_FALSE(weak2.expired());
    EXPECT_EQ(weak1.use_count(), 1);
    EXPECT_EQ(weak2.use_count(), 1);
    
    auto locked1 = weak1.lock();
    auto locked2 = weak2.lock();
    EXPECT_TRUE(locked1);
    EXPECT_TRUE(locked2);
    EXPECT_EQ(locked1.get(), locked2.get());
}

TEST_F(WeakPointerTests, MoveConstruction) {
    sptr::shared_ptr<Resource> shared(new Resource(123));
    sptr::weak_ptr<Resource> weak1(shared);
    sptr::weak_ptr<Resource> weak2(std::move(weak1));
    
    EXPECT_TRUE(weak1.expired());
    EXPECT_FALSE(weak2.expired());
    EXPECT_EQ(weak1.use_count(), 0);
    EXPECT_EQ(weak2.use_count(), 1);
    
    auto locked = weak2.lock();
    EXPECT_TRUE(locked);
    EXPECT_EQ(locked->value(), 123);
}

TEST_F(WeakPointerTests, ExpiredPointer) {
    sptr::weak_ptr<Resource> weak;
    
    {
        sptr::shared_ptr<Resource> shared(new Resource(123));
        weak = shared;
        EXPECT_FALSE(weak.expired());
        EXPECT_EQ(weak.use_count(), 1);
    }
    
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(weak.use_count(), 0);
    EXPECT_FALSE(weak.lock());
}

TEST_F(WeakPointerTests, Reset) {
    sptr::shared_ptr<Resource> shared(new Resource(123));
    sptr::weak_ptr<Resource> weak(shared);
    
    EXPECT_FALSE(weak.expired());
    
    weak.reset();
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(weak.use_count(), 0);
}

TEST_F(WeakPointerTests, Swap) {
    sptr::shared_ptr<Resource> shared1(new Resource(1));
    sptr::shared_ptr<Resource> shared2(new Resource(2));
    
    sptr::weak_ptr<Resource> weak1(shared1);
    sptr::weak_ptr<Resource> weak2(shared2);
    
    weak1.swap(weak2);
    
    auto locked1 = weak1.lock();
    auto locked2 = weak2.lock();
    
    EXPECT_EQ(locked1->value(), 2);
    EXPECT_EQ(locked2->value(), 1);
}

TEST_F(WeakPointerTests, CopyAssignment) {
    sptr::shared_ptr<Resource> shared(new Resource(123));
    sptr::weak_ptr<Resource> weak1(shared);
    sptr::weak_ptr<Resource> weak2;
    
    weak2 = weak1;
    
    EXPECT_FALSE(weak1.expired());
    EXPECT_FALSE(weak2.expired());
    EXPECT_EQ(weak1.use_count(), 1);
    EXPECT_EQ(weak2.use_count(), 1);
    
    auto locked1 = weak1.lock();
    auto locked2 = weak2.lock();
    
    EXPECT_EQ(locked1.get(), locked2.get());
}

TEST_F(WeakPointerTests, MoveAssignment) {
    sptr::shared_ptr<Resource> shared(new Resource(123));
    sptr::weak_ptr<Resource> weak1(shared);
    sptr::weak_ptr<Resource> weak2;
    
    weak2 = std::move(weak1);
    
    EXPECT_TRUE(weak1.expired());
    EXPECT_FALSE(weak2.expired());
    EXPECT_EQ(weak1.use_count(), 0);
    EXPECT_EQ(weak2.use_count(), 1);
    
    auto locked = weak2.lock();
    EXPECT_TRUE(locked);
    EXPECT_EQ(locked->value(), 123);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}