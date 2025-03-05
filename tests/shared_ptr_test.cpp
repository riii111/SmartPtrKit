#include <gtest/gtest.h>
#include "shared_ptr.hpp"

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

class SharedPointerTests : public ::testing::Test {
protected:
    void SetUp() override {
        Resource::reset();
    }
};

TEST_F(SharedPointerTests, DefaultConstruction) {
    sptr::shared_ptr<Resource> ptr;
    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(ptr.use_count(), 0);
}

TEST_F(SharedPointerTests, PointerConstruction) {
    Resource* r = new Resource(123);
    sptr::shared_ptr<Resource> ptr(r);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.get(), r);
    EXPECT_EQ(ptr->value(), 123);
    EXPECT_EQ((*ptr).id(), 0);
    EXPECT_EQ(ptr.use_count(), 1);
}

TEST_F(SharedPointerTests, CopyConstruction) {
    sptr::shared_ptr<Resource> ptr1(new Resource(123));
    sptr::shared_ptr<Resource> ptr2(ptr1);
    
    EXPECT_TRUE(ptr1);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr1.get(), ptr2.get());
    EXPECT_EQ(ptr1.use_count(), 2);
    EXPECT_EQ(ptr2.use_count(), 2);
}

TEST_F(SharedPointerTests, MoveConstruction) {
    Resource* r = new Resource(123);
    sptr::shared_ptr<Resource> ptr1(r);
    sptr::shared_ptr<Resource> ptr2(std::move(ptr1));
    
    EXPECT_FALSE(ptr1);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_EQ(ptr2.get(), r);
    EXPECT_EQ(ptr1.use_count(), 0);
    EXPECT_EQ(ptr2.use_count(), 1);
}

TEST_F(SharedPointerTests, CopyAssignment) {
    sptr::shared_ptr<Resource> ptr1(new Resource(123));
    sptr::shared_ptr<Resource> ptr2;
    
    ptr2 = ptr1;
    
    EXPECT_TRUE(ptr1);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr1.get(), ptr2.get());
    EXPECT_EQ(ptr1.use_count(), 2);
    EXPECT_EQ(ptr2.use_count(), 2);
}

TEST_F(SharedPointerTests, MoveAssignment) {
    Resource* r = new Resource(123);
    sptr::shared_ptr<Resource> ptr1(r);
    sptr::shared_ptr<Resource> ptr2;
    
    ptr2 = std::move(ptr1);
    
    EXPECT_FALSE(ptr1);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_EQ(ptr2.get(), r);
    EXPECT_EQ(ptr1.use_count(), 0);
    EXPECT_EQ(ptr2.use_count(), 1);
}

TEST_F(SharedPointerTests, Reset) {
    Resource* r1 = new Resource(1);
    Resource* r2 = new Resource(2);
    
    sptr::shared_ptr<Resource> ptr(r1);
    EXPECT_EQ(Resource::destroyed, 0);
    
    ptr.reset(r2);
    EXPECT_EQ(Resource::destroyed, 1);
    EXPECT_EQ(ptr.get(), r2);
    EXPECT_EQ(ptr.use_count(), 1);
    
    ptr.reset();
    EXPECT_EQ(Resource::destroyed, 2);
    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr.use_count(), 0);
}

TEST_F(SharedPointerTests, Swap) {
    Resource* r1 = new Resource(1);
    Resource* r2 = new Resource(2);
    
    sptr::shared_ptr<Resource> ptr1(r1);
    sptr::shared_ptr<Resource> ptr2(r2);
    
    ptr1.swap(ptr2);
    
    EXPECT_EQ(ptr1.get(), r2);
    EXPECT_EQ(ptr2.get(), r1);
    EXPECT_EQ(ptr1.use_count(), 1);
    EXPECT_EQ(ptr2.use_count(), 1);
}

TEST_F(SharedPointerTests, Destructor) {
    EXPECT_EQ(Resource::destroyed, 0);
    {
        sptr::shared_ptr<Resource> ptr(new Resource());
        EXPECT_EQ(Resource::destroyed, 0);
    }
    EXPECT_EQ(Resource::destroyed, 1);
}

TEST_F(SharedPointerTests, MakeShared) {
    auto ptr = sptr::make_shared<Resource>(42);
    
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr->value(), 42);
    EXPECT_EQ(ptr.use_count(), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}