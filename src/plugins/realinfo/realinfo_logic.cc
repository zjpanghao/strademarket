// author: pangh
// date: Mon Jun 20 16:50:31 2016

#include "realinfo_logic.h"
#include "core/common.h"
#include "base/logic/logic_comm.h"
#include "base/logic/base_values.h"
#include "public/config/config.h"

#include "logic/logic_unit.h"
#include "net/operator_code.h"
#include "realinfo/realinfo_pro.h"
#include "stockshare/stock_base_info.h"
#include "stockshare/stockshare_engine.h"
#include "market_info/stock_price.h"
#include "market_info/realinfo_util.h"
#include <sstream>

#define DEFAULT_CONFIG_PATH                    "./plugins/realinfo/realinfo_config.xml"
#define TIME_REALINFO_UPDATE_ALL                          10000
const int UPDATE_STOCK = 1; 
const int UPDATE_MARKET = 2; 
#define SEND_HTTP_ERROR(b) \  
  do {\
  std::string response = b;\
  base_logic::LogicUnit::SendMessageBySize(socket, response);\
  }while(0)
#define SEND_ERROR_INFO(c, i) \  
  do {\
  SendRealInfoLatestProtocol error_pro;\
  error_pro.SetErrorState(c, i);\
  std::string response = error_pro.GetJson();\
  base_logic::LogicUnit::SendMessageBySize(socket, response);\
  }while(0)
#define SEND_HTTP_INFO(b) \  
  do {\
  std::string response = b;\
  base_logic::LogicUnit::SendMessageBySize(socket, response);\
  }while(0)

namespace realinfo {

RealInfoLogic* RealInfoLogic::instance_ = NULL;

RealInfoLogic::RealInfoLogic() {
  if (!Init())
    assert(0);
}

RealInfoLogic::~RealInfoLogic() {
}

bool RealInfoLogic::Init() {
  bool r = false;
  srand(time(NULL));
  market_engine_ = stock_price::SingleStockPrice::GetInstance();
  market_engine_->Init(true);
  stock_engine_ = stockshare::StockshareEngine::GetInstance();
  struct tm current;
  time_t now = time(NULL);
  localtime_r(&now, &current);
  day_ = current.tm_mday;
  return true;
}

RealInfoLogic* RealInfoLogic::GetInstance() {
  if (instance_ == NULL)
    instance_ = new RealInfoLogic();
  return instance_;
}

void RealInfoLogic::FreeInstance() {
  delete instance_;
  instance_ = NULL;
}

bool RealInfoLogic::OnRealInfoConnect(struct server *srv,
                                            const int socket) {
  return true;
}

bool RealInfoLogic::OnRealInfoMessage(struct server *srv,
                                            const int socket, const void *msg,
                                            const int len) {
  bool r = true;
  do {
    const char* packet = reinterpret_cast<const char*>(msg);
    if (NULL == packet) {
      r = false;
      break;
    }
    std::string http_str(packet, len);
    std::string error_str;
    int error_code = 0;
    scoped_ptr <base_logic::ValueSerializer> serializer(
        base_logic::ValueSerializer::Create(base_logic::IMPL_HTTP,
                                            &http_str));
    NetBase* value = (NetBase*) (serializer.get()->Deserialize(&error_code,
                                                               &error_str));
    LOG_DEBUG2("http_str:%s", http_str.c_str());
    if (NULL == value) {
      error_code = STRUCT_ERROR;
      LOG_MSG("error struct ");
      SEND_ERROR_INFO(STRUCT_ERROR, "error struct");
      r = true;
      break;
    }
    scoped_ptr < RecvPacketBase > head_packet(new RecvPacketBase(value));
    int32 type = head_packet->GetType();
    switch (type) {
      case REALINFO_LATESTN: {
        r = OnSingleStockLatestRecords(srv, socket, value, msg, len);
        break;
      }
      case REALINFO_INDEX: {
        r = OnMarketIndexInfo(srv, socket, value, msg, len);
        break;
      }
      default: {
        r = false;
        break;
      }
    }
  } while (0);
  return r;
}

bool RealInfoLogic::OnRealInfoClose(struct server *srv,
                                          const int socket) {
  return true;
}

bool RealInfoLogic::OnBroadcastConnect(struct server *srv, const int socket,
                                          const void *msg, const int len) {
  return true;
}

bool RealInfoLogic::OnBroadcastMessage(struct server *srv, const int socket,
                                          const void *msg, const int len) {
  return true;
}

bool RealInfoLogic::OnBroadcastClose(struct server *srv, const int socket) {
  return true;
}

bool RealInfoLogic::OnIniTimer(struct server *srv) {
   srv->add_time_task(srv, "realinfo", UPDATE_MARKET, 10, -1);
   srv->add_time_task(srv, "realinfo", UPDATE_STOCK, 3600, -1);
  return true;
}

bool RealInfoLogic::OnTimeout(struct server *srv, char *id, int opcode,
                              int time) {
  switch (opcode) {
    case UPDATE_STOCK:
      stock_engine_->UpdateStockBasicInfo();
      break;
    case UPDATE_MARKET:
      market_engine_->ScanRedis();
      break;
  }
  return true;
}

bool RealInfoLogic::InitRealInfo(const stock_price::RealtimeCodeInfo &share_info, StockRealInfo *info) {
  if (!info)
    return false;
  info->open_price = share_info.open;
  info->low_price = share_info.low;
  info->yesterday_close_price = share_info.close;
  info->high_price = share_info.high;
  info->now_price = share_info.price;
  info->avg_price = (info->high_price + info->low_price + info->now_price)/3;
  info->amount = share_info.amount;
  info->vol = share_info.vol;
  LOG_MSG2("The vol %f\n", share_info.vol);
  info->limit_down = share_info.lower;
  info->limit_up = share_info.upper;
  info->change_price = info->now_price - info->yesterday_close_price;
  info->change_rate = (info->now_price - info->yesterday_close_price) / (info->yesterday_close_price + 0.000001);
  info->turnover_rate = 0;
  info->name = share_info.name;
  return true; 
}

std::string RealInfoLogic::GetStradeDay(time_t stamp) {
  time_t seconds = stamp;
  struct tm day;
  localtime_r(&seconds, &day);
  char buf[64];
  snprintf(buf, 64, "%04d-%02d-%02d", day.tm_year + 1900, day.tm_mon + 1, day.tm_mday);
  LOG_MSG2("The day is %s", buf);
  return buf;  
}

bool RealInfoLogic::BuildIndexData(std::list<StockRealInfo> *info_list) {
  
  std::list<StockRealInfo> ll;
  const char *codes[] = {"sh000001", "sh399001", "hs300"};
  const char *names[] = {"上证指数", "深证成指", "沪深300"};
  for (int i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
    std::string code = codes[i];
#ifdef USE_MARKET
    stock_price::RealtimeCodeInfo   stock_real_info;
    if (!market_engine_->GetLatestInfo(code, &stock_real_info)) {
      LOG_MSG2("Get stock error for %s", code.c_str());
      return false;
    } 
#endif
    StockRealInfo info;
    InitRealInfo(stock_real_info, &info);
    if (i < sizeof(names) / sizeof(names[0]))
      info.name = names[i];
    info_list->push_back(info);
  }
  return true;
}

bool RealInfoLogic::OnMarketIndexInfo(struct server *srv,
                                      const int socket, NetBase* netbase,
                                      const void *msg, const int len) {
  SendRealInfoLatestProtocol index_pro;
  std::list<StockRealInfo> index_list;
  if (BuildIndexData(&index_list) == false) {
    SEND_ERROR_INFO(-1, "get index data error");
    return false;
  }
  index_pro.SetIndexInfo(index_list);
  std::string json = index_pro.GetJson();
  LOG_MSG2("The json %s\n", json.c_str());
  SEND_HTTP_INFO(json);
  return true;
  
}

bool RealInfoLogic::InitBuyN(stock_price::RealtimeCodeInfo &stock_real_info,
                             StockDealNInfo *info) {
  stock_price::RealtimeCodeInfo::Order *porder = stock_real_info.orders;
  for (int i = 0; i < 5; i++) {
    StockDealInfo buy_deal_info = {porder->buy, porder->buy_vol};
    info->buy.push_back(buy_deal_info);
    StockDealInfo sell_deal_info = {porder->sale, porder->sale_vol};
    LOG_MSG2("sail %d price %d", i, porder->sale);
    info->sell.push_back(sell_deal_info);
    porder++;
  }
  return true;
}

bool RealInfoLogic::OnSingleStockLatestRecords(struct server *srv,
                                               const int socket, NetBase* netbase,
                                               const void* msg, const int len) {
  bool r = true;
  int err = 0;
  std::string stock_code;
  int64 record_type;
  r = netbase->GetString(L"stock_code", &stock_code);
  if (!r) {
    err = -1;
    LOG_ERROR("OnSingleCandle error");
    SEND_ERROR_INFO(-100, "error stock_code");
    return false;
  }
  stock_code = stock_code.substr(0, 6);
  LOG_MSG2("stockcodes %s\n", stock_code.c_str());
  
  SendRealInfoLatestProtocol candle_pro;
  StockDealNInfo info;
#ifdef USE_MARKET
  stock_price::RealtimeCodeInfo stock_real_info;
  if (market_engine_->GetLatestInfo(stock_code, &stock_real_info) == false) {
    LOG_ERROR2("%s", "Get stock error");
    LOG_MSG2("%s", "Get stock error");
    SEND_ERROR_INFO(-1, "error GetStockCurrRealMarketInfo error");
    return false;
  }
#endif
  InitBuyN(stock_real_info, &info);
  StockRealInfo real_info;
  InitRealInfo(stock_real_info, &real_info);
  double outstand = 0 ; 
#ifdef USE_STOCKSHARE
  stockshare::StockBasicInfo basic_info;
  if (!stock_engine_->GetStockBasicInfo(stock_code, &basic_info)) {
    SEND_ERROR_INFO(-1, "error GetStockTotalError");
    return false;
  }
  outstand = basic_info.outstanding;
#endif
  LOG_MSG2("The outstanding %f", outstand);
  real_info.turnover_rate =  real_info.vol*100/100000000/(outstand + 0.000000001);
  candle_pro.set_latest_info(info, real_info);
  std::string json = candle_pro.GetJson();
  LOG_MSG2("The json %s\n", json.c_str());
  SEND_HTTP_INFO(json);
  return r;
}

}  // realinfo

