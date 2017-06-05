/*=============================================================================
 #    Copyright (c) 2015
 #    ShanghaiKunyan.  All rights reserved
 #
 #    Filename     : /home/tyh/project/wookong/northsea/restful/plugins/stockshare/stock_base_info.cc
 #    Author       : Tianyiheng
 #    Email        : tianyiheng@kunyan-inc.com
 #    Date         : 2016-06-20 18:13
 #    Description  :
 =============================================================================*/
#include <cassert>
#include <algorithm>
#include "stockshare/stock_base_info.h"
#include "tools/tools.h"
#include "stockshare/db_mysql.h"

namespace stockshare {

StockBasicInfoMgr* StockBasicInfoMgr::s_instance_ = NULL;
pthread_mutex_t StockBasicInfoMgr::s_mutex_ = PTHREAD_MUTEX_INITIALIZER;

StockBasicInfoMgr* StockBasicInfoMgr::Instance() {
  if (NULL == s_instance_) {
    pthread_mutex_lock(&s_mutex_);
    if (NULL == s_instance_) {
      s_instance_ = new StockBasicInfoMgr();
      assert(NULL != s_instance_);
    }
    pthread_mutex_unlock(&s_mutex_);
  }
  return s_instance_;
}

void StockBasicInfoMgr::FreeInstance() {
  pthread_mutex_lock(&s_mutex_);
  if (NULL != s_instance_) {
    delete s_instance_;
    s_instance_ = NULL;
  }
  pthread_mutex_unlock(&s_mutex_);
}

bool StockBasicInfoMgr::UpdateStockBasic() {
  std::map<std::string, StockBasicInfo> base_info;
  DbMysql::QueryStockBasics(&base_info);
  if (base_info.size() == 0)
    return false;
  base_logic::WLockGd manager_lock(lock_);
  stockbase_.swap(base_info);
  LOG_DEBUG2("stokbase size:%d", stockbase_.size());
  return true;
}

}  // namespace stockshare
