#include <iostream>

#include "EventLoop.hpp"
#include "HttpClient.hpp"
#include "Timer.hpp"
#include <unistd.h>
#include <math.h>


class MyClient : public Kite::HttpClient
{
public:
    MyClient(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::HttpClient(ev)
        , timer(ev)
    {
        d_content_length = 0;
        d_progress = 0;
        setCaFile("/etc/ssl/cert.pem");
    }

    void get(const std::string &url)
    {
        timer.reset();
        Kite::HttpClient::get(url);
    }

protected:

    Kite::Timer timer;


    virtual void onHeadersReady(const std::map<std::string,std::string> &responseHeaders)
    {
        for(auto const &h: responseHeaders) {
            if (h.first == "Content-Length") {
                d_content_length = std::stoi(h.second);
            }
        }
    }

    virtual void onReadActivated() override
    {
        char buf[4048];
        int len = read(buf, 4048);
        if (len < 1) return;

        d_progress += len;

        char speedSign = 'K';
        char progressSign = 'K';
        char totalSign = 'K';

        double speeds = floor(((double)d_progress / 1024.0) / ((double)timer.elapsed() / 1000.0));
        if (speeds >= 1024) {
            speedSign = 'M';
            speeds/=1024.0;
        }

        double displayProgress = d_progress / 1024.0;

        if (displayProgress >= 1024) {
            progressSign = 'M';
            displayProgress/=1024.0;
        }

        int pc = 0;
        double  total = 0;
        if (d_content_length > 0) {
            pc = (int)floor(((double)d_progress/(double)d_content_length) * 100);
            total = d_content_length / 1024.0;
            if (total >= 1024) {
                totalSign = 'M';
                total /= 1024.0;
            }
            if (total >= 1024) {
                totalSign = 'G';
                total /= 1024.0;
            }
        }
        fprintf(stderr, "> downloading %d%% (%.1f %cb of %.1f %cb at %.1f %cb/s)                    \r",
                pc, displayProgress, progressSign, total, totalSign, speeds, speedSign);

        //::write(fileno(stdout), buf, len);
    }
    virtual void onFinished(Kite::HttpClient::Status status, int exitCode, const std::string& body) override
    {
        ev()->exit(0);
        fprintf(stderr, "\n> done. exitCode: %d, errorMessage: %s\n", exitCode, errorMessage().c_str());
    }

private:
    int d_content_length;
    int d_progress;
};

int main(int argc, char **argv)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<MyClient>        client(new MyClient(ev));

    client->get(argv[1]);
    return ev->exec();
}
