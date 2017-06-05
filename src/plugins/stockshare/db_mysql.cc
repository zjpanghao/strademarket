// author: panghao
// date: Mon Jun 20 16:50:31 2016
#include <stdlib.h>
#include "db_mysql.h"
#include <assert.h>
#include <mysql/mysql.h>
#include <map>
#include <sstream>
#include "base/db/base_db_mysql_auto.h"
#include "tools/tools.h"
#include "logic/logic_comm.h"
#include "db/db_config.h"
#include "sqldbpool/mysql_db_pool.h"

namespace stockshare {

// static member
plugin_pub_db::MySqlMultiPool * DbMysql::s_multi_db_pool_pt_ = NULL;

DbMysql::DbMysql() {

}

DbMysql::~DbMysql() {

}

void DbMysql::Dest() {
  // base_db::MysqlDBPool::Dest();
  if (NULL != s_multi_db_pool_pt_) {
    s_multi_db_pool_pt_->Release();
    delete s_multi_db_pool_pt_;
    s_multi_db_pool_pt_ = NULL;
  }
}

void DbMysql::Init(std::string config_file) {
  plugin_pub::BaseConfig ini_config;
  if (!ini_config.LoadFile(config_file)) {
    LOG_DEBUG2("DbMysql::Init,加载配置文件 [%s]失败", config_file.c_str());
  }
  LOG_DEBUG2("DbMysql::Init,加载配置文件[%s]成功", config_file.c_str());
  if (NULL == s_multi_db_pool_pt_) {
    s_multi_db_pool_pt_ = new plugin_pub_db::MySqlMultiPool;
    assert(NULL != s_multi_db_pool_pt_);
  }
  plugin_pub_db::MySqlConfig mysql_config;
  std::string s;
  int n = 0;
  // 初始化连接池1
  s = ini_config.GetParam < std::string > ("mysql1", "host");
  mysql_config.set_host(s);

  n = ini_config.GetParam<int>("mysql1", "port");
  mysql_config.set_port(n);

  s = ini_config.GetParam < std::string > ("mysql1", "user");
  mysql_config.set_user(s);

  s = ini_config.GetParam < std::string > ("mysql1", "pswd");
  mysql_config.set_pswd(s);

  s = ini_config.GetParam < std::string > ("mysql1", "dbname");
  mysql_config.set_dbname(s);

  n = ini_config.GetParam<int>("mysql1", "pool_size");
  mysql_config.set_pool_size(n);
  LOG_DEBUG2("mysql1，host:%s, port:%d, user:%s, pswd:%s, pool_size:%d",
             mysql_config.host().c_str(), mysql_config.port(),
             mysql_config.user().c_str(), mysql_config.pswd().c_str(),
             mysql_config.pool_size());
  // 初始化pool1                                                                      
  s_multi_db_pool_pt_->InitPool(1, mysql_config);

  // 初始化连接池2
  s = ini_config.GetParam < std::string > ("mysql2", "host");
  mysql_config.set_host(s);

  n = ini_config.GetParam<int>("mysql2", "port");
  mysql_config.set_port(n);

  s = ini_config.GetParam < std::string > ("mysql2", "user");
  mysql_config.set_user(s);

  s = ini_config.GetParam < std::string > ("mysql2", "pswd");
  mysql_config.set_pswd(s);

  s = ini_config.GetParam < std::string > ("mysql2", "dbname");
  mysql_config.set_dbname(s);

  n = ini_config.GetParam<int>("mysql2", "pool_size");
  mysql_config.set_pool_size(n);
  LOG_DEBUG2("mysql2，host:%s, port:%d, user:%s, pswd:%s, pool_size:%d",
             mysql_config.host().c_str(), mysql_config.port(),
             mysql_config.user().c_str(), mysql_config.pswd().c_str(),
             mysql_config.pool_size());
  // 初始化pool2                                                                      
  s_multi_db_pool_pt_->InitPool(2, mysql_config);
}

void DbMysql::QueryStockBasics(
    std::map<std::string, StockBasicInfo> *stock_base_map) {
  AUTO_MYSQL_POOL_ENGINE_USE(s_multi_db_pool_pt_, 2, engine);
  if (NULL == engine) {
    LOG_ERROR("DbMysql::LoadStockBasicInfo, NULL == engin");
    return;
  }
  std::stringstream sql;
  sql << "call GetStockBasicInfo()";

  bool r = engine->SQLExec(sql.str().c_str());
  if (!r) {
    LOG_ERROR("DbMysql::GetStockBasicInfo failed");
    return;
  }
  int record_num = engine->RecordCount();
  MYSQL_ROW row_ret;
  for (int i = 0; i < record_num; ++i) {
    row_ret = *(reinterpret_cast<MYSQL_ROW*>(engine->FetchRows()->proc));
    if (NULL != row_ret) {
      int j = 0;
      StockBasicInfo info;
      if (row_ret[j])
        info.code = row_ret[j++];
      if (info.code.length() == 0)
        continue;
      if (row_ret[j])
        info.total = atof(row_ret[j++]);
      if (row_ret[j])
        info.outstanding = atof(row_ret[j++]);
      if (row_ret[j])
        info.pb = atof(row_ret[j++]);
      if (row_ret[j])
        info.pe = atof(row_ret[j++]);
      if (row_ret[j])
        info.bvps = atof(row_ret[j++]);
      if (row_ret[j])
        info.eps = atof(row_ret[j++]);
      (*stock_base_map)[info.code] = info;
    }
  }
}

}  // namespace stockshare

