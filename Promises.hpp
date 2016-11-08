#ifndef KITE_PROMISES_HPP_asd
#define KITE_PROMISES_HPP_asd

#include <iostream>

namespace Kite
{

template <class T>
class Promise
{
public:
    Promise();
    ~Promise();
    void run(std::weak_ptr<Kite::EventLoop> ev);
    void accept(const T&);
    void reject(const std::exception &e);
    void cancel();

    typedef std::function<void(T)> CompletedCallback;
    Promise &then(CompletedCallback completed);

    typedef std::function<void(const std::exception &e)> FailCallback;
    Promise &fail(FailCallback failed);

    typedef std::function<void()> FinalCallback;
    Promise &finally(FinalCallback fin);
private:
    class Private {
    public:
        ~Private() {
            if (!finished) {
                if (fail) {
                    fail(std::underflow_error("destroyed"));
                } else {
                    std::cerr << "[warning] Kite::Promise was destroyed prematurely, but no fail handler installed" << std::endl;
                }
                for (auto f : fin) {
                    f();
                }
            }
        }
        CompletedCallback then;
        FailCallback      fail;
        std::vector<FinalCallback>     fin;
        bool finished;
    };
    std::shared_ptr<Private> d;
};

template <class T>
Promise<T>::Promise()
{
    d.reset(new Private);
    d->finished = false;
}

template <class T>
Promise<T>::~Promise()
{
}

template <class T>
Promise<T> &Promise<T>::fail(FailCallback fail)
{
    d->fail = fail;
    return *this;
}

template <class T>
Promise<T> &Promise<T>::then(CompletedCallback then)
{
    d->then = then;
    return *this;
}

template <class T>
Promise<T> &Promise<T>::finally(FinalCallback finally)
{
    d->fin.push_back(finally);
    return *this;
}

template <class T>
void Promise<T>::cancel()
{
    //TODO
}

template <class T>
void Promise<T>::accept(const T &val)
{
    if (d->then) {
        d->then(val);
    }
    for (auto f : d->fin) {
        f();
    }
    d->finished = true;
}

template <class T>
void Promise<T>::reject(const std::exception &e)
{
    if (d->fail) {
        d->fail(e);
    } else {
        std::cerr << "[warning] Kite::Promise was rejected but no fail handler installed" << std::endl;
        std::cerr << e.what() << std::endl;
    }
    for (auto f : d->fin) {
        f();
    }
    d->finished = true;
}


template <class T>
class AllPromises : public Kite::Promise<std::vector<T>>
{
public:
    AllPromises &wait(Kite::Promise<T> promise);
    ~AllPromises();
private:
    std::vector<Kite::Promise<T>> dPromises;
    std::vector<T> dR;
};

template <class T>
AllPromises<T>::~AllPromises()
{
    auto copy = dPromises;
    dPromises.clear();
    for (auto promise : copy) {
        promise.cancel();
    }
}

template <class T>
AllPromises<T> &AllPromises<T>::wait(Kite::Promise<T> promise) {
    dPromises.push_back(promise);
    promise.then([this](T t){
        dR.push_back(t);
        if (dR.size() >= dPromises.size()) {
            AllPromises::accept(dR);
        }
    });
    promise.fail([this](const std::exception &e){
        auto copy = dPromises;
        dPromises.clear();
        for (auto promise : copy) {
            promise.cancel();
        }
        AllPromises::reject(e);
    });
    return *this;
};



};
#endif
