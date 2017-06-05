// author: panghao
// date: Mon Jun 20 16:50:31 2016

#ifndef __PLUGIN__REALINFO_LOGIC___
#define __PLUGIN__REALINFO_LOGIC___
#include <string>
#include "core/common.h"
#include "net/http_data.h"
//#include "realinfo/src/realinfo_data.h"
namespace stockshare {
class StockshareEngine;
}
namespace stock_price {
class StockPrice;
struct RealtimeCodeInfo;
};

namespace realinfo {
struct StockRealInfo;
struct StockDealNInfo;
class RealInfoLogic {
 public:
  RealInfoLogic();
  virtual ~RealInfoLogic();
 private:
  static RealInfoLogic *instance_;
 public:
  static RealInfoLogic *GetInstance();
  static void FreeInstance();
 public:
  bool OnRealInfoConnect(struct server *srv, const int socket);
  bool OnRealInfoMessage(struct server *srv, const int socket,
                            const void *msg, const int len);
  bool OnRealInfoClose(struct server *srv, const int socket);
  bool OnBroadcastConnect(struct server *srv, const int socket,
                          const void *data, const int len);
  bool OnBroadcastMessage(struct server *srv, const int socket, const void *msg,
                          const int len);
  bool OnBroadcastClose(struct server *srv, const int socket);
  bool OnIniTimer(struct server *srv);
  bool OnTimeout(struct server *srv, char* id, int opcode, int time);

 protected:
  bool OnRequestRealInfo(NetBase *kv, struct server *srv, const int socket,
                            const void *msg, const int len);
 private:
  bool CheckCacheDataValid(const std::string &cache_result);
  bool OnMarketIndexInfo(struct server *srv,
                         const int socket, NetBase* netbase,
                         const void *msg, const int len);
  std::string GetStradeDay(time_t stamp);
  bool Init();
  bool OnSingleStockLatestRecords(struct server *srv, const int socket,
                            NetBase* netbase, const void* msg, const int len);
  bool OnSingleStockTodayRecords(struct server *srv, const int stocket,
                            NetBase* netbase, const void* msg, const int len);
                             
  stock_price::StockPrice *market_engine_;
  stockshare::StockshareEngine *stock_engine_;
  bool InitRealInfo(const stock_price::RealtimeCodeInfo &code_info, StockRealInfo *info);
  bool BuildIndexData(std::list<StockRealInfo> *info_list);
  bool InitBuyN(stock_price::RealtimeCodeInfo &stock_real_info,
                StockDealNInfo *info);
  void UpdateHistory();
  int day_;
};

}  // namespace candlestock

#endif // __PLUGIN__REALINFO_LOGIC___

