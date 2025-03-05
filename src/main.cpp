#include <iostream>
#include <string>

#include "unique_ptr.hpp"
#include "shared_ptr.hpp"
#include "weak_ptr.hpp"

class Person {
public:
    Person(std::string name) : m_name(std::move(name)) {
        std::cout << "Person created: " << m_name << std::endl;
    }
    
    ~Person() {
        std::cout << "Person destroyed: " << m_name << std::endl;
    }
    
    void greet() const {
        std::cout << "Hello, my name is " << m_name << std::endl;
    }
    
private:
    std::string m_name;
};

void demonstrate_unique_ptr() {
    std::cout << "\n=== unique_ptr demonstration ===\n" << std::endl;
    
    // Create a unique_ptr
    sptr::unique_ptr<Person> p1(new Person("Alice"));
    p1->greet();
    
    // Create using make_unique
    auto p2 = sptr::make_unique<Person>("Bob");
    p2->greet();
    
    // Move ownership
    sptr::unique_ptr<Person> p3 = std::move(p2);
    
    // p2 is now nullptr
    std::cout << "Is p2 null? " << (p2.get() == nullptr ? "Yes" : "No") << std::endl;
    
    // p3 has ownership
    p3->greet();
    
    std::cout << "\nLeaving scope, Person objects will be destroyed\n" << std::endl;
}

void demonstrate_shared_ptr() {
    std::cout << "\n=== shared_ptr demonstration ===\n" << std::endl;
    
    // Create a shared_ptr
    sptr::shared_ptr<Person> s1(new Person("Charlie"));
    std::cout << "Reference count: " << s1.use_count() << std::endl;
    
    // Create a shared_ptr sharing ownership
    {
        sptr::shared_ptr<Person> s2 = s1;
        std::cout << "Reference count: " << s1.use_count() << std::endl;
        s2->greet();
        
        // Create another shared_ptr
        sptr::shared_ptr<Person> s3 = s1;
        std::cout << "Reference count: " << s1.use_count() << std::endl;
        
        std::cout << "s2 and s3 going out of scope..." << std::endl;
    }
    
    std::cout << "Reference count: " << s1.use_count() << std::endl;
    s1->greet();
    
    std::cout << "\nLeaving scope, Charlie will be destroyed\n" << std::endl;
}

void demonstrate_weak_ptr() {
    std::cout << "\n=== weak_ptr demonstration ===\n" << std::endl;
    
    // Create a weak_ptr from a shared_ptr
    sptr::weak_ptr<Person> w1;
    
    {
        std::cout << "Creating shared_ptr..." << std::endl;
        sptr::shared_ptr<Person> s1(new Person("Dave"));
        w1 = s1;
        
        std::cout << "Is w1 expired? " << (w1.expired() ? "Yes" : "No") << std::endl;
        std::cout << "Reference count: " << w1.use_count() << std::endl;
        
        // Lock the weak_ptr to get a shared_ptr
        auto s2 = w1.lock();
        if (s2) {
            s2->greet();
            std::cout << "Reference count: " << w1.use_count() << std::endl;
        }
        
        std::cout << "Shared_ptr going out of scope..." << std::endl;
    }
    
    // The shared_ptr is gone, but we still have a weak_ptr
    std::cout << "Is w1 expired? " << (w1.expired() ? "Yes" : "No") << std::endl;
    std::cout << "Reference count: " << w1.use_count() << std::endl;
    
    // Try to lock it
    auto s3 = w1.lock();
    if (s3) {
        s3->greet();
    } else {
        std::cout << "Could not lock weak_ptr, the resource is gone" << std::endl;
    }
    
    std::cout << "\nLeaving scope, no more resources to destroy\n" << std::endl;
}

int main() {
    demonstrate_unique_ptr();
    demonstrate_shared_ptr();
    demonstrate_weak_ptr();
    
    return 0;
}