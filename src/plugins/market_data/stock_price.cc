#include "stock_price.h"
#include "storage/data_engine.h"
#include "stock_util.h"
#include "user_redis_controller.h"
#include "market_data_engine.h"
#include "realinfo_util.h"
#include "realinfo_control.h"
#include <pthread.h>
#define ONE_DAY_SECONDS 86400

namespace stock_price {

StockPrice * SingleStockPrice::instance_ = NULL;
pthread_mutex_t SingleStockPrice::mutex_ = PTHREAD_MUTEX_INITIALIZER;

const char* const hkey_prefix_[DataEngine::MAX_TYPE] = { "sh_stock_",
    "sz_stock_" };

bool StockPrice::Init(std::list<base::ConnAddr>& addrlist) {
  bool ret = true;
  LOG_MSG2("The addrlist size %d", addrlist.size());
  LOG_MSG2("%s", "Now Init param");
  redis_.InitParam(addrlist);
  for (size_t i = 0; i < DataEngine::MAX_TYPE; ++i) {
    data_[i].Init((DataEngine::Type) i, hkey_prefix_[i], &redis_);
  }
  RealInfoControl::GetInstance();
  UpdataWhenBoot();
  return ret;
}

void StockPrice::UpdataWhenBoot() {
  for (size_t i = 0; i < DataEngine::MAX_TYPE; ++i) {
    std::map<std::string, DataEngine::CodeInfoArray> day_data;
    DataEngine *data = &data_[i];
    time_t now = time(NULL);
    std::string date = RealInfoUtil::BuildFetchDate(now);
    time_t ts = now;
    struct tm current;
    localtime_r(&now, &current);
    while (1) {
      date = RealInfoUtil::BuildFetchDate(ts);
      if (RealInfoUtil::IsWorkDay(date) && current.tm_hour > 9) {
        LOG_MSG2("Check %s ok", date.c_str());
        data->FetchDayData(date, &day_data);
        LOG_MSG2("SIZE %d", day_data.size());
        RealInfoControl::GetInstance()->UpdateData(date, day_data);
        break;
      }
      ts -= ONE_DAY_SECONDS;
      current.tm_hour = 10;   // makesure yesterday ok, if it is a trading day
    }
  }
}

void StockPrice::ScanRedis() {

  time_t ts = time(NULL);
  struct tm current;
  std::string date = RealInfoUtil::BuildFetchDate(ts);
  localtime_r(&ts, &current);
  if (!RealInfoUtil::IsWorkDay(date) || !RealInfoUtil::InScanHour(current)) {
    return;
  }
  
  LOG_MSG2("%s", "scanredis");
  pthread_mutex_lock(&mutex_lock_);
  std::map<std::string, DataEngine::CodeInfoArray> day_map;
  for (size_t i = 0; i < DataEngine::MAX_TYPE; ++i) {
    data_[i].Update(date, &day_map);
    if (day_map.size() > 0)
      RealInfoControl::GetInstance()->UpdateData(date, day_map);
  }
  pthread_mutex_unlock(&mutex_lock_);
}

void StockPrice::GetKLineData(const std::string &code,
                              DataEngine::CodeInfoArray& code_info) {
  std::list<RealtimeCodeInfo> info;
  RealInfoControl::GetInstance()->GetKLineData(code, &info);
  LOG_MSG2("The size %d", code_info.size());
  code_info.reserve(code_info.size());
  std::list<RealtimeCodeInfo>::iterator it = info.begin();
  while (it != info.end()) {
    code_info.push_back(*it++);
  }

}

bool StockPrice::GetLatestInfo(const std::string &code,
                               RealtimeCodeInfo *info) {
  return RealInfoControl::GetInstance()->GetLatestInfo(code, info);
}

bool StockPrice::GetLatestAllInfo(
    std::map<std::string, RealtimeCodeInfo> *info) {
  return RealInfoControl::GetInstance()->GetLatestAllInfo(info);
}

}
