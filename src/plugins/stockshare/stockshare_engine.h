// author: panghao
// date: Mon Jun 20 16:50:31 2016

#ifndef __PLUGIN__STOCKSHARE_LOGIC___
#define __PLUGIN__STOCKSHARE_LOGIC___
#include <string>
#include "core/common.h"
#include "net/http_data.h"

namespace stockshare {
struct StockBasicInfo;
class StockBasicInfoMgr;
class StockshareEngine {
 public:
  StockshareEngine();
  virtual ~StockshareEngine();
 private:
  static StockshareEngine  *instance_;
 public:
  static StockshareEngine *GetInstance();
  bool GetStockBasicInfo(const std::string &code, StockBasicInfo *info);
  bool UpdateStockBasicInfo();

private:
  bool Init();
  StockBasicInfoMgr *basic_info_;
};

}

#endif

