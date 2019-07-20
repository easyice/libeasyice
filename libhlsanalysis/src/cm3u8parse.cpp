/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "cm3u8parse.h"
#include "hls_type.h"
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "creadline.h"


char *strtok_r(char *s, const char *delim, char **save_ptr)
{
    char *token;

    if (s == NULL)
        s = *save_ptr;

    /* Scan leading delimiters. */
    s += strspn (s, delim);
    if (*s == '\0')
        return NULL;

    /* Find the end of the token. */
    token = s;
    s = strpbrk (token, delim);
    if (s == NULL)
        /* This token finishes the string. */
        *save_ptr = strchr (token, '\0');
    else
    {
        /* Terminate the token and make *SAVE_PTR point past it. */
        *s = '\0';
        *save_ptr = s + 1;
    }
    return token;
}

static int parse_SegmentInformation(hls_stream_t *hls, char *p_read, int *duration)
{
    assert(hls);
    assert(p_read);

    /* strip of #EXTINF: */
    char *p_next = NULL;
    char *token = strtok_r(p_read, ":", &p_next);
    if (token == NULL)
        return VLC_EGENERIC;

    /* read duration */
    token = strtok_r(NULL, ",", &p_next);
    if (token == NULL)
        return VLC_EGENERIC;

    int value;
    char *endptr;
    if (hls->version < 3)
    {
        errno = 0;
        value = strtol(token, &endptr, 10);
        if (token == endptr || errno == ERANGE)
        {
            *duration = -1;
            return VLC_EGENERIC;
        }
        *duration = value;
    }
    else
    {
        errno = 0;
        double d = strtof(token, &endptr);
        if (token == endptr || errno == ERANGE)
        {
            *duration = -1;
            return VLC_EGENERIC;
        }
        if ((d) - ((int)d) >= 0.5)
            value = ((int)d) + 1;
        else
            value = ((int)d);
        *duration = value;
    }

    /* Ignore the rest of the line */
    return VLC_SUCCESS;
}

static int parse_TargetDuration(hls_stream_t *hls,const char *p_read)
{
    assert(hls);

    int duration = -1;
    int ret = sscanf(p_read, "#EXT-X-TARGETDURATION:%d", &duration);
    if (ret != 1)
    {
        return VLC_EGENERIC;
    }

    hls->duration = duration; /* seconds */
    return VLC_SUCCESS;
}

/* Parsing */
static char *parse_Attributes(const char *line, const char *attr)
{
    char *p;
    char *begin = (char *) line;
    char *end = begin + strlen(line);

    /* Find start of attributes */
    if ((p = strchr(begin, ':' )) == NULL)
        return NULL;

    begin = p;
    do
    {
        if (strncasecmp(begin, attr, strlen(attr)) == 0
          && begin[strlen(attr)] == '=')
        {
            /* <attr>=<value>[,]* */
            p = strchr(begin, ',');
            begin += strlen(attr) + 1;
            if (begin >= end)
                return NULL;
            if (p == NULL) /* last attribute */
                return strndup(begin, end - begin);
            /* copy till ',' */
            return strndup(begin, p - begin);
        }
        begin++;
    } while(begin < end);

    return NULL;
}


static char *relative_URI(const char *psz_url, const char *psz_path)
{
    char *ret = NULL;
    assert(psz_url != NULL && psz_path != NULL);


    //If the path is actually an absolute URL, don't do anything.
    if (strncmp(psz_path, "http", 4) == 0)
        return NULL;

    size_t len = strlen(psz_path);

    char *new_url = strdup(psz_url);
    if (unlikely(!new_url))
        return NULL;

    if( psz_path[0] == '/' ) //Relative URL with absolute path
    {
        //Try to find separator for name and path, try to skip
        //access and first ://
        char *slash = strchr(&new_url[8], '/');
        if (unlikely(slash == NULL))
            goto end;
        *slash = '\0';
    } else {
        int levels = 0;
        while(len >= 3 && !strncmp(psz_path, "../", 3)) {
            psz_path += 3;
            len -= 3;
            levels++;
        }
        do {
            char *slash = strrchr(new_url, '/');
            if (unlikely(slash == NULL))
                goto end;
            *slash = '\0';
        } while (levels--);
    }

    if (asprintf(&ret, "%s/%s", new_url, psz_path) < 0)
        ret = NULL;

end:
    free(new_url);
    return ret;
}

static int parse_StreamInformation(const char* m3u8,const char *p_read, const char *uri,hls_stream_s& hls)
{
    int id;
    uint64_t bw;
    char *attr;

    //assert(*hls == NULL);

    attr = parse_Attributes(p_read, "PROGRAM-ID");
    if (attr)
    {
        id = atol(attr);
        free(attr);
    }
    else
        id = 0;

    attr = parse_Attributes(p_read, "BANDWIDTH");
    if (attr == NULL)
    {
        //msg_Err(s, "#EXT-X-STREAM-INF: expected BANDWIDTH=<value>");
        return VLC_EGENERIC;
    }
    bw = atoll(attr);
    free(attr);

    if (bw == 0)
    {
        //msg_Err(s, "#EXT-X-STREAM-INF: bandwidth cannot be 0");
        return VLC_EGENERIC;
    }

    //msg_Dbg(s, "bandwidth adaptation detected (program-id=%d, bandwidth=%"PRIu64").", id, bw);

    char *psz_uri = relative_URI(m3u8, uri);

    //*hls = hls_New(*hls_stream, id, bw, psz_uri ? psz_uri : uri);

    hls.id = id;
    hls.bandwidth = bw;
    hls.url = psz_uri ? psz_uri : uri;
    
    free(psz_uri);

    return /*(*hls == NULL) ? VLC_ENOMEM :*/ VLC_SUCCESS;
}

static int parse_Version(hls_stream_t *hls,const char *p_read)
{
    assert(hls);

    int version;
    int ret = sscanf(p_read, "#EXT-X-VERSION:%d", &version);
    if (ret != 1)
    {
        //msg_Err(s, "#EXT-X-VERSION: no protocol version found, should be version 1.");
        return VLC_EGENERIC;
    }

    /* Check version */
    hls->version = version;
    if (hls->version <= 0 || hls->version > 3)
    {
        //msg_Err(s, "#EXT-X-VERSION should be version 1, 2 or 3 iso %d", version);
        return VLC_EGENERIC;
    }
    return VLC_SUCCESS;
}



static int parse_AllowCache( hls_stream_t *hls,const char *p_read)
{
    assert(hls);

    char answer[4] = "\0";
    int ret = sscanf(p_read, "#EXT-X-ALLOW-CACHE:%3s", answer);
    if (ret != 1)
    {
        //msg_Err(s, "#EXT-X-ALLOW-CACHE, ignoring ...");
        return VLC_EGENERIC;
    }

    hls->b_cache = (strncmp(answer, "NO", 2) != 0);
    return VLC_SUCCESS;
}


static int string_to_IV(char *string_hexa, uint8_t iv[AES_BLOCK_SIZE])
{
    unsigned long long iv_hi, iv_lo;
    char *end = NULL;
    if (*string_hexa++ != '0')
        return VLC_EGENERIC;
    if (*string_hexa != 'x' && *string_hexa != 'X')
        return VLC_EGENERIC;

    string_hexa++;

    size_t len = strlen(string_hexa);
    if (len <= 16) {
        iv_hi = 0;
        iv_lo = strtoull(string_hexa, &end, 16);
        if (end)
            return VLC_EGENERIC;
    } else {
        iv_lo = strtoull(&string_hexa[len-16], NULL, 16);
        if (end)
            return VLC_EGENERIC;
        string_hexa[len-16] = '\0';
        iv_hi = strtoull(string_hexa, NULL, 16);
        if (end)
            return VLC_EGENERIC;
    }

    for (int i = 7; i >= 0 ; --i) {
        iv[  i] = iv_hi & 0xff;
        iv[8+i] = iv_lo & 0xff;
        iv_hi >>= 8;
        iv_lo >>= 8;
    }

    return VLC_SUCCESS;
}


static int parse_Key(stream_sys_t *sys, hls_stream_t *hls,const char *p_read)
{
    assert(hls);

    /* #EXT-X-KEY:METHOD=<method>[,URI="<URI>"][,IV=<IV>] */
    int err = VLC_SUCCESS;
    char *attr = parse_Attributes(p_read, "METHOD");
    if (attr == NULL)
    {
        //msg_Err(s, "#EXT-X-KEY: expected METHOD=<value>");
        return err;
    }

    if (strncasecmp(attr, "NONE", 4) == 0)
    {
        char *uri = parse_Attributes(p_read, "URI");
        if (uri != NULL)
        {
            //msg_Err(s, "#EXT-X-KEY: URI not expected");
            err = VLC_EGENERIC;
        }
        free(uri);
        /* IV is only supported in version 2 and above */
        if (hls->version >= 2)
        {
            char *iv = parse_Attributes(p_read, "IV");
            if (iv != NULL)
            {
                //msg_Err(s, "#EXT-X-KEY: IV not expected");
                err = VLC_EGENERIC;
            }
            free(iv);
        }
    }
    else if (strncasecmp(attr, "AES-128", 7) == 0)
    {
        char *value, *uri, *iv;
        if (sys->b_aesmsg == false)
        {
           // msg_Dbg(s, "playback of AES-128 encrypted HTTP Live media detected.");
            sys->b_aesmsg = true;
        }
        value = uri = parse_Attributes(p_read, "URI");
        if (value == NULL)
        {
            //msg_Err(s, "#EXT-X-KEY: URI not found for encrypted HTTP Live media in AES-128");
            free(attr);
            return VLC_EGENERIC;
        }

        /* Url is put between quotes, remove them */
        if (*value == '"')
        {
            /* We need to strip the "" from the attribute value */
            uri = value + 1;
            char* end = strchr(uri, '"');
            if (end != NULL)
                *end = 0;
        }
        /* For absolute URI, just duplicate it
         * don't limit to HTTP, maybe some sanity checking
         * should be done more in here? */
        if( strstr( uri , "://" ) )
        {
            hls->psz_current_key_path =  uri ;
        }
        else
        {
                char *p = relative_URI(hls->url.c_str(), uri);
            hls->psz_current_key_path = p;
            free(p);
        }
        free(value);

        value = iv = parse_Attributes(p_read, "IV");
        if (iv == NULL)
        {
            /*
            * If the EXT-X-KEY tag does not have the IV attribute, implementations
            * MUST use the sequence number of the media file as the IV when
            * encrypting or decrypting that media file.  The big-endian binary
            * representation of the sequence number SHALL be placed in a 16-octet
            * buffer and padded (on the left) with zeros.
            */
            hls->b_iv_loaded = false;
        }
        else
        {
            /*
            * If the EXT-X-KEY tag has the IV attribute, implementations MUST use
            * the attribute value as the IV when encrypting or decrypting with that
            * key.  The value MUST be interpreted as a 128-bit hexadecimal number
            * and MUST be prefixed with 0x or 0X.
            */

            if (string_to_IV(iv, hls->psz_AES_IV) == VLC_EGENERIC)
            {
                //msg_Err(s, "IV invalid");
                err = VLC_EGENERIC;
            }
            else
                hls->b_iv_loaded = true;
            free(value);
        }
    }
    else
    {
        //msg_Warn(s, "playback of encrypted HTTP Live media is not supported.");
        err = VLC_EGENERIC;
    }
    free(attr);
    return err;
}

CM3u8Parse::CM3u8Parse()
{

}

CM3u8Parse::~CM3u8Parse()
{

}

// bool CM3u8Parse::Parse(stream_sys_t& sys,hls_stream_t* init_hls,const string& m3u8_content)
// {
//         if (!isHTTPLiveStreaming(m3u8_content))
//         {
//                 return false;
//         }
//         
//         /* Is it a meta index file ? */
//         sys.b_meta = (strstr((const char *)m3u8_content.c_str(), "#EXT-X-STREAM-INF") == NULL) ? false : true;
//         
//         if (sys.b_meta)
//         {
//                 return ParseMeta(sys,m3u8_content);
//         }
//         
// 
//         return ParseStream(sys,init_hls,m3u8_content);
// }

bool CM3u8Parse::ParseMeta(stream_sys_t& sys,const string& m3u8_content)
{
        CReadLine readline;
        readline.SetString(m3u8_content);
        //msg_Dbg(s, "Meta playlist");

        int err = VLC_SUCCESS;
        
        /* M3U8 Meta Index file */
        do {
                /* Next line */
                string line = readline.ReadLine();
                if (line.empty())
                {
                        break;
                }

                /* */
                if (strncmp(line.c_str(), "#EXT-X-STREAM-INF", 17) == 0)
                {
                        string uri = readline.ReadLine();
                        if (uri.empty())
                        {
                                return false;
                        }

                        if (uri[0] == '#')
                        {
                                //msg_Warn(s, "Skipping invalid stream-inf: %s", uri);
                                return false;
                        }

                        bool new_stream_added = false;
                        hls_stream_t hls;
                        err = parse_StreamInformation(sys.m3u8.c_str(), line.c_str(), uri.c_str(),hls);
                        if (err == VLC_SUCCESS)
                        {
                                m_lstStreams.push_back(hls);
                        }


                }

        } while (err == VLC_SUCCESS);

       // msg_Dbg(s, "%d streams loaded in Meta playlist", (int)stream_count);
        if (m_lstStreams.empty())
        {
                //msg_Err(s, "No playable streams found in Meta playlist");
                err = VLC_EGENERIC;
                return false;
        }


        return true;
}

bool CM3u8Parse::ParseStream(stream_sys_t& sys,hls_stream_t* init_hls,const string& m3u8_content)
{
        int err = VLC_SUCCESS;
        
        sys.b_live = (strstr((const char *)m3u8_content.c_str(), "#EXT-X-ENDLIST") == NULL) ? true : false;
        if (init_hls->url.empty())
        {
                init_hls->url = sys.m3u8;
        }
      
        hls_stream_t *hls = init_hls;

        CReadLine readline;
        readline.SetString(m3u8_content);
        
        /* */
        bool media_sequence_loaded = false;
        int segment_duration = -1;
        do
        {
                /* Next line */
                string line = readline.ReadLine();
                if (line.empty())
                {
                        break;
                }

                if (strncmp(line.c_str(), "#EXTINF", 7) == 0)
                {
                        char * s_line = new char[line.length()+1];
                        memset(s_line,0,line.length()+1);
                        memcpy(s_line,line.c_str(),line.length());
                        err = parse_SegmentInformation(hls, s_line, &segment_duration);
                        delete [] s_line;
                }
                else if (strncmp(line.c_str(), "#EXT-X-TARGETDURATION", 21) == 0)
                {
                        err = parse_TargetDuration(hls, line.c_str());
                }
                else if (strncmp(line.c_str(), "#EXT-X-MEDIA-SEQUENCE", 21) == 0)
                {
                        /* A Playlist file MUST NOT contain more than one EXT-X-MEDIA-SEQUENCE tag. */
                        /* We only care about first one */
                        if (!media_sequence_loaded)
                        {
                                //err = parse_MediaSequence(s, hls, line);
                                media_sequence_loaded = true;
                        }
                }
                else if (strncmp(line.c_str(), "#EXT-X-KEY", 10) == 0)
                {
                        err = parse_Key(&sys, hls, line.c_str());
                }
                else if (strncmp(line.c_str(), "#EXT-X-PROGRAM-DATE-TIME", 24) == 0)
                {
                        //err = parse_ProgramDateTime(s, hls, line);
                }
                else if (strncmp(line.c_str(), "#EXT-X-ALLOW-CACHE", 18) == 0)
                {
                        err = parse_AllowCache( hls, line.c_str());
                }
                else if (strncmp(line.c_str(), "#EXT-X-DISCONTINUITY", 20) == 0)
                {
                        //err = parse_Discontinuity(s, hls, line);
                }
                else if (strncmp(line.c_str(), "#EXT-X-VERSION", 14) == 0)
                {
                        err = parse_Version(hls, line.c_str());
                }
                else if (strncmp(line.c_str(), "#EXT-X-ENDLIST", 14) == 0)
                {
                        //err = parse_EndList(s, hls);
                }
                else if ((strncmp(line.c_str(), "#", 1) != 0) && (line[0] != '\0') )
                {
                        err = parse_AddSegment(hls, segment_duration, line.c_str());
                        segment_duration = -1; /* reset duration */
                }


        } while (err == VLC_SUCCESS);
        
        hls->segment_count = m_lstSegments.size();
        return err == VLC_SUCCESS;

}

int CM3u8Parse::parse_AddSegment(hls_stream_t *hls, const int duration, const char *uri)
{
        assert(hls);
        assert(uri);

        /* Store segment information */

        char *psz_uri = relative_URI(hls->url.c_str(), uri);

        segment_t segment;
        segment.duration = duration; /* seconds */
        segment.size = 0; /* bytes */
        segment.sequence = 0;
        segment.bandwidth = 0;
        segment.url = psz_uri ? psz_uri : uri;
        segment.psz_key_path = hls->psz_current_key_path;
        //memcpy(segment.psz_AES_IV,hls->psz_AES_IV,AES_BLOCK_SIZE);
        m_lstSegments.push_back(segment);

        segment.sequence = hls->sequence + m_lstSegments.size() - 1;
        free(psz_uri);


        return VLC_SUCCESS;
}


 bool CM3u8Parse::isHTTPLiveStreaming(const string& m3u8_content)
{
    const char *peek = m3u8_content.c_str();

    int size = m3u8_content.size();
    if (size < 7)
        return false;

    if (memcmp(peek, "#EXTM3U", 7) != 0)
        return false;

    peek += 7;
    size -= 7;

    /* Parse stream and search for
     * EXT-X-TARGETDURATION or EXT-X-STREAM-INF tag, see
     * http://tools.ietf.org/html/draft-pantos-http-live-streaming-04#page-8 */
    while (size--)
    {
        static const char *const ext[] = {
            "TARGETDURATION",
            "MEDIA-SEQUENCE",
            "KEY",
            "ALLOW-CACHE",
            "ENDLIST",
            "STREAM-INF",
            "DISCONTINUITY",
            "VERSION"
        };

        if (*peek++ != '#')
            continue;

        if (size < 6)
            continue;

        if (memcmp(peek, "EXT-X-", 6))
            continue;

        peek += 6;
        size -= 6;

        for (size_t i = 0; i < ARRAY_SIZE(ext); i++)
        {
            size_t len = strlen(ext[i]);
            if (size < 0 || (size_t)size < len)
                continue;
            if (!memcmp(peek, ext[i], len))
                return true;
        }
    }

    return false;
}

bool CM3u8Parse::isMeta(const string& m3u8_content)
{
        /* Is it a meta index file ? */
        return (strstr((const char *)m3u8_content.c_str(), "#EXT-X-STREAM-INF") == NULL) ? false : true;
        
}

