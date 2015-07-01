#ifndef HTTPFILE_H
#define HTTPFILE_H
#if 1
#include <mbed.h>
#include "../IHTTPData.h"


class HTTPFile : public IHTTPDataIn {
    
    public:
        HTTPFile(char* filename);
        
        /** Closes the file, should be called once the http connection is closed.
         */
        void close();
        
    protected:     
       
        friend class HTTPClient;
    
        /** Reset stream to its beginning 
        * Called by the HTTPClient on each new request
        */
        virtual void writeReset();
        
        /** Write a piece of data transmitted by the server
        * @param[in] buf Pointer to the buffer from which to copy the data
        * @param[in] len Length of the buffer
        * @returns number of bytes written.
        */
        virtual int write(const char* buf, size_t len);
        
        /** Set MIME type
        * @param[in] type Internet media type from Content-Type header
        */
        virtual void setDataType(const char* type);
        
        /** Determine whether the data is chunked
        *  Recovered from Transfer-Encoding header
        * @param[in] chunked indicates the transfer is chunked.
        */
        virtual void setIsChunked(bool chunked);
        
        /** If the data is not chunked, set its size
        * From Content-Length header
        * @param[in] len defines the size of the non-chunked transfer.
        */
        virtual void setDataLen(size_t len);
    private:
        FILE *file;
        size_t m_len;
        bool m_chunked;
};
#endif
#endif  // HTTPFILE_H