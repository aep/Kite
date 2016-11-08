#include <iostream>
#include "EventLoop.hpp"
#include "DomainNameResolver.hpp"

int main(int argc, char **argv)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    Kite::AllPromises<std::vector<Kite::DomainNameRecord>> all;
    all.wait(Kite::DomainNameResolver::resolve(ev, "swag.com"))
       .wait(Kite::DomainNameResolver::resolve(ev, "google.de"))
       .wait(Kite::DomainNameResolver::resolve(ev, "heise.de"))
       .then([ev](const std::vector<std::vector<Kite::DomainNameRecord>> &recordset) {
           for (auto records : recordset) {
              for (auto record : records) {
                  std::cerr << "\t" << "\t" << record.address.address() << std::endl;
              }
           }
       })
       .finally(std::bind(&Kite::EventLoop::exit, ev, 0));
    return ev->exec();
}
