/*
 * Copyright (c) 2010 - 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "QComExtractorFactory"
#include <utils/Log.h>
#include <utils/String8.h>
#include <cutils/properties.h>

#include "MMParserExtractor.h"
#include <utils/RefBase.h>
#include <media/stagefright/MediaDefs.h>
#include <QCMediaDefs.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/foundation/AMessage.h>

namespace android {

//! DTS format can have sync marker within 64KB worth of data
#define MAX_FORMAT_HEADER_SIZE (64*1024)

#define FORMAT_HEADER_SIZE 1024
#define AAC_FORMAT_HEADER_SIZE 8202
#define MIN_HEADER_DATA_NEEDED 20
#define ID3V2_HEADER_SIZE 10
#define MAX_METADATA_SIZE ( 3*1024*1024)

FileSourceFileFormat convertMimetoFileFormat(const char* mime) {
    if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_OGG)) {
        return FILE_SOURCE_OGG;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AC3)) {
        return FILE_SOURCE_AC3;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) {
        return FILE_SOURCE_AAC;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC_ADTS)) {
        return FILE_SOURCE_AAC;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_DTS)) {
        return FILE_SOURCE_DTS;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_DTS_LBR)) {
        return FILE_SOURCE_DTS;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_EAC3)) {
        return FILE_SOURCE_AC3;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG)) {
        return FILE_SOURCE_MP3;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_NB)) {
        return FILE_SOURCE_AMR_NB;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_WB)) {
        return FILE_SOURCE_AMR_WB;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_FLAC)) {
        return FILE_SOURCE_FLAC;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AIFF)) {
        return FILE_SOURCE_AIFF;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_APE)) {
        return FILE_SOURCE_APE;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_DSF )) {
        return FILE_SOURCE_DSF;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_DFF )) {
        return FILE_SOURCE_DSDIFF;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WAV)) {
        return FILE_SOURCE_WAV;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_AVI)) {
        return FILE_SOURCE_AVI;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_ASF)) {
        return FILE_SOURCE_ASF;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCP)) {
        return FILE_SOURCE_QCP;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_3G2)) {
        return FILE_SOURCE_3G2;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG4)) {
        return FILE_SOURCE_MPEG4;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2TS)) {
        return FILE_SOURCE_MP2TS;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2PS)) {
        return FILE_SOURCE_MP2PS;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MATROSKA)) {
        return FILE_SOURCE_MKV;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCFLV)) {
        return FILE_SOURCE_FLV;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MOV)) {
        return FILE_SOURCE_MOV;
    } else {
        return FILE_SOURCE_UNKNOWN;
    }
}

bool isExclusiveFileFormat(const char* mime) {
    bool retVal = true;
    if(!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_3G2)      ||
       !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG4)    ||
       !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2TS)  ||
       !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2PS)  ||
       !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MATROSKA) ||
       !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCFLV)    ||
       !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG)         ||
       !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_OGG)      ||
       !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WAV)      ||
       !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MOV)) {
        return false;
    }
    return retVal;
}

extern "C" MediaExtractor* createExtractor(const sp<DataSource> &source, const char* mime) {
    char property_value[PROPERTY_VALUE_MAX] = {0};
    int parser_flags = 0;
    property_get("mm.enable.qcom_parser", property_value, "0");
    parser_flags = atoi(property_value);
    LOGV("Parser_Flags == %x",parser_flags);

    if (!parser_flags) {
        LOGI("QTI parser is not preferred");
        return NULL;
    }

    if ((!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_OGG)) && (PARSER_OGG & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AC3)) && (PARSER_AC3 & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) && (PARSER_AAC & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC_ADTS)) && (PARSER_AAC & parser_flags)||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_DTS)) && (PARSER_DTS & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_DTS_LBR)) && (PARSER_DTS & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_EAC3)) && (PARSER_AC3 & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG)) && (PARSER_MP3 & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_NB)) && (PARSER_AMR_NB & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_WB)) && (PARSER_AMR_WB & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_FLAC )) && (PARSER_FLAC & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AIFF )) && (PARSER_AIFF & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_APE )) && (PARSER_APE & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_DSF )) && (PARSER_DSF & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_DFF )) && (PARSER_DSDIFF & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WAV)) && (PARSER_WAV & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_AVI)) && (PARSER_AVI & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_ASF)) && (PARSER_ASF & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCP)) && (PARSER_QCP & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_3G2)) && (PARSER_3G2 & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG4)) && (PARSER_3GP & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2TS)) && (PARSER_MP2TS & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2PS)) && (PARSER_MP2PS & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MATROSKA)) && (PARSER_MKV & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MOV)) && (PARSER_MOV & parser_flags) ||
        (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCFLV) ) && (PARSER_FLV & parser_flags)) {

        // As the extractors are running in separate process, currently  framework isn't
        // communicating about the sniff failure to this layer.
        // We shouldn't end up creating QTI parser when sniff is failing.
        // Hence, sniff one more time before actually creating the extractor.
        // Currently this is done for the video fileformats as well some audio formats
        // which are common in stock android.

        if (!isExclusiveFileFormat(mime)) {
            FileSourceFileFormat formatToCheck = convertMimetoFileFormat(mime);
            uint32_t buffSize   = FORMAT_HEADER_SIZE;
            uint8_t* headerBuff = (uint8_t*)malloc(MAX_FORMAT_HEADER_SIZE);
            uint8_t* pTempBuf   = NULL;
            FileSourceStatus status = FILE_SOURCE_INVALID;

            //! Read 1024bytes worth of data in first attempt
            ssize_t retReadSize = source->readAt(0, headerBuff, FORMAT_HEADER_SIZE);
            //! Minimum 1024 bytes worth of data needs to be read
            if (retReadSize < (ssize_t)buffSize) {
                LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
                //! Free the buffer before exit
                if(headerBuff != NULL) {
                    free(headerBuff);
                    headerBuff = NULL;
                }
                return NULL;
            }
            uint32 nDataRead = (uint32)retReadSize;
            LOGV("amount of data read at start %lu", nDataRead);
            status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,&nDataRead);
            LOGV("Updated nDataRead value after format %d check is %lu",
                formatToCheck, nDataRead);
            if (headerBuff != NULL) {
                free(headerBuff);
                headerBuff = NULL;
            }
            if (status != FILE_SOURCE_SUCCESS) {
                return NULL;
            }
        }

        LOGV(" instantiate MMParserExtractor \n");
        return new MMParserExtractor(source, mime);
    }

    LOGI("QTI parser is not preferred for mime: %s", mime);
    return NULL;
}

typedef enum
{
    Sniff_OGG,
    Sniff_AVI,
    Sniff_WAV,
    Sniff_DSF,
    Sniff_DSDIFF,
    Sniff_AC3,
    Sniff_ASF,
    Sniff_AMRNB,
    Sniff_AMRWB,
    Sniff_MKAV,
    Sniff_MOV,
    Sniff_3GP,
    Sniff_QCP,
    Sniff_FLAC,
    Sniff_FLV,
    Sniff_MPEG2TS,
    Sniff_3G2,
    Sniff_MPEG2PS,
    Sniff_AIFF,
    Sniff_APE,
    Sniff_AAC,
    Sniff_MP3,
    Sniff_DTS,
    MAX_SINFF_COUNT
}Sniff_codes;

/* ======================================================================
FUNCTION
  GetID3Size

DESCRIPTION
  Return True if the file start with ID3 tag otherwise, returns FALSE;

INPUT/OUTPUT
  pucDataBuf[in]    : Data buffer to compare ID3 tag
  pulID3TagLen[in/out]: Return ID3 tag size if found any.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is ID3 tag found otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool GetID3Size(uint8* pucDataBuf, uint32* pulID3TagLen,
                                 uint32 ulBufLen)
{
  //Check if file has ID3Tag present before AAC/MP3/FLAC/APE file header.
  bool ret = false;
  uint8* pProcessBuff = pucDataBuf;
  if ((NULL != pProcessBuff) && (NULL != pulID3TagLen))
  {
    *pulID3TagLen = 0;
    //ID3V2_HEADER_SIZE is 10
    while (*pulID3TagLen + ID3V2_HEADER_SIZE < ulBufLen)
    {
      if( ( 0 == memcmp("ID3",pProcessBuff,strlen("ID3") ) ) )
      {
        uint32 ulID3TagSize = 0;
        uint32 ulIndex;
        // Find ID3 tag size: skip 6 byte = ID3(3byte)+ Ver(2byte)+ Flag (1byte)
        pProcessBuff +=6;
        for (ulIndex =0; ulIndex < 4; ulIndex++)
        {
          ulID3TagSize = (ulID3TagSize << 7) | (*(pProcessBuff)++ & 0x7F);
        }
        //! Skip ID3 tag size field and tag size worth of data.
        //! This is done to check for more ID3 tags at end of current tags.
        pProcessBuff += ulID3TagSize;
        *pulID3TagLen += ulID3TagSize + ID3V2_HEADER_SIZE;
        ret = true;
      }
      else
      {
        break;
      }
    }//! while(*pulID3TagLen + ucHdrSize < ulBufLen)
  }//if ((NULL != pProcessBuff) && (NULL != pulID3TagLen))
  return ret;
}


bool ExtendedSniff(const sp<DataSource> &source, String8 *mimeType,
                   float *confidence, sp<AMessage> *){

  LOGV("Sniff Start\n");
  bool retVal = false;

  FileSourceFileFormat formatToCheck;
  uint32_t buffSize   = FORMAT_HEADER_SIZE;
  uint8_t* headerBuff = (uint8_t*)malloc(MAX_FORMAT_HEADER_SIZE);
  uint8_t* pTempBuf   = NULL;
  FileSourceStatus status = FILE_SOURCE_INVALID;
  if(NULL == headerBuff) {
      LOGE("sniff fail: Malloc failed");
      return false;
  }

  //! Read 1024bytes worth of data in first attempt
  ssize_t retReadSize = source->readAt(0, headerBuff, FORMAT_HEADER_SIZE);
  //! Minimum 1024 bytes worth of data needs to be read
  if(retReadSize < (ssize_t)buffSize) {
      LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
      //! Free the buffer before exit
      if(headerBuff != NULL) {
          free(headerBuff);
          headerBuff = NULL;
      }
      return false;
  }
  uint32 nDataRead = (uint32)retReadSize;
  LOGV("amount of data read at start %lu", nDataRead);
  char property_value[PROPERTY_VALUE_MAX] = {0};
  int parser_flags = 0;
  property_get("mm.enable.qcom_parser", property_value, "0");
  parser_flags = atoi(property_value);
  LOGV("Parser_Flags == %x",parser_flags);
  for(int index=0; index<MAX_SINFF_COUNT; ++index)
  {
      bool foundMatching = false;
      switch(index)
      {
        case Sniff_AVI:
          if( PARSER_AVI & parser_flags ) {
            LOGV("SniffAVI ===>\n");
            formatToCheck = FILE_SOURCE_AVI;
            foundMatching = true;
          }
          break;

        case Sniff_AC3:
          if( PARSER_AC3 & parser_flags ) {
            LOGV("SniffAC3 ===>\n");
            formatToCheck = FILE_SOURCE_AC3;
            foundMatching = true;
          }
          break;

        case Sniff_ASF:
          if( PARSER_ASF & parser_flags ) {
            LOGV("SniffASF ===>\n");
            formatToCheck = FILE_SOURCE_ASF;
            foundMatching = true;
          }
          break;

        case Sniff_3GP:
          if( PARSER_3GP & parser_flags ) {
            LOGV("Sniff3GP ===>\n");
            formatToCheck = FILE_SOURCE_MPEG4;
            foundMatching = true;
          }
          break;

        case Sniff_MOV:
          if( PARSER_MOV & parser_flags ) {
            LOGV("SniffQT ===>\n");
            formatToCheck = FILE_SOURCE_MOV;
            foundMatching = true;
          }
          break;

        case Sniff_OGG:
          if( PARSER_OGG & parser_flags ) {
            LOGV("SniffOGG ===>\n");
            formatToCheck = FILE_SOURCE_OGG;
            foundMatching = true;
          }
          break;

        case Sniff_MKAV:
          if( PARSER_MKV & parser_flags ) {
            LOGV("SniffMKAV ===>\n");
            formatToCheck = FILE_SOURCE_MKV;
            foundMatching = true;
          }
          break;

        case Sniff_FLV:
          if( PARSER_FLV & parser_flags ) {
            LOGV("SniffFLV ===>\n");
            formatToCheck = FILE_SOURCE_FLV;
            foundMatching = true;
          }
          break;

        case Sniff_QCP:
          if( PARSER_QCP & parser_flags ) {
            LOGV("SniffQCP ===>\n");
            formatToCheck = FILE_SOURCE_QCP;
            foundMatching = true;
          }
          break;

        case Sniff_AMRNB:
          if( PARSER_AMR_NB & parser_flags ) {
            LOGV("SniffAMRNB ===>\n");
            formatToCheck = FILE_SOURCE_AMR_NB;
            foundMatching = true;
          }
          break;

        case Sniff_AMRWB:
          if( PARSER_AMR_WB & parser_flags ) {
            LOGV("SniffAMRWB ===>\n");
            formatToCheck = FILE_SOURCE_AMR_WB;
            foundMatching = true;
          }
          break;

        case Sniff_AAC:
          if( PARSER_AAC & parser_flags) {
            LOGV("SniffAAC ===>\n");
            formatToCheck = FILE_SOURCE_AAC;
            foundMatching = true;
          }
          break;

        case Sniff_DTS:
          if( PARSER_DTS & parser_flags ) {
            LOGV("SniffDTS ===>\n");
            formatToCheck = FILE_SOURCE_DTS;
            foundMatching = true;
          }
          break;

        case Sniff_DSF:
          if( PARSER_DSF & parser_flags ) {
            LOGV("SniffDSF ===>\n");
            formatToCheck = FILE_SOURCE_DSF;
            foundMatching = true;
          }
          break;

        case Sniff_DSDIFF:
          if( PARSER_DSDIFF & parser_flags ) {
            LOGV("SniffDSDIFF ===>\n");
            formatToCheck = FILE_SOURCE_DSDIFF;
            foundMatching = true;
          }
          break;

        case Sniff_WAV:
          if( PARSER_WAV & parser_flags ) {
            LOGV("SniffWAV ===>\n");
            formatToCheck = FILE_SOURCE_WAV;
            foundMatching = true;
          }
          break;

        case Sniff_MP3:
          if( PARSER_MP3 & parser_flags ) {
            LOGV("SniffMP3 ===>\n");
            formatToCheck = FILE_SOURCE_MP3;
            foundMatching = true;
          }
          break;

         case Sniff_MPEG2TS:
          // By Default, QCOM MP2TS Parser is used in QRD branch
          if( PARSER_MP2TS & parser_flags ) {
            LOGV("SniffMP2TS ===>\n");
            formatToCheck = FILE_SOURCE_MP2TS;
            foundMatching = true;
          }
          break;

        case Sniff_3G2:
          if( PARSER_3G2 & parser_flags ) {
            LOGV("Sniff3G2 ===>\n");
            formatToCheck = FILE_SOURCE_3G2;
            foundMatching = true;
          }
          break;

        case Sniff_MPEG2PS:
          if( PARSER_MP2PS & parser_flags ) {
            LOGV("SniffMPEG2PS ===>\n");
            formatToCheck = FILE_SOURCE_MP2PS;
            foundMatching = true;
          }
          break;
#if defined(FLAC_OFFLOAD_ENABLED) || defined(QTI_FLAC_DECODER)
        case Sniff_FLAC:
          if( PARSER_FLAC & parser_flags ) {
            LOGV("SniffFLAC ===>\n");
            formatToCheck = FILE_SOURCE_FLAC;
            foundMatching = true;
          }
          break;
#endif
         case Sniff_AIFF:
          if( PARSER_AIFF & parser_flags )
          {
            LOGV("SniffAIFF ===>\n");
            formatToCheck = FILE_SOURCE_AIFF;
            foundMatching = true;
          }
          break;

         case Sniff_APE:
          if( PARSER_APE & parser_flags )
          {
            LOGV("SniffAPE ===>\n");
            formatToCheck = FILE_SOURCE_APE;
            foundMatching = true;
          }
          break;

        default:
          LOGW("Not matched with any sniff above, lets try other sniffs ");
          foundMatching = false;
          break;
     } //! switch(index)

     if(!foundMatching)
         continue;

     //! This if block is for containers, which can have ID3 tags at start
     if(FILE_SOURCE_AAC == formatToCheck ||
        FILE_SOURCE_MP3 == formatToCheck ||
        FILE_SOURCE_FLAC == formatToCheck ||
        FILE_SOURCE_APE == formatToCheck) {
        // AAC, MP3, APE, FLAC files may have ID3 tag at start of the file.
        // To find proper sync marker ID3 tag worth size of data needs to be skipped.
        // 1) Read 12 bytes.
        // 2) Check if ID3Tag present or not
        // 3) If ID3 tag present than read from id3 size, else from 0
        if( nDataRead <  3 * sizeof(uint32)) {
          retReadSize = source->readAt(0, headerBuff, 3 * sizeof(uint32));
          nDataRead   = retReadSize;
          if(retReadSize < 3 * sizeof(uint32)) {
            LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
            break;
          }
        }
        bool eRet = false;
        uint32 ulID3TagLen = 0;
        eRet = GetID3Size(headerBuff, &ulID3TagLen, nDataRead);
        LOGV("GetID3Size ulID3TagLen %u",ulID3TagLen);
        if((ulID3TagLen < MAX_METADATA_SIZE)&&(ulID3TagLen > 0))
        {
          retReadSize = source->readAt(ulID3TagLen,headerBuff,AAC_FORMAT_HEADER_SIZE);
        }
        else if(0 == ulID3TagLen)
        {
          retReadSize = source->readAt(0,headerBuff,AAC_FORMAT_HEADER_SIZE);
        }
        else
        {
          LOGE(" ID3 tag size is corrupted");
          break;
        }
        if(retReadSize == 0)
        {
          LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
          break;
        }
        nDataRead = retReadSize;
        status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,
                                        &nDataRead);
        LOGV("Return status after 1st call %d, size %lu", status, nDataRead);
     }
     //! DTS is special case, where sync marker can come within 64KB
     else if(formatToCheck == FILE_SOURCE_DTS)
     {
         //! DTS requires 64K worth of data to validate DTS sync marker
         retReadSize = source->readAt(0, headerBuff, MAX_FORMAT_HEADER_SIZE);
         nDataRead   = retReadSize;
         //! Min 1024 worth of data needs to be read
         if(retReadSize < (ssize_t)buffSize) {
             LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
             break;
         }
         status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,&nDataRead);
     }
     else
     {
         status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,&nDataRead);
         LOGV("Updated nDataRead value after format %d check is %lu",
              formatToCheck, nDataRead);
     }

     if(status == FILE_SOURCE_SUCCESS)
     {
         retVal = true;
         *confidence = 0.8;

         switch(formatToCheck)
         {
             case FILE_SOURCE_AVI:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_AVI;
                 LOGV(" SniffAVI success<=== \n");
                 break;

            case FILE_SOURCE_AC3:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_AC3;
                 LOGV(" SniffAC3 success<=== \n");
                 break;

             case FILE_SOURCE_ASF:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_ASF;
                 LOGV(" SniffASF success<=== \n");
                 break;

             case FILE_SOURCE_QCP:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCP;
                 LOGV(" SniffQCP success<=== \n");
                 break;

             case FILE_SOURCE_AAC:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_AAC;
                 LOGV(" SniffAAC success<=== \n");
                 break;

             case FILE_SOURCE_DTS:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_DTS;
                 LOGV(" SniffDTS success<=== \n");
                 break;

             case FILE_SOURCE_DSF:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_DSF;
                 LOGV(" SniffDSF success<=== \n");
                 break;

             case FILE_SOURCE_DSDIFF:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_DFF;
                 LOGV(" SniffDSDIFF success<=== \n");
                 break;
             case FILE_SOURCE_AMR_NB:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_AMR_NB;
                 LOGV(" SniffAMRNB success<=== \n");
                 break;

             case FILE_SOURCE_AMR_WB:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_AMR_WB;
                 LOGV(" SniffAMRWB success<=== \n");
                 break;

             case FILE_SOURCE_WAV:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_WAV;
                 LOGV(" SniffWAV success<=== \n");
                 break;

             case FILE_SOURCE_OGG:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_OGG;
                 LOGV(" SniffOGG success<=== \n");
                 break;

             case FILE_SOURCE_MP3:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_MPEG;
                 LOGV(" SniffMP3 success<=== \n");
                 break;

             case FILE_SOURCE_MP2TS:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG2TS;
                 LOGV(" SniffMPEG2TS success<=== \n");
                 break;

             case FILE_SOURCE_MKV:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_MATROSKA;
                  LOGV(" SniffMKV success<=== \n");
                  break;

             case FILE_SOURCE_FLV:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCFLV;
                 LOGV(" SniffFLV success<=== \n");
                 break;

            case FILE_SOURCE_MP2PS:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG2PS;
                 LOGV(" SniffMPEG2PS success<=== \n");
                 break;

            case FILE_SOURCE_MPEG4:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG4;
                 LOGV(" Sniff3gp success<=== \n");
                 break;

             case FILE_SOURCE_3G2:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_3G2;
                 LOGV(" Sniff3G2 success<=== \n");
                 break;
#if defined(FLAC_OFFLOAD_ENABLED) || defined(QTI_FLAC_DECODER)
             case FILE_SOURCE_FLAC:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_FLAC;
                 LOGV(" SniffFLAC success<=== \n");
                 break;
#endif
             case FILE_SOURCE_AIFF:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_AIFF;
                 LOGV(" SniffAIFF success<=== \n");
                 break;

             case FILE_SOURCE_APE:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_APE;
                 LOGV(" SniffAPE success<=== \n");
                 break;
             case FILE_SOURCE_MOV:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_MOV;
                 LOGV(" SniffMOV success<=== \n");
                 break;

             default:
                 LOGE("Something wrong, breaking ");
                 break;
         }
     } else {
         LOGV("Didn't match the sniff <=== status = %d, formattocheck %d ",status, formatToCheck);
         buffSize = FORMAT_HEADER_SIZE;   // making sure this is not over written
     }

     if(retVal)
        break;

  } //! for(int index=0; index<MAX_SINFF_COUNT; ++index)
  //! Free the buffer before exit
  if(headerBuff != NULL) {
     free(headerBuff);
     headerBuff = NULL;
  }

  bool isDrm = false;
  DrmManagerClient *drmManagerClient;
  sp<DecryptHandle> decryptHandle;
  source->DrmInitialization();
  source->getDrmInfo(decryptHandle, &drmManagerClient);
  if (decryptHandle != NULL) {
      isDrm = true;
      ALOGI("Prefer qti parser for DRM files");
  }

  return retVal;

}

#if 0
bool Sniff3G2(const sp<DataSource> &source, String8 *mimeType, float *confidence, sp<AMessage> */*meta*/){

  LOGV("Sniff3G2 Start\n");
  bool retVal = false;

  FileSourceFileFormat formatToCheck = FILE_SOURCE_3G2;
  uint32 buffSize= FORMAT_HEADER_SIZE;
  uint8_t headerBuff[buffSize];
  FileSourceStatus status = FILE_SOURCE_INVALID;

  ssize_t retSize = source->readAt(0, headerBuff, buffSize);
  if(retSize < (ssize_t)buffSize) {
      LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
      return false;
  }

  status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,&buffSize);

  if(status == FILE_SOURCE_SUCCESS) {
      *mimeType = MEDIA_MIMETYPE_CONTAINER_3G2;
      LOGV("Sniff_3G2 success<=== \n");
      *confidence = 0.8; //bumping the confidence
      retVal = true;
  } else {
      LOGW("Didn't match the sniff3G2 <=== status = %d ",status);
  }

  return retVal;
}
#endif

const DataSource::SnifferFunc sniffers[] = {
    ExtendedSniff
};


extern "C" void snifferArray(const DataSource::SnifferFunc* snifferArray[], int *count) {
    (*snifferArray)=sniffers;
    *count = sizeof(sniffers)/sizeof(DataSource::SnifferFunc);
}

} //namespace android
