#ifndef TCP_ENCODER_H
#define TCP_ENCODER_H

#include <stdint.h>
#include <string.h>

// Network byte order macros
#define HTONS(x) ((uint16_t)((((x) & 0xFF) << 8) | (((x) >> 8) & 0xFF)))
#define HTONL(x) ((uint32_t)((((x) & 0xFF000000) >> 24) | \
                             (((x) & 0x00FF0000) >> 8)  | \
                             (((x) & 0x0000FF00) << 8)  | \
                             (((x) & 0x000000FF) << 24)))

// Protocol definitions
#define IP_VERSION_4    4
#define IP_HDR_LEN      20
#define TCP_HDR_LEN     20
#define IP_PROTO_TCP    6
#define IP_TTL_DEFAULT  64

// TCP flags
#define TCP_FIN  0x01
#define TCP_SYN  0x02
#define TCP_RST  0x04
#define TCP_PSH  0x08
#define TCP_ACK  0x10
#define TCP_URG  0x20

// Packed structures for network headers
typedef struct __attribute__((packed)) {
    uint8_t  version_ihl;      // Version (4 bits) + IHL (4 bits)
    uint8_t  tos;              // Type of Service
    uint16_t total_length;     // Total Length
    uint16_t identification;   // Identification
    uint16_t flags_fragment;   // Flags (3 bits) + Fragment Offset (13 bits)
    uint8_t  ttl;              // Time to Live
    uint8_t  protocol;         // Protocol
    uint16_t header_checksum;  // Header Checksum
    uint32_t src_addr;         // Source Address
    uint32_t dst_addr;         // Destination Address
} ip_header_t;

typedef struct __attribute__((packed)) {
    uint16_t src_port;         // Source Port
    uint16_t dst_port;         // Destination Port
    uint32_t seq_num;          // Sequence Number
    uint32_t ack_num;          // Acknowledgment Number
    uint8_t  data_offset_rsvd; // Data Offset (4 bits) + Reserved (3 bits) + NS (1 bit)
    uint8_t  flags;            // CWR, ECE, URG, ACK, PSH, RST, SYN, FIN
    uint16_t window_size;      // Window Size
    uint16_t checksum;         // Checksum
    uint16_t urgent_ptr;       // Urgent Pointer
} tcp_header_t;

// Pseudo header for TCP checksum calculation
typedef struct __attribute__((packed)) {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t  zero;
    uint8_t  protocol;
    uint16_t tcp_length;
} tcp_pseudo_header_t;

// TCP packet structure
typedef struct {
    ip_header_t  ip;
    tcp_header_t tcp;
    uint8_t      data[];
} tcp_packet_t;

// Connection state
typedef struct {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint16_t window_size;
    uint16_t mss;
} tcp_connection_t;

// Function prototypes
uint16_t calculate_checksum(const void* data, size_t length);
uint16_t calculate_tcp_checksum(const tcp_header_t* tcp_hdr, 
                               const tcp_pseudo_header_t* pseudo_hdr,
                               const void* data, size_t data_len);

size_t encode_ip_header(uint8_t* buffer, uint32_t src_ip, uint32_t dst_ip,
                       uint16_t total_len, uint16_t id);

size_t encode_tcp_header(uint8_t* buffer, const tcp_connection_t* conn,
                        uint8_t flags, const void* data, size_t data_len);

size_t encode_tcp_packet(uint8_t* buffer, size_t buffer_size,
                        const tcp_connection_t* conn, uint8_t flags,
                        const void* data, size_t data_len);

// Utility functions
uint32_t ip_str_to_addr(const char* ip_str);
void print_packet_hex(const uint8_t* packet, size_t length);

// Implementation

uint16_t calculate_checksum(const void* data, size_t length) {
    const uint16_t* ptr = (const uint16_t*)data;
    uint32_t sum = 0;
    
    // Sum all 16-bit words
    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }
    
    // Add odd byte if present
    if (length > 0) {
        sum += *(const uint8_t*)ptr << 8;
    }
    
    // Add carry bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

uint16_t calculate_tcp_checksum(const tcp_header_t* tcp_hdr,
                               const tcp_pseudo_header_t* pseudo_hdr,
                               const void* data, size_t data_len) {
    uint32_t sum = 0;
    const uint16_t* ptr;
    size_t len;
    
    // Checksum pseudo header
    ptr = (const uint16_t*)pseudo_hdr;
    for (int i = 0; i < sizeof(tcp_pseudo_header_t) / 2; i++) {
        sum += ptr[i];
    }
    
    // Checksum TCP header (excluding checksum field)
    ptr = (const uint16_t*)tcp_hdr;
    len = sizeof(tcp_header_t);
    while (len > 1) {
        if (ptr == &tcp_hdr->checksum) {
            ptr++; // Skip checksum field
            len -= 2;
            continue;
        }
        sum += *ptr++;
        len -= 2;
    }
    
    // Checksum data
    if (data && data_len > 0) {
        ptr = (const uint16_t*)data;
        len = data_len;
        while (len > 1) {
            sum += *ptr++;
            len -= 2;
        }
        if (len > 0) {
            sum += *(const uint8_t*)ptr << 8;
        }
    }
    
    // Add carry bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

size_t encode_ip_header(uint8_t* buffer, uint32_t src_ip, uint32_t dst_ip,
                       uint16_t total_len, uint16_t id) {
    ip_header_t* ip = (ip_header_t*)buffer;
    
    memset(ip, 0, sizeof(ip_header_t));
    
    ip->version_ihl = (IP_VERSION_4 << 4) | (IP_HDR_LEN / 4);
    ip->tos = 0;
    ip->total_length = HTONS(total_len);
    ip->identification = HTONS(id);
    ip->flags_fragment = HTONS(0x4000); // Don't fragment
    ip->ttl = IP_TTL_DEFAULT;
    ip->protocol = IP_PROTO_TCP;
    ip->src_addr = HTONL(src_ip);
    ip->dst_addr = HTONL(dst_ip);
    
    // Calculate IP checksum
    ip->header_checksum = 0;
    ip->header_checksum = calculate_checksum(ip, IP_HDR_LEN);
    
    return IP_HDR_LEN;
}

size_t encode_tcp_header(uint8_t* buffer, const tcp_connection_t* conn,
                        uint8_t flags, const void* data, size_t data_len) {
    tcp_header_t* tcp = (tcp_header_t*)buffer;
    tcp_pseudo_header_t pseudo;
    
    memset(tcp, 0, sizeof(tcp_header_t));
    
    tcp->src_port = HTONS(conn->src_port);
    tcp->dst_port = HTONS(conn->dst_port);
    tcp->seq_num = HTONL(conn->seq_num);
    tcp->ack_num = HTONL(conn->ack_num);
    tcp->data_offset_rsvd = (TCP_HDR_LEN / 4) << 4; // Data offset in 32-bit words
    tcp->flags = flags;
    tcp->window_size = HTONS(conn->window_size);
    tcp->urgent_ptr = 0;
    
    // Setup pseudo header for checksum
    pseudo.src_addr = HTONL(conn->src_ip);
    pseudo.dst_addr = HTONL(conn->dst_ip);
    pseudo.zero = 0;
    pseudo.protocol = IP_PROTO_TCP;
    pseudo.tcp_length = HTONS(TCP_HDR_LEN + data_len);
    
    // Calculate TCP checksum
    tcp->checksum = 0;
    tcp->checksum = calculate_tcp_checksum(tcp, &pseudo, data, data_len);
    
    return TCP_HDR_LEN;
}

size_t encode_tcp_packet(uint8_t* buffer, size_t buffer_size,
                        const tcp_connection_t* conn, uint8_t flags,
                        const void* data, size_t data_len) {
    if (buffer_size < IP_HDR_LEN + TCP_HDR_LEN + data_len) {
        return 0; // Buffer too small
    }
    
    static uint16_t packet_id = 1;
    size_t total_len = IP_HDR_LEN + TCP_HDR_LEN + data_len;
    size_t offset = 0;
    
    // Encode IP header
    offset += encode_ip_header(buffer + offset, conn->src_ip, conn->dst_ip,
                              total_len, packet_id++);
    
    // Encode TCP header
    offset += encode_tcp_header(buffer + offset, conn, flags, data, data_len);
    
    // Copy data payload
    if (data && data_len > 0) {
        memcpy(buffer + offset, data, data_len);
        offset += data_len;
    }
    
    return offset;
}

uint32_t ip_str_to_addr(const char* ip_str) {
    uint32_t addr = 0;
    int a, b, c, d;
    
    // Simple parsing - assumes valid format
    sscanf(ip_str, "%d.%d.%d.%d", &a, &b, &c, &d);
    addr = (a << 24) | (b << 16) | (c << 8) | d;
    
    return addr;
}

void print_packet_hex(const uint8_t* packet, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (i % 16 == 0) printf("\n%04zx: ", i);
        printf("%02x ", packet[i]);
    }
    printf("\n");
}

// Example usage
void tcp_encoder_example(void) {
    uint8_t packet_buffer[1500];
    tcp_connection_t conn;
    const char* payload = "Hello, TCP World!";
    size_t packet_len;
    
    // Initialize connection
    conn.src_ip = ip_str_to_addr("192.168.1.100");
    conn.dst_ip = ip_str_to_addr("192.168.1.200");
    conn.src_port = 12345;
    conn.dst_port = 80;
    conn.seq_num = 1000;
    conn.ack_num = 2000;
    conn.window_size = 8192;
    conn.mss = 1460;
    
    // Create SYN packet
    packet_len = encode_tcp_packet(packet_buffer, sizeof(packet_buffer),
                                  &conn, TCP_SYN, NULL, 0);
    
    printf("SYN packet (%zu bytes):", packet_len);
    print_packet_hex(packet_buffer, packet_len);
    
    // Create data packet
    conn.seq_num = 1001;
    conn.ack_num = 2001;
    packet_len = encode_tcp_packet(packet_buffer, sizeof(packet_buffer),
                                  &conn, TCP_PSH | TCP_ACK, 
                                  payload, strlen(payload));
    
    printf("\nData packet (%zu bytes):", packet_len);
    print_packet_hex(packet_buffer, packet_len);
}

#endif // TCP_ENCODER_H
