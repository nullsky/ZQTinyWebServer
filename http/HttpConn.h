//
// Created by jackqzhou on 2022/7/9.
//

#ifndef ZQTINYWEBSERVER_HTTPCONN_H
#define ZQTINYWEBSERVER_HTTPCONN_H

#include <netinet/in.h>
#include <string>

class HttpConn {
public:
    static const int FILE_NAME_LENGTH = 300;
    static const int READ_BUF_SIZE = 2048;
    static const int WRITE_BUF_SZIE = 1024;
    enum METHOD {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEAD,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSE_CONNECTION
    };
    enum LINE_STATUS {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    HttpConn() {}
    ~HttpConn() {}

    void init(int sockfd, const sockaddr_in &addr, char *, int, int, std::string user, std::string password, std::string database_name);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();
    bool write();

    void init_mysql_result();
    int time_flag;
    int improv;

private:
    void init();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line();
    LINE_STATUS parse_line();
    void unmap();

    bool add_response()
};


#endif //ZQTINYWEBSERVER_HTTPCONN_H
