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

// Test for circular references
class Node {
public:
    explicit Node(int val) : value(val) {}
    
    void connect(const sptr::shared_ptr<Node>& other) {
        next = other;
    }
    
    int value;
    sptr::shared_ptr<Node> next;
};

TEST_F(SharedPointerTests, CircularReference) {
    // Create two nodes with circular references
    EXPECT_EQ(Resource::destroyed, 0);
    
    {
        // Create nodes
        auto node1 = sptr::make_shared<Node>(1);
        auto node2 = sptr::make_shared<Node>(2);
        
        // Create circular reference
        node1->connect(node2);
        node2->connect(node1);
        
        EXPECT_EQ(node1.use_count(), 2); // Referenced by our variable and node2->next
        EXPECT_EQ(node2.use_count(), 2); // Referenced by our variable and node1->next
    }
    
    // Without proper management, the circular reference would prevent cleanup
    // This is where weak_ptr would be needed in real applications
}

// Test for shared_ptr casting functions
class Base {
public:
    virtual ~Base() {}
    virtual int get_value() const { return 1; }
};

class Derived : public Base {
public:
    int get_value() const override { return 2; }
};

TEST_F(SharedPointerTests, DynamicCast) {
    sptr::shared_ptr<Base> base = sptr::make_shared<Derived>();
    EXPECT_EQ(base->get_value(), 2);
    
    // Dynamic cast to derived class should succeed
    auto derived = sptr::dynamic_pointer_cast<Derived>(base);
    EXPECT_TRUE(derived);
    EXPECT_EQ(derived->get_value(), 2);
    
    // Dynamic cast to unrelated class should fail
    auto resource = sptr::dynamic_pointer_cast<Resource>(base);
    EXPECT_FALSE(resource);
}

TEST_F(SharedPointerTests, StaticCast) {
    sptr::shared_ptr<Derived> derived = sptr::make_shared<Derived>();
    
    // Static cast to base class
    sptr::shared_ptr<Base> base = sptr::static_pointer_cast<Base>(derived);
    EXPECT_TRUE(base);
    EXPECT_EQ(base->get_value(), 2);
    
    // Static cast back to derived class
    auto derived2 = sptr::static_pointer_cast<Derived>(base);
    EXPECT_TRUE(derived2);
    EXPECT_EQ(derived2->get_value(), 2);
}

TEST_F(SharedPointerTests, ConstCast) {
    sptr::shared_ptr<const Base> const_base = sptr::make_shared<Base>();
    
    // Const cast to remove const
    auto non_const = sptr::const_pointer_cast<Base>(const_base);
    EXPECT_TRUE(non_const);
    EXPECT_EQ(non_const->get_value(), 1);
    
    // Reference counts should be properly handled
    EXPECT_EQ(const_base.use_count(), 2);
    EXPECT_EQ(non_const.use_count(), 2);
}

// Test using weak_ptr to break circular references
class WeakNode {
public:
    explicit WeakNode(int val) : value(val) {}
    
    void connect(const sptr::shared_ptr<WeakNode>& other) {
        // Store a weak_ptr instead of a shared_ptr
        next_weak = other;
    }
    
    int value;
    sptr::weak_ptr<WeakNode> next_weak;
};

TEST_F(SharedPointerTests, WeakPtrBreakingCircularReference) {
    EXPECT_EQ(Resource::destroyed, 0);
    
    {
        // Create nodes
        auto node1 = sptr::make_shared<WeakNode>(1);
        auto node2 = sptr::make_shared<WeakNode>(2);
        
        // Create connections using weak pointers
        node1->connect(node2);
        node2->connect(node1);
        
        EXPECT_EQ(node1.use_count(), 1); // Only referenced by our variable
        EXPECT_EQ(node2.use_count(), 1); // Only referenced by our variable
        
        // We can still access the connected nodes through the weak_ptr
        auto locked1 = node1->next_weak.lock();
        EXPECT_TRUE(locked1);
        EXPECT_EQ(locked1->value, 2);
        
        auto locked2 = node2->next_weak.lock();
        EXPECT_TRUE(locked2);
        EXPECT_EQ(locked2->value, 1);
    }
    
    // Both nodes should be properly destroyed when shared_ptrs go out of scope
    // because weak_ptrs don't prevent destruction
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}