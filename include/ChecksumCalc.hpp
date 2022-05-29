// Copyright 2021 Your Name <your_email>

#ifndef INCLUDE_CHECKSUMCALC_HPP_
#define INCLUDE_CHECKSUMCALC_HPP_

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../third-party/PicoSHA2/picosha2.h"
#include "ThreadPool.hpp"
#include "Addition.hpp"

using ROCKSDB_NAMESPACE::ColumnFamilyDescriptor;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
using ROCKSDB_NAMESPACE::ColumnFamilyOptions;
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::DBOptions;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteBatch;
using ROCKSDB_NAMESPACE::WriteOptions;

class ChecksumCalc {
 public:
  ChecksumCalc();
  ~ChecksumCalc();

 private:
  void read(const CmdArgs& cmd,
            std::vector<ColumnFamilyDescriptor>& column_families);
  void read_db();
  void read_column(rocksdb::Iterator* iter, size_t i);
  void write(const CmdArgs& cmd,
             const std::vector<ColumnFamilyDescriptor>& column_families);
  void write_db();
  void write_column(ColumnFamilyHandle* handle,
                    const std::vector<std::string>& column_keys,
                    const std::vector<std::string>& column_values);
  void checksum();
  std::string get_hash(const std::string& value);

 public:
  void run(const CmdArgs& cmd);

 private:
  DB* _db;
  ThreadPool* _pool;
  std::vector<ColumnFamilyHandle*> _handles;
  std::vector<std::vector<std::string>> _keys;
  std::vector<std::vector<std::string>> _values;
  std::vector<std::vector<std::string>> _hashes;
  std::mutex _keyval_mutex;
};

void print_db(DB* db, const std::vector<ColumnFamilyHandle*>& handles);

#endif  // INCLUDE_CHECKSUMCALC_HPP_
