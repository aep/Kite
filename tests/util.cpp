#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "dessert/dessert.hpp"
#include "Util.hpp"

desserts("ends_width") {
    dessert (Kite::ends_with("the hoff", "hoff"));
    dessert (Kite::ends_with("the hoff", "ff"));
    dessert (!Kite::ends_with("the hoff", "fff"));
    dessert (!Kite::ends_with("the hoff", "the"));
    dessert (Kite::ends_with(
                "f*j??9M?v2mc?K?I?Z?F6???o?/Rs|???G]c3??y',?=?d=?n?GS?0u=W??@??",
                "??@??"));
    dessert (Kite::ends_with("a", "a"));
    dessert (Kite::ends_with("", ""));
    dessert (Kite::ends_with("a", ""));
    dessert (!Kite::ends_with("", "a"));
}


desserts("trim") {
    std::string t1 = " \t   \n\r\v\t derp\t\v\nbla\n ";
    Kite::ltrim(t1);
    dessert(t1 == "derp\t\v\nbla\n ");
    Kite::rtrim(t1);
    dessert(t1 == "derp\t\v\nbla");
}
desserts("trimmed") {
    std::string o = " \t   \n\r\v\t derp\t\v\nbla\n ";
    std::string t1 = o;
    dessert(Kite::ltrimmed(t1) == "derp\t\v\nbla\n ");
    dessert(t1 == o);
    dessert(Kite::ltrimmed(Kite::rtrimmed(t1)) == "derp\t\v\nbla");
    dessert(Kite::rtrimmed(Kite::ltrimmed(t1)) == "derp\t\v\nbla");
    dessert(Kite::trimmed(t1) == "derp\t\v\nbla");
    dessert(t1 == o);
}


int main(){}
