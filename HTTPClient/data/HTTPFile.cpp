#include "HTTPFile.h"

//#define DEBUG "HTfi"
#include <cstdio>
#if (defined(DEBUG) && !defined(TARGET_LPC11U24))
#define DBG(x, ...)  std::printf("[DBG %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#define WARN(x, ...) std::printf("[WRN %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#define ERR(x, ...)  std::printf("[ERR %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#define INFO(x, ...) std::printf("[INF %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#else
#define DBG(x, ...)
#define WARN(x, ...)
#define ERR(x, ...)
#define INFO(x, ...)
#endif

HTTPFile::HTTPFile(char* filename) {
    INFO("HTTPFile %s", filename);
    file = fopen(filename, "w");   
    m_chunked = false; 
}

void HTTPFile::close() {
    INFO("close()");
    if (file) {
        fclose(file);
        file = NULL;
    }        
}

void HTTPFile::writeReset() {
    INFO("writeReset()");
    if (file) {
        rewind(file);   
    }
}

int HTTPFile::write(const char* buf, size_t len) {
    size_t written;
    INFO("write(%d,%s) m_len(%d), chunk %d", len, buf, m_len, m_chunked);
    if (file) {
        written = fwrite(buf, 1, len, file);   
        INFO("  writ:%d, ftell: %d", written, ftell(file)); 
        if ((!m_chunked && (ftell(file) >= m_len)) || (m_chunked && !written)) {
            INFO("closing");
            close();
        }
    }
    return len;    
}

void HTTPFile::setDataType(const char* type) {
    INFO("setDataType(%s)", type);
}

//void HTTPFile::setLocation(const char * location) {
//
//}

void HTTPFile::setIsChunked(bool chunked) {
    INFO("setIsChunked(%d)", chunked);
    m_chunked = chunked;
}

void HTTPFile::setDataLen(size_t len) {
    INFO("setDataLen(%d)", len);
    m_len = len;
}
