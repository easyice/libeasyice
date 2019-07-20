/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "creadline.h"
#include "hls_define.h"
#include <assert.h>

#include <string.h>
#include <stdlib.h>



CReadLine::CReadLine()
{

}

CReadLine::~CReadLine()
{

}

bool CReadLine::SetString(const string& str)
{
        if (str.empty())
        {
                return false;
        }
        m_str = str;
        
        m_pRead = NULL;
        m_pBegin = (uint8_t*)m_str.c_str();
        m_pEnd = m_pBegin + m_str.length();
        
        return true;
}

string CReadLine::ReadLine()
{
        if (m_pBegin >= m_pEnd)
        {
                return "";
        }
        char *line = ReadLine(m_pBegin, &m_pRead, m_pEnd - m_pBegin);
        if (line == NULL)
        {
                return "";
        }
        m_pBegin = m_pRead;
        
        string str = line;
        free(line);
        return str;
}

char *CReadLine::ReadLine(uint8_t *buffer, uint8_t **pos, const size_t len)
{
    assert(buffer);

    char *line = NULL;
    uint8_t *begin = buffer;
    uint8_t *p = begin;
    uint8_t *end = p + len;

    while (p < end)
    {
        if ((*p == '\r') || (*p == '\n') || (*p == '\0'))
            break;
        p++;
    }

    /* copy line excluding \r \n or \0 */
    line = strndup((char *)begin, p - begin);

    while ((*p == '\r') || (*p == '\n') || (*p == '\0'))
    {
        if (*p == '\0')
        {
            *pos = end;
            break;
        }
        else
        {
            /* next pass start after \r and \n */
            p++;
            *pos = p;
        }
    }

    return line;
}
