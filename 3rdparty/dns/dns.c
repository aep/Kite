/* The absolute minimum DNS resolver
 *
 * - Only around 200LOC, single C file
 * - Gives you the socket so you can poll/select with timeout
 * - Does not allocate heap
 * - Only supports A records
 * - Does not read /etc/resolv.conf
 * - Does not cache
 *
 * (C) 2016 Arvid E. Picciani
 * This code may be modified and distributed under the terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DNS_ERROR_SOCKET_FAILED    1
#define DNS_ERROR_NAME_TOO_LONG    2
#define DNS_ERROR_SENDTO_FAILED    3
#define DNS_ERROR_RECVFROM_FAILED  4
#define DNS_ERROR_UDP_INJECTED     5
#define DNS_ERROR_ANSWER_TOO_LARGE 6
#define DNS_ERROR_NXDOMAIN         7
#define PAYLOAD_BUFFER_SIZE 512

typedef struct {
    uint16_t    id;           // query id
    uint16_t    flags;        // flags
    uint16_t    queries;      // number of queries
    uint16_t    answers;      // number of answers
    uint16_t    authorities;  // number of authority something
    uint16_t    additionals;  // some crap
    char        payload[PAYLOAD_BUFFER_SIZE]; // payload buffer
} dns_packet_t;

typedef struct {
    int ttl;
    int address;
} dns_record_t;

typedef struct {
    int id;
    int socket;
    dns_packet_t  packet;
    in_addr_t     dns_address;
    dns_record_t  records[16];
} dns_t;


static int dns_init(dns_t *dns, const char *dns_ip);
static int dns_request(dns_t *dns, const char *name);
static int dns_receive(dns_t *dns);
static int dns_set_timeout(dns_t *dns, int seconds);
static void dns_close(dns_t *dns);


static int dns_init(dns_t *dns, const char *dns_ip)
{
    memset(dns, 0, sizeof(dns_t));
    dns->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (dns->socket < 0) {
        return DNS_ERROR_SOCKET_FAILED;
    }
    dns->dns_address = inet_addr(dns_ip);
    return 0;
}

static void dns_close(dns_t *dns)
{
    close(dns->socket);
}

#if 0
static int dns_set_timeout(dns_t *dns, int seconds)
{
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    return setsockopt(dns->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
}
#endif

static int dns_request(dns_t *dns, const char *name)
{
    dns->id = htons(0x1337);

    memset(&dns->packet, 0, sizeof(dns->packet));
    dns->packet.id       = dns->id;
    dns->packet.flags    = htons(0x100); //request recursion
    dns->packet.queries  = htons(1);
    dns->packet.answers  = 0;
    dns->packet.authorities = 0;
    dns->packet.additionals = 0;

    if (strlen(name) > PAYLOAD_BUFFER_SIZE - 20) {
        return DNS_ERROR_NAME_TOO_LONG;
    }

    //encode labels
    char *datap = dns->packet.payload;
    uint8_t *label_length = (uint8_t *)datap++;
    for(const char *n = name; *n != 0; n++) {
        if (*n == '.') {
            label_length = (uint8_t *)datap++;
        } else {
            *label_length += 1;
            *datap++ = *n;
        }
    }
    *datap++ = 0;

    *datap++ = 0;    //16bit padding
    *datap++ = 0x01; //request A records
    *datap++ = 0;    //16bit padding
    *datap++ = 1;    //inet class



    int len = (sizeof(dns->packet) - PAYLOAD_BUFFER_SIZE) +  (datap - dns->packet.payload);

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(53);
    sa.sin_addr.s_addr = dns->dns_address;

    if (sendto(dns->socket, &dns->packet, len , 0 , (struct sockaddr *)&sa, sizeof(sa)) < len) {
        return DNS_ERROR_SENDTO_FAILED;
    }
    return 0;
}

static int dns_receive(dns_t *dns)
{
    memset(&dns->packet, 0, sizeof(dns->packet));

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    unsigned int slen = sizeof(sa);

    int len = recvfrom(dns->socket, &dns->packet, sizeof(dns->packet), 0,
                (struct sockaddr *) &sa, &slen);

    if (len < 1) {
        return DNS_ERROR_RECVFROM_FAILED;
    }

    if (sa.sin_addr.s_addr != dns->dns_address) {
        return DNS_ERROR_UDP_INJECTED;
    }
    if (dns->packet.id != dns->id) {
        return DNS_ERROR_UDP_INJECTED;
    }
    if (ntohs(dns->packet.queries) > 1) {
        return DNS_ERROR_UDP_INJECTED;
    }

    if (ntohs(dns->packet.answers) < 1) {
        return DNS_ERROR_NXDOMAIN;
    }

    char *datap = dns->packet.payload;
    //skip query sections
    for (int i = 0; i < ntohs(dns->packet.queries); i++) {
        for (; *datap != 0; datap++) {}
        datap += 5;
    }

    //parse answer sections
    int record_count = 0;
    for (int i = 0; i < ntohs(dns->packet.answers); i++) {
        //skip name. find a pointer or a zero..
        for (;; datap++) {
            if (*datap == 0)   { break; }
            if (*datap & 0xc0) { ++datap; break; }
        }
        ++datap;
        uint16_t recordType  = htons(*(uint16_t *)datap);
        datap += 2;
        uint16_t recordClass = htons(*(uint16_t *)datap);
        datap += 2;
        uint32_t ttl         = htonl(*(uint32_t *)datap);
        datap += 4;
        uint16_t rdlength    = htons(*(uint16_t *)datap);
        datap += 2;

        if (recordClass == 1 && recordType == 1 && rdlength == 4) {
            dns->records[record_count].ttl = ttl;
            dns->records[record_count].address = *(int32_t *)datap;
            if (++record_count >= 16) {
                return DNS_ERROR_ANSWER_TOO_LARGE;
            }
        }
        datap += rdlength;
    }
    return 0;
}

#ifdef DNS_TEST_MAIN
int main(int argc, char **argv)
{
    dns_t dns;
    if (dns_init(&dns, "8.8.8.8"))
        return 1;
    if (dns_request(&dns, argv[1]))
        return 2;

    //you can do nonblocking poll/select on dns.socket before calling receive
    dns_set_timeout(&dns, 10);

    if (dns_receive(&dns))
        return 3;
    for(dns_record_t *r = dns.records; r->ttl != 0; r++) {
        printf("%s\t%d\tIN\tA\t%hhu.%hhu.%hhu.%hhu\n",
                argv[1],
                r->ttl,
                (r->address >> 0 ) & 0xFF,
                (r->address >> 8 ) & 0xFF,
                (r->address >> 16) & 0xFF,
                (r->address >> 24) & 0xFF);

    }
    dns_close(&dns);
}
#endif
