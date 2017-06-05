// author: panghao
// date: Mon Jun 20 16:50:31 2016

#include "stockshare_engine.h"
#include <time.h>
#include "core/common.h"
#include "base/logic/logic_comm.h"
#include "base/logic/base_values.h"
#include "db/base_db_mysql_auto.h"
#include "public/config/config.h"

#include "logic/logic_unit.h"
#include "net/operator_code.h"
#include "db_mysql.h"
#include "stockshare/stock_base_info.h"

#include <sstream>

namespace stockshare {

StockshareEngine* StockshareEngine::instance_=NULL;

StockshareEngine::StockshareEngine() {
}

StockshareEngine::~StockshareEngine() {
}

bool StockshareEngine::Init() {
  bool r = false;
  std::string my_config_file = "stockshare/base_config.txt";
  stockshare::DbMysql::Init(my_config_file);
  LOG_DEBUG("StockshareEngine::InitStockBasicInfo");
  basic_info_ = StockBasicInfoMgr::Instance();
  r = basic_info_->Init();
  
  return r;
}

StockshareEngine* StockshareEngine::GetInstance() {
  if(instance_==NULL) {
    instance_ = new StockshareEngine();
    instance_->Init();
  }
  return instance_;
}

bool StockshareEngine::GetStockBasicInfo(const std::string &code, StockBasicInfo *info) {
  return basic_info_->GetStockBasicInfo(code, info);
}

bool StockshareEngine::UpdateStockBasicInfo() {
  return basic_info_->UpdateStockBasic();
}

} // stockshare

