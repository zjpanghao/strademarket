// author: panghao
// date: Mon Jun 20 16:50:31 2016

#ifndef _PLUGINS_STOCKSHARE_DB_MYSQL_H_
#define _PLUGINS_STOCKSHARE_DB_MYSQL_H_

#include <list>
#include <map>
#include <set>
#include <string>
#include "public/config/config_info.h"
#include "baseconfig/base_config.h" 
#include "sqldbpool/mysql_db_pool.h"
#include "stockshare/stock_base_info.h"

namespace stockshare {
class DbMysql {
 public:
  DbMysql();
  ~DbMysql();

  static void Init(std::string config_file);
  static void Dest();
  // 查询A股下所有股票代码
  static void QueryStockBasics(
      std::map<std::string, StockBasicInfo> *stock_base_map);
 protected:
  static plugin_pub_db::MySqlMultiPool *s_multi_db_pool_pt_;
};

}
#endif // PLUGINS_STOCKSHARE_DB_MYSQL_H_ 
