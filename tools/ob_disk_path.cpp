/*
 *  (C) 2007-2010 Taobao Inc.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *         ob_disk_path.cpp is for what ...
 *
 *  Version: $Id: ob_disk_path.cpp 2011年03月22日 09时54分22秒 qushan Exp $
 *
 *  Authors:
 *     qushan < qushan@taobao.com >
 *        - some work details if you want
 */


#include "sstable/ob_disk_path.h"
#include "chunkserver/ob_tablet.h"
#include "chunkserver/ob_chunk_server_main.h"

using namespace oceanbase::common;
using namespace oceanbase::chunkserver;

namespace oceanbase 
{
  namespace sstable
  {
    uint64_t get_sstable_disk_no(const uint64_t sstable_file_id)
    {
      return (sstable_file_id & DISK_NO_MASK);
    }

    int get_config_item(const char *&data_dir, const char *&app_name)
    {
      int ret = OB_SUCCESS;

      data_dir = "/data";
      app_name = "collect";

      return ret;
    }

    int get_sstable_directory(const int32_t disk_no, char *path, const int64_t path_len)
    {
      int ret = OB_SUCCESS;
      if (disk_no < 0 || NULL == path || path_len < 0)
      {
        TBSYS_LOG(WARN, "get_sstable_directory invalid arguments, "
            "disk_no=%d,path=%p,path_len=%ld", disk_no, path, path_len);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        // read from configure file
        const char *data_dir = NULL;
        const char *app_name = NULL;
        ret = get_config_item(data_dir, app_name);
        if (OB_SUCCESS == ret)
        {
          int bufsiz = snprintf(path, path_len, "%s/%d/%s/sstable", data_dir, disk_no, app_name);
          if (bufsiz + 1 > path_len)
          {
            TBSYS_LOG(WARN, "get_sstable_directory, path_len=%ld <= bufsiz=%d", path_len, bufsiz);
            ret = OB_SIZE_OVERFLOW;
          }
        }
      }


      return ret;
    }

    int get_sstable_path(const ObSSTableId& sstable_id, char *path, const int64_t path_len)
    {
      int ret = OB_SUCCESS;
      if (sstable_id.sstable_file_offset_ < 0 || NULL == path || path_len < 0)
      {
        TBSYS_LOG(WARN, "get_sstable_path invalid arguments, "
            "offset=%ld, path=%p,path_len=%ld", 
            sstable_id.sstable_file_offset_, path, path_len);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        // read from configure file
        const char *data_dir = NULL;
        const char *app_name = NULL;
        ret = get_config_item(data_dir, app_name);
        if (OB_SUCCESS == ret)
        {
          int32_t disk_no =  (sstable_id.sstable_file_id_ & DISK_NO_MASK);
          if (disk_no < 0)
          {
            TBSYS_LOG(WARN, "get_sstable_path, sstable file id = %ld invalid", 
                sstable_id.sstable_file_id_);
            ret = OB_ERROR;
          }
          else
          {
            int bufsiz = snprintf(path, path_len, "%s/%d/%s/sstable/%ld",
                data_dir, disk_no, app_name, sstable_id.sstable_file_id_);
            if (bufsiz + 1 > path_len)
            {
              TBSYS_LOG(WARN, "get_sstable_path, path_len=%ld <= bufsiz=%d", 
                  path_len, bufsiz);
              ret = OB_SIZE_OVERFLOW;
            }
          }
        }
      }

      return ret;
    }

    int get_tmp_meta_path(const int32_t disk_no, char *path, const int32_t path_len)
    {
      int ret = OB_SUCCESS;
      if (disk_no <= 0 || NULL == path ||  path_len <= 0)
      {
        TBSYS_LOG(WARN, "get_tmp_meta_path invalid arguments, "
            "disk_no=%d,path=%p,path_len=%d", disk_no, path, path_len);
        ret = OB_INVALID_ARGUMENT;
      }

      if (OB_SUCCESS == ret)
      {
        const char *data_dir = NULL;
        const char *app_name = NULL;
        ret = get_config_item(data_dir, app_name);
        if (OB_SUCCESS == ret)
        {
          int64_t current_time = tbsys::CTimeUtil::getTime();
          int bufsiz = snprintf(path, path_len, "%s/%d/%s/sstable/tmp_idx_%d_%ld", 
              data_dir, disk_no, app_name, disk_no, current_time);

          if (bufsiz + 1 > path_len)
          {
            TBSYS_LOG(WARN, "get_meta_path, path_len=%d <= bufsiz=%d", path_len, bufsiz);
            ret = OB_SIZE_OVERFLOW;
          }
        }
      }

      return ret;
    }

    int get_meta_path(const int32_t disk_no, const bool current, char *path, const int32_t path_len)
    {
      int ret = OB_SUCCESS;
      if (disk_no <= 0 || NULL == path ||  path_len <= 0)
      {
        TBSYS_LOG(WARN, "get_meta_path invalid arguments, "
            "disk_no=%d,path=%p,path_len=%d", disk_no, path, path_len);
        ret = OB_INVALID_ARGUMENT;
      }

      if (OB_SUCCESS == ret)
      {
        const char *data_dir = NULL;
        const char *app_name = NULL;
        ret = get_config_item(data_dir, app_name);
        if (OB_SUCCESS == ret)
        {
          int bufsiz = 0;
          if (current) 
          {
            bufsiz = snprintf(path, path_len, "%s/%d/%s/sstable/idx_%d", 
                data_dir, disk_no, app_name, disk_no);
          }
          else
          {
            time_t t;
            ::time(&t);
            struct tm* tm = ::localtime(&t);
            bufsiz = snprintf(path, path_len, "%s/%d/%s/sstable/idx_%d.%04d%02d%02d%02d%02d%02d", 
                data_dir, disk_no, app_name, disk_no, 
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
          }
          if (bufsiz + 1 > path_len)
          {
            TBSYS_LOG(WARN, "get_meta_path, path_len=%d <= bufsiz=%d", path_len, bufsiz);
            ret = OB_SIZE_OVERFLOW;
          }
        }
      }

      return ret;
    }

    int get_meta_path(const int64_t version, const int32_t disk_no, 
        const bool current, char *path, const int32_t path_len)
    {
      int ret = OB_SUCCESS;
      if (version <=0 || disk_no <= 0 || NULL == path ||  path_len <= 0)
      {
        TBSYS_LOG(WARN, "get_sstable_directory invalid arguments, "
            "version=%ld, disk_no=%d,path=%p,path_len=%d", 
            version, disk_no, path, path_len);
        ret = OB_INVALID_ARGUMENT;
      }

      if (OB_SUCCESS == ret)
      {
        // read from configure file
        const char *data_dir = NULL;
        const char *app_name = NULL;
        ret = get_config_item(data_dir, app_name);
        if (OB_SUCCESS == ret)
        {
          int bufsiz = 0;
          if (current) 
          {
            bufsiz = snprintf(path, path_len, "%s/%d/%s/sstable/idx_%ld_%d", 
                data_dir, disk_no, app_name, version, disk_no);
          }
          else
          {
            time_t t;
            ::time(&t);
            struct tm* tm = ::localtime(&t);
            bufsiz = snprintf(path, path_len, "%s/%d/%s/sstable/idx_%ld_%d.%04d%02d%02d%02d%02d%02d", 
                data_dir, disk_no, app_name, version, disk_no, 
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
          }
          if (bufsiz + 1 > path_len)
          {
            TBSYS_LOG(WARN, "get_sstable_directory, path_len=%d <= bufsiz=%d", path_len, bufsiz);
            ret = OB_SIZE_OVERFLOW;
          }
        }
      }

      return ret;
    }


    int get_recycle_directory(const int32_t disk_no, char *path, const int64_t path_len)
    {
      int ret = OB_SUCCESS;
      if (disk_no < 0 || NULL == path || path_len < 0)
      {
        TBSYS_LOG(WARN, "get_recycle_directory invalid arguments, "
            "disk_no=%d,path=%p,path_len=%ld", disk_no, path, path_len);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        // read from configure file
        const char *data_dir = NULL;
        const char *app_name = NULL;
        ret = get_config_item(data_dir, app_name);
        if (OB_SUCCESS == ret)
        {
          int bufsiz = snprintf(path, path_len, "%s/%d/Recycle", data_dir, disk_no);
          if (bufsiz + 1 > path_len)
          {
            TBSYS_LOG(WARN, "get_recycle_directory , path_len=%ld <= bufsiz=%d", path_len, bufsiz);
            ret = OB_SIZE_OVERFLOW;
          }
        }
      }


      return ret;
    }

    int get_import_sstable_directory(const int32_t disk_no, char *path, 
      const int64_t path_len)
    {
      int ret = OB_SUCCESS;
      if (disk_no < 0 || NULL == path || path_len < 0)
      {
        TBSYS_LOG(WARN, "get_sstable_import_directory invalid arguments, "
            "disk_no=%d, path=%p, path_len=%ld", disk_no, path, path_len);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        // read from configure file
        const char *data_dir = NULL;
        const char *app_name = NULL;
        ret = get_config_item(data_dir, app_name);
        if (OB_SUCCESS == ret)
        {
          int bufsiz = snprintf(path, path_len, "%s/%d/%s/import", data_dir, disk_no, app_name);
          if (bufsiz + 1 > path_len)
          {
            TBSYS_LOG(WARN, "get_sstable_import_directory, path_len=%ld <= bufsiz=%d", path_len, bufsiz);
            ret = OB_SIZE_OVERFLOW;
          }
        }
      }

      return ret;
    }

    int get_import_sstable_path(const int32_t disk_no, 
      const char* sstable_name, char *path, const int64_t path_len)
    {
      int ret = OB_SUCCESS;
      if (disk_no < 0 || NULL == sstable_name || NULL == path || path_len < 0)
      {
        TBSYS_LOG(WARN, "get_import_sstable_path invalid arguments, "
            "disk_no=%d, sstable_name=%p, path=%p,path_len=%ld", 
            disk_no, sstable_name, path, path_len);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        // read from configure file
        const char *data_dir = NULL;
        const char *app_name = NULL;
        ret = get_config_item(data_dir, app_name);
        if (OB_SUCCESS == ret)
        {
          int bufsiz = snprintf(path, path_len, "%s/%d/%s/import/%s",
              data_dir, disk_no, app_name, sstable_name);
          if (bufsiz + 1 > path_len)
          {
            TBSYS_LOG(WARN, "get_import_sstable_path, path_len=%ld <= bufsiz=%d", 
                path_len, bufsiz);
            ret = OB_SIZE_OVERFLOW;
          }
        }
      }

      return ret;
    }

    int idx_file_name_filter(const struct dirent *d)
    {
      int ret = 0;
      if (NULL != d)
      {
        ret = (0 == strncmp(d->d_name,"idx_",4)) ? 1 : 0;
      }
      return ret;
    }

    int bak_idx_file_name_filter(const struct dirent *d)
    {
      int ret = 0;
      if (NULL != d)
      {
        ret = (0 == strncmp(d->d_name,"bak_idx_",8)) ? 1 : 0;
      }
      return ret;
    }


  }
}
