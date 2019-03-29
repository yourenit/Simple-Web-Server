#pragma once

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "locker.h"

class http_conn
{
public:
    http_conn() {}
    ~http_conn() {}

public:
    //文件名的最大长度
    static const int FILENAME_LEN = 200;
    //读缓冲区的大小
    static const int READ_BUFFER_SIZE = 2048;
    //写缓冲区的大小
    static const int WRITE_BUFFER_SIZE = 1024;
    //http请求方法，仅支持GET
    enum METHOD { GET = 0,POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH };
    //解析客户请求时，主状态机所处的状态
    //主状态机的两种可能状态：当前正在分析请求行，当前正在分析头部字段
    enum CHECK_CODE { CHECK_STATE_REQUESTLINE = 0,CHECK_STATE_HEADER,CHECK_STATE_CONTENT};
    //服务器处理http请求的可能结果
    //NO_REQUEST表示请求不完整，需要继续获取数据；
    //GET_REQUEST表示获得一个完整的客户请求；
    //BAD_REQUEST表示客户请求有语法错误；
    //FORBIDDEN_REQUEST表示客户对资源没有足够的访问权限；
    //INTERNAL_ERROR表示服务器内部错误；
    //CLOSED_CONNECTION表示客户端已经关闭连接。
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST,
                    INTERNAL_ERROR, CLOSED_CONNECTION};
    //从状态机的三种可能状态，即行的读取状态，分别表示、；读取到一个完整的行、行出错、和行数据尚且不完整
    enum LINK_CODE { LINK_OK = 0,LINK_BAD, LINK_OPEN };

public:
    //初始化新接受的连接
    void init(int sockfd,const sockaddr_in& addr);
    //关闭连接
    void close_conn(bool real_close = true);
    //处理客户需求
    void process();
    //非阻塞读操作
    bool read();
    //非阻塞写
    bool write();

private:
    //初始化连接
    void init();
    //解析http请求
    HTTP_CODE process_read();
    //填充http应答
    bool process_write(HTTP_CODE ret);

    //下列函数被process_read()调用用来分析http请求
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_content(char* text);
    HTTP_CODE do_request();
    char *get_line(){ return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();

    //下面函数用于process_write调用填充http应答
    void unmap();
    bool add_response(const char *format,...);
    bool add_content(const char *content);
    bool add_status_line(int status,const char* title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
private:

};

