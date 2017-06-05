//  Copyright (c) 2015-2015 The george Authors. All rights reserved.
//  Created on: 2016年5月18日 Author: kerry

#ifndef DATA_MYSQL_ENGINE_H__
#define DATA_MYSQL_ENGINE_H__
#include <string>
#include <list>
#include "mysql_pool.h"
#include "storage/data_engine.h"
#include "logic/base_values.h"
#include "basic/scoped_ptr.h"

namespace base_logic {

class DataMYSQLEngine : public DataEngine {
 public:
	DataMYSQLEngine() {}
	virtual ~DataMYSQLEngine() {}
 public:
     void InitParam(std::list<base::ConnAddr>& addrlist);
     void Release();
  public:
     bool ReadData(const int32 type, base_logic::Value* value,
             void (*storage_get)(void*, base_logic::Value*));

     bool WriteData(const int32 type, base_logic::Value* value);

  private:
     base_logic::MYSQL_Pool        db_pool_;
};
}
#endif
