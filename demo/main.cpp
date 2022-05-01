#include <boost/program_options.hpp>

#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>

#include <iostream>
#include <string>
#include <vector>

namespace  po = boost::program_options;

struct CmdArgs {
  std::string log_lvl;
  std::string output;
  std::string input;
  unsigned int threads;
};

bool parse_cmd(int argc, char* argv[], CmdArgs& cmd_args) {
  // Add options
  po::options_description visible("Options");
  visible.add_options()
      ("help, h", "produce help message")
      ("log-level", po::value<std::string>(&cmd_args.log_lvl)->default_value("error"),
       "info|warning|error")
      ("thread-count",
       po::value<unsigned int>(&cmd_args.threads)->default_value(3),
       "count of threads")
      ("output",
       po::value<std::string>(&cmd_args.output)->default_value("path/to/input"),
        "path/to/output")
      ;

  po::options_description hidden("Hidden options");
  hidden.add_options()
      ("input-file", po::value<std::string>(&cmd_args.input), "input file");

  // Add positional options
  po::positional_options_description p;
  p.add("input-file", -1);

  // Parse options
  po::options_description cmd_options;
  cmd_options.add(visible).add(hidden);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(cmd_options).positional(p).run(), vm);
  po::notify(vm);

  // Check errors
  if (vm.count("help")) {
    std::cout << "\nUsage:\n" << argv[0] << " [options] <path/to/input>\n" << std::endl;
    std::cout << visible << std::endl;
    return false;
  }

  if (vm.count("input-file"))
  {
    std::cout << "Input file are: " << vm["input-file"].as<std::string>() << std::endl;
  } else {
    std::cout << "Program can't work without input file." << std::endl;
    return false;
  }

  return true;
}

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

int main() {
  try {
    std::string kDBPath = "MyDataBase";

    // open DB
    Options options;
    options.create_if_missing = true;
    DB* db;

    std::vector<std::string> cf;
    Status s = DB::ListColumnFamilies(rocksdb::DBOptions(), kDBPath, &cf);
    assert(s.ok());

    std::vector<ColumnFamilyDescriptor> column_families;
    for (auto& i : cf) {
      column_families.push_back(ColumnFamilyDescriptor(i, ColumnFamilyOptions()));
    }

    std::vector<ColumnFamilyHandle*> handles;
//    ColumnFamilyHandle* cff;
    s = DB::Open(options, kDBPath, column_families, &handles, &db);
    assert(s.ok());

//    s = db->CreateColumnFamily(ColumnFamilyOptions(), "new_cf", &cff);
//    assert(s.ok());
//
//    handles.push_back(cff);
//
//    s = db->Put(WriteOptions(), handles[1], Slice("key2"), Slice("value2"));
//    assert(s.ok());

    rocksdb::Iterator* iter = db->NewIterator(ReadOptions(), handles[1]);
    iter->SeekToLast();
    std::cout << iter->value().ToString() << std::endl;
    std::cout << iter->key().ToString() << std::endl;

    // create column family
//    ColumnFamilyHandle* cf;
//    s = db->CreateColumnFamily(ColumnFamilyOptions(), "new_cf", &cf);
//    assert(s.ok());

//    std::string value;
//    s = db->Get(ReadOptions(), "key", &value);
//    assert(s.ok());
//    std::cout << value << std::endl;

//    s = db->DropColumnFamily(handles[1]);
//    assert(s.ok());

    // close DB
    for (auto handle : handles) {
      s = db->DestroyColumnFamilyHandle(handle);
      assert(s.ok());
    }

    delete iter;
    delete db;

//    // open DB with two column families
//    std::vector<ColumnFamilyDescriptor> column_families;
//    // have to open default column family
//    column_families.push_back(ColumnFamilyDescriptor(
//        ROCKSDB_NAMESPACE::kDefaultColumnFamilyName, ColumnFamilyOptions()));
//    // open the new one, too
//    column_families.push_back(ColumnFamilyDescriptor(
//        "new_cf", ColumnFamilyOptions()));
//    std::vector<ColumnFamilyHandle*> handles;
//    s = DB::Open(DBOptions(), kDBPath, column_families, &handles, &db);
//    assert(s.ok());
//
//    // put and get from non-default column family
//    s = db->Put(WriteOptions(), handles[1], Slice("key"), Slice("value"));
//    assert(s.ok());
//    std::string value;
//    s = db->Get(ReadOptions(), handles[1], Slice("key"), &value);
//    assert(s.ok());
//    std::cout << value << std::endl;
//
//    // atomic write
//    WriteBatch batch;
//    batch.Put(handles[0], Slice("key2"), Slice("value2"));
//    batch.Put(handles[1], Slice("key3"), Slice("value3"));
//    batch.Delete(handles[0], Slice("key"));
//    s = db->Write(WriteOptions(), &batch);
//    assert(s.ok());
//
//    // drop column family
//    s = db->DropColumnFamily(handles[1]);
//    assert(s.ok());
//
//    // close db
//    for (auto handle : handles) {
//      s = db->DestroyColumnFamilyHandle(handle);
//      assert(s.ok());
//    }
//    delete db;
//    CmdArgs cmd_args;
//    if (!parse_cmd(argc, argv, cmd_args))
//      return 0;
  }
  catch(std::exception &e)
  {
    std::cout << "Exception: " << e.what() << std::endl;
    return 1;
  }
  catch(...)
  {
    std::cout << "Unknown error!" << std::endl;
  }
}