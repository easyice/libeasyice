/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef HLS_TYPE_H
#define HLS_TYPE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "json/json.h"



using namespace std;




typedef unsigned char BYTE;
typedef unsigned char uint8_t;


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


/*****************************************************************************
 * Error values (shouldn't be exposed)
 *****************************************************************************/
#define VLC_SUCCESS        (-0) /**< No error */
#define VLC_EGENERIC       (-1) /**< Unspecified error */
#define VLC_ENOMEM         (-2) /**< Not enough memory */
#define VLC_ETIMEOUT       (-3) /**< Timeout */
#define VLC_ENOMOD         (-4) /**< Module not found */
#define VLC_ENOOBJ         (-5) /**< Object not found */
#define VLC_ENOVAR         (-6) /**< Variable not found */
#define VLC_EBADVAR        (-7) /**< Bad variable value */
#define VLC_ENOITEM        (-8) /**< Item not found */


/* Branch prediction */
#ifdef __GNUC__
#   define likely(p)   __builtin_expect(!!(p), 1)
#   define unlikely(p) __builtin_expect(!!(p), 0)
#else
#   define likely(p)   (!!(p))
#   define unlikely(p) (!!(p))
#endif



#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp  _strnicmp 
#define strtoull _strtoui64
#define  strtoll  _strtoi64
#endif









/*****************************************************************************
 *
 *****************************************************************************/
#define AES_BLOCK_SIZE 16 /* Only support AES-128 */
typedef struct segment_s
{
        segment_s()
        {
                
        }
        segment_s(const segment_s &ss)
        {
                sequence        = ss.sequence;
                duration        = ss.duration;
                size            = ss.size;
                bandwidth       = ss.bandwidth;
                url             = ss.url;
                psz_key_path    = ss.psz_key_path;
                memcpy(aes_key,ss.aes_key,16);
                b_key_loaded    = ss.b_key_loaded;
        }
    int         sequence;   /* unique sequence number */
    int         duration;   /* segment duration (seconds) */
    uint64_t    size;       /* segment size in bytes */
    uint64_t    bandwidth;  /* bandwidth usage of segments (bits per second)*/

    string        url;
    string       psz_key_path;         /* url key path */
    uint8_t     aes_key[16];      /* AES-128 */
    bool        b_key_loaded;

    //add by elva
    //uint8_t      psz_AES_IV[AES_BLOCK_SIZE];    /* IV used when decypher the block */
} segment_t;



typedef struct stream_sys_s
{
        stream_sys_s()
        {
                b_live = false;
                b_error = false;
                b_meta = false;
                b_aesmsg = false;
        }
        string          m3u8;       /* M3U8 url */
        bool            b_live;     /* live stream? or vod? */
        bool            b_error;    /* parsing error */
        bool            b_meta;     /* meta playlist */
        bool            b_aesmsg;   /* only print one time that the media is encrypted */

}stream_sys_t;

typedef struct hls_stream_s
{
        hls_stream_s()
        {
                id = -1;
                version = -1;
                sequence = 0;
                duration = -1;
                bandwidth = 0;
                size = 0;
                segment_count = 0;
                
        }
        hls_stream_s(const hls_stream_s &hls)
        {
                id              = hls.id;
                version         = hls.version;
                sequence        = hls.sequence;
                duration        = hls.duration;
                bandwidth       = hls.bandwidth;
                size            = hls.size;
                url             = hls.url;
                b_cache         = hls.b_cache;
                psz_current_key_path = hls.psz_current_key_path;
                memcpy(psz_AES_IV,hls.psz_AES_IV,AES_BLOCK_SIZE);
                b_iv_loaded     = hls.b_iv_loaded;
                segment_count   = hls.segment_count;
        }
        
        int         id;         /* program id */
        int         version;    /* protocol version should be 1 */
        int         sequence;   /* media sequence number */
        int         duration;   /* maximum duration per segment (s) */
        uint64_t    bandwidth;  /* bandwidth usage of segments (bits per second)*/
        uint64_t    size;       /* stream length is calculated by taking the sum
                                foreach segment of (segment->duration * hls->bandwidth/8) */

        //vlc_array_t *segments;  /* list of segments */
        string        url;        /* uri to m3u8 */
        // vlc_mutex_t lock;
        bool        b_cache;    /* allow caching */

        string        psz_current_key_path;          /* URL path of the encrypted key */
        uint8_t      psz_AES_IV[AES_BLOCK_SIZE];    /* IV used when decypher the block */
        bool         b_iv_loaded;
        
        int          segment_count;
} hls_stream_t;



///////////////////////////////////////////////////////////////////////////

//
//static char *strdup (const char *str)
//{
//    size_t len = strlen (str) + 1;
//    char *res = (char*)malloc (len);
//    if (res)
//        memcpy (res, str, len);
//    return res;
//}





#ifndef va_copy 
# ifdef __va_copy 
# define va_copy(DEST,SRC) __va_copy((DEST),(SRC)) 
# else 
# define va_copy(DEST, SRC) memcpy((&DEST), (&SRC), sizeof(va_list)) 
# endif 
#endif  



typedef struct _HLS_REPORT_PARAM_DOWNLOADING_T
{
	_HLS_REPORT_PARAM_DOWNLOADING_T()
	{
		progress = 0;
		dlnow = 0;
		dltotal = 0;
	}
    const Json::Value to_json() const
    {
        Json::Value root;
        root["segment"] = segment;
        root["progress"] = progress;
        root["dlnow"] = dlnow;
        root["dltotal"] = dltotal;
        return root;
    }
	string segment;
	int progress;
	int dlnow;
	int dltotal;
}HLS_REPORT_PARAM_DOWNLOADING_T;

typedef struct _HLS_REPORT_PARAM_DOWNLOAD_HISTORY_T
{
	_HLS_REPORT_PARAM_DOWNLOAD_HISTORY_T()
	{
		used_time = 0;
		file_size = 0;
		duration = 0;
		download_speed = 0;
		status = true;
		server_port = 0;
	}
    const Json::Value to_json() const
    {
        Json::Value root;
        root["segment"] = segment;
        root["server_address"] = server_address;
        root["server_port"] = server_port;
        root["used_time"] = used_time;
        root["file_size"] = file_size;
        root["duration"] = duration;
        root["download_speed"] = download_speed;
        root["status"] = status;
        return root;
    }
	string segment;
	string server_address;
	int server_port;
	double used_time; //second
	int file_size;
	double duration;
	double download_speed; // bit/s
	bool status; //true if success
}HLS_REPORT_PARAM_DOWNLOAD_HISTORY_T;

typedef struct _HA_PARAM_DIAGNOSIS_MSG_T
{
    const Json::Value to_json() const
    {
        Json::Value root;
        root["level"] = level;
        root["layer"] = layer;
        root["description"] = description;
        return root;
    }

	string level;
	string layer;
	string description;
}HA_PARAM_DIAGNOSIS_MSG_T;


#endif

