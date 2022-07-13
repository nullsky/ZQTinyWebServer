//
// Created by jackqzhou on 2022/7/9.
//

#ifndef ZQTINYWEBSERVER_HTTPCONN_H
#define ZQTINYWEBSERVER_HTTPCONN_H

#include <netinet/in.h>
#include <string>
#include <mysql.h>
#include <sys/stat.h>
#include <map>

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

    sockaddr_in *get_address() {
        return &address_;
    };
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

    bool add_headers(int content_length);
    bool add_status_line(int status, const char *title);
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_content_type();
    bool add_linger();
    bool add_blank_line();

public:
    static int epollfd_;
    static int user_count_;
    MYSQL *mysql_;
    int state_; // 0:读 1:写

private:
    int sockfd_;
    sockaddr_in address_;
    char read_buf_[READ_BUF_SIZE];
    int read_idx_;
    int check_idx_;
    int start_line_;
    char write_buf_[WRITE_BUF_SZIE];
    int wirte_idx_;
    CHECK_STATE check_state_;
    METHOD method_;

    char real_file_[FILE_NAME_LENGTH];
    char *url_;
    char *version_;
    char *host_;
    int content_length_;
    bool linger_;

    char *file_address_;
    struct stat file_stat_; // 文件属性信息
    struct iovec iv_[2];    // 向量元素
    int iv_count_;
    int cgi_; // 是否启用post
    char *request_header_;
    int byte_to_send_;
    char *doc_root_;

    std::map<std::string, std::string> users_;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];

};


#endif //ZQTINYWEBSERVER_HTTPCONN_H
