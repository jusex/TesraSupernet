



#include <list>
#include <locale>
#include <stdexcept>

namespace
{



bool sanity_test_widen(char testchar)
{
    const std::ctype<char>& test(std::use_facet<std::ctype<char> >(std::locale()));
    return test.narrow(test.widen(testchar), 'b') == testchar;
}





bool sanity_test_list(unsigned int size)
{
    std::list<unsigned int> test;
    for (unsigned int i = 0; i != size; ++i)
        test.push_back(i + 1);

    if (test.size() != size)
        return false;

    while (!test.empty()) {
        if (test.back() != test.size())
            return false;
        test.pop_back();
    }
    return true;
}

} 




bool sanity_test_range_fmt()
{
    std::string test;
    try {
        test.at(1);
    } catch (const std::out_of_range&) {
        return true;
    } catch (...) {
    }
    return false;
}

bool glibcxx_sanity_test()
{
    return sanity_test_widen('a') && sanity_test_list(100) && sanity_test_range_fmt();
}
