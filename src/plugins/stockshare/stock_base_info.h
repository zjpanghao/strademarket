/*=============================================================================
#    Copyright (c) 2015
#    ShanghaiKunyan.  All rights reserved
#
#    Filename     : /home/tyh/project/wookong/northsea/restful/plugins/stockshare/stock_base_info.h
#    Author       : Tianyiheng
#    Email        : tianyiheng@kunyan-inc.com
#    Date         : 2016-06-20 18:13
#    Description  : 
=============================================================================*/

#ifndef __WOOKONG_PLUGIN_STOCKSHARE_STOCK_BASE_INFO_H__
#define __WOOKONG_PLUGIN_STOCKSHARE_STOCK_BASE_INFO_H__

#include <pthread.h>
#include <set>
#include <string>
#include <vector>
#include <list>
#include <map>
#include "logic/logic_comm.h"


namespace stockshare {

struct StockBasicInfo {
 public:
  StockBasicInfo() 
     : total(0), outstanding(1), pb(0), pe(0), bvps(0), eps(0) {

  }
  std::string code;
  double total;
  double outstanding;
  double pb;
  double pe;
  double bvps;
  double eps;
};

class StockBasicInfoMgr {
 public:
  StockBasicInfoMgr(){ InitThreadrw(&lock_);}
  ~StockBasicInfoMgr() {}
  
 public:
   static StockBasicInfoMgr* Instance();
   static void FreeInstance();
   bool Init() {
     return UpdateStockBasic();
   }
   bool UpdateStockBasic();
   bool GetStockBasicInfo(const std::string &code, StockBasicInfo *info) {
     base_logic::RLockGd manager_lock(lock_);
     std::map<std::string, StockBasicInfo>::iterator it =
         stockbase_.find(code);
     if (it == stockbase_.end()) {
       return false;
     }
     *info = it->second;
     return true;
   }
 protected:
   std::map<std::string, StockBasicInfo> stockbase_;
   static StockBasicInfoMgr* s_instance_;
   static pthread_mutex_t s_mutex_;
   struct threadrw_t* lock_;
};

#define StockBaseMgrPt stockshare::StockBasicInfoMgr::Instance()

}  // namespace stockshare


#endif // __WOOKONG_PLUGIN_STOCKSHARE_STOCK_BASE_INFO_H__
