// Copyright 2022 VladislavRZ <rzhevskii_vladislav@mail.ru>

#include <ChecksumCalc.hpp>

ChecksumCalc::ChecksumCalc()
    :_db(nullptr)
    ,_pool(nullptr) {}

ChecksumCalc::~ChecksumCalc() {
  delete _db;
  delete _pool;
}

void ChecksumCalc::run(const CmdArgs& cmd) {
  // Set options
  Options options;
  options.create_if_missing = false;

  // Open input DB
  BOOST_LOG_TRIVIAL(info) << "Open data base: " << cmd.input << ";";
  std::vector<std::string> cf;
  Status s = DB::ListColumnFamilies(rocksdb::DBOptions(), cmd.input, &cf);
  if (!s.ok()) throw std::runtime_error(s.ToString());

  std::vector<ColumnFamilyDescriptor> column_families;
  for (auto& i : cf) {
    column_families.push_back(ColumnFamilyDescriptor(i, ColumnFamilyOptions()));
  }
  s = DB::Open(options, cmd.input, column_families, &_handles, &_db);
  if (!s.ok()) throw std::runtime_error(s.ToString());
  print_db(_db, _handles);

  // Read keys and values
  BOOST_LOG_TRIVIAL(info) << "Read keys and values from data base: "
                          << cmd.input <<";";
  _pool = new ThreadPool(cmd.threads);
  read_db();

  // Close input DB
  BOOST_LOG_TRIVIAL(info) << "Close database: " << cmd.input << ";";
  for (auto handle : _handles) {
    s = _db->DestroyColumnFamilyHandle(handle);
    if (!s.ok()) throw std::runtime_error(s.ToString());
  }
  delete _db;
  delete _pool;
  _db = nullptr;
  _pool = nullptr;
  _handles.clear();

  // Calculate checksum
  BOOST_LOG_TRIVIAL(info) << "Calculate checksum;";
  checksum();

  // Set options
  options.create_if_missing = true;
  options.create_missing_column_families = true;
  options.error_if_exists = true;

  // Open output DB
  BOOST_LOG_TRIVIAL(info) << "Open data base: " << cmd.output << ";";
  s = DB::Open(options, cmd.output, column_families, &_handles, &_db);
  if (!s.ok()) throw std::runtime_error(s.ToString());

  // Write keys and values
  BOOST_LOG_TRIVIAL(info) << "Write keys and values to data base: "
                          << cmd.output << ";";
  write_db();
  print_db(_db, _handles);

  // Close output DB
  BOOST_LOG_TRIVIAL(info) << "Close data base: " << cmd.output << ";";
  for (auto handle : _handles) {
    s = _db->DestroyColumnFamilyHandle(handle);
    if (!s.ok()) throw std::runtime_error(s.ToString());
  }
  delete _db;
  _db = nullptr;
}

void ChecksumCalc::read_db() {
  std::vector<rocksdb::Iterator*> iterators;
  _keys.resize(_handles.size());
  _values.resize((_handles.size()));
  for (size_t i = 0; i < _handles.size(); ++i) {
    iterators.push_back(_db->NewIterator(ReadOptions(), _handles[i]));
    _pool->enqueue([&iterators, i, this] { read_column(iterators[i], i); });
  }

  while (!_pool->tasks_empty()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  for (auto iter : iterators) {
    delete iter;
  }
}

void ChecksumCalc::read_column(rocksdb::Iterator* iter, size_t i) {
  // Read keys and values from column
  iter->SeekToFirst();
  std::vector<std::string> keys;
  std::vector<std::string> values;
  while (iter->Valid()) {
    keys.push_back(iter->key().ToString());
    values.push_back(iter->value().ToString());
    iter->Next();
  }

  // Save values
  _keyval_mutex.lock();
  _keys[i] = std::move(keys);
  _values[i] = std::move(values);
  _keyval_mutex.unlock();
}

void ChecksumCalc::write_db() {
  for(size_t i = 0; i < _handles.size(); ++i) {
    write_column(_handles[i], _keys[i], _hashes[i]);
  }
}

void ChecksumCalc::write_column(ColumnFamilyHandle* handle,
                                const std::vector<std::string>& column_keys,
                                const std::vector<std::string>& column_values) {
  WriteBatch batch;
  for (size_t i = 0; i < column_keys.size(); ++i) {
    batch.Put(handle, Slice(column_keys[i]), Slice(column_values[i]));
  }

  Status s = _db->Write(WriteOptions(), &batch);
  if (!s.ok()) BOOST_LOG_TRIVIAL(error) << s.ToString();
}

void ChecksumCalc::checksum() {
  _hashes.resize(_values.size());
  for (size_t i = 0; i < _values.size(); ++i) {
    for (size_t j = 0; j < _values[i].size(); ++j) {
      _hashes[i].push_back(get_hash(_values[i][j]));
    }
  }
}

std::string ChecksumCalc::get_hash(const std::string& value) {
  std::vector<unsigned char> entry_data;
  for (unsigned char val : value) {
    entry_data.push_back(val);
  }
  return picosha2::hash256_hex_string(entry_data);
}

void print_db(DB* db, const std::vector<ColumnFamilyHandle*>& handles) {
  std::vector<rocksdb::Iterator*> iterators;
  bool stop_flag = false;

  if (handles.empty()) {
    BOOST_LOG_TRIVIAL(warning) << "Empty data base.";
    return;
  }
  std::cout << "\n|";

  for (size_t i = 0; i < handles.size(); ++i) {
    iterators.push_back(db->NewIterator(ReadOptions(), handles[i]));
    iterators[i]->SeekToFirst();
    std::cout << std::setw(70) << handles[i]->GetName() << " |";
  }

  std::cout << "\n|";
  for (size_t i = 0; i < handles.size(); ++i) {
    std::cout << std::setfill('-') << std::setw(72) << "|" << std::setfill(' ');
  }

  while(!stop_flag) {
    stop_flag = true;
    std::cout << "\n|";
    for (auto iter : iterators) {
      if (!iter->Valid()) {
        std::cout << std::setw(72) << " |";
      } else {
        stop_flag = false;
        std::stringstream ss;
        ss << iter->key().ToString() << ":" << iter->value().ToString();
        std::cout << std::setw(70) << ss.str() << " |";
        iter->Next();
      }
    }
    std::cout << "\n|";
    for (size_t i = 0; i < handles.size(); ++i) {
      std::cout << std::setfill('-') << std::setw(72) << "|" << std::setfill(' ');
    }
  }

  std::cout << std::endl;

  for (auto iter : iterators) {
    delete iter;
  }
}