#include <iostream>
#include <typeinfo> // for typeid

int main() {
    int num = 10;
    double pi = 3.14;
    char letter = 'A';
    bool isTrue = true;
    float value = 2.5;

    


    std::cout << "Type of num: " << typeid(num).name() << std::endl;
    std::cout << "Type of pi: " << typeid(pi).name() << std::endl;
    std::cout << "Type of letter: " << typeid(letter).name() << std::endl;
    std::cout << "Type of isTrue: " << typeid(isTrue).name() << std::endl;
    std::cout << "Type of value: " << typeid(value).name() << std::endl;

    std::cout << "Type of value: " << typeid(value * pi).name() << std::endl;


    return 0;
}
