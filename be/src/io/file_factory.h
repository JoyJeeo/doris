// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
#pragma once

#include <gen_cpp/PlanNodes_types.h>
#include <gen_cpp/Types_types.h>
#include <glog/logging.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "common/factory_creator.h"
#include "common/status.h"
#include "io/fs/file_reader_options.h"
#include "io/fs/file_reader_writer_fwd.h"

namespace doris {
namespace io {
class FileSystem;
class FileWriter;
} // namespace io
class ExecEnv;
class RuntimeProfile;
class RuntimeState;

struct FileSystemProperties {
    TFileType::type system_type;
    std::map<std::string, std::string> properties;
    THdfsParams hdfs_params;
    std::vector<TNetworkAddress> broker_addresses;
};

struct FileDescription {
    std::string path;
    int64_t start_offset;
    int64_t file_size;
};

class FileFactory {
    ENABLE_FACTORY_CREATOR(FileFactory);

public:
    static io::FileCachePolicy get_cache_policy(RuntimeState* state);

    /// Create FileWriter
    static Status create_file_writer(TFileType::type type, ExecEnv* env,
                                     const std::vector<TNetworkAddress>& broker_addresses,
                                     const std::map<std::string, std::string>& properties,
                                     const std::string& path, int64_t start_offset,
                                     std::unique_ptr<io::FileWriter>& file_writer);

    /// Create FileReader
    static Status create_file_reader(
            RuntimeProfile* profile, const FileSystemProperties& system_properties,
            const FileDescription& file_description, std::shared_ptr<io::FileSystem>* file_system,
            io::FileReaderSPtr* file_reader,
            io::FileCachePolicy cache_policy = io::FileCachePolicy::NO_CACHE);

    // Create FileReader for stream load pipe
    static Status create_pipe_reader(const TUniqueId& load_id, io::FileReaderSPtr* file_reader);

    static Status create_hdfs_reader(const THdfsParams& hdfs_params, const std::string& path,
                                     std::shared_ptr<io::FileSystem>* hdfs_file_system,
                                     io::FileReaderSPtr* reader,
                                     const io::FileReaderOptions& reader_options);

    static Status create_s3_reader(const std::map<std::string, std::string>& prop,
                                   const std::string& path,
                                   std::shared_ptr<io::FileSystem>* s3_file_system,
                                   io::FileReaderSPtr* reader,
                                   const io::FileReaderOptions& reader_options);

    static Status create_broker_reader(const TNetworkAddress& broker_addr,
                                       const std::map<std::string, std::string>& prop,
                                       const FileDescription& file_description,
                                       std::shared_ptr<io::FileSystem>* hdfs_file_system,
                                       io::FileReaderSPtr* reader,
                                       const io::FileReaderOptions& reader_options);

    static TFileType::type convert_storage_type(TStorageBackendType::type type) {
        switch (type) {
        case TStorageBackendType::LOCAL:
            return TFileType::FILE_LOCAL;
        case TStorageBackendType::S3:
            return TFileType::FILE_S3;
        case TStorageBackendType::BROKER:
            return TFileType::FILE_BROKER;
        case TStorageBackendType::HDFS:
            return TFileType::FILE_HDFS;
        default:
            LOG(FATAL) << "not match type to convert, from type:" << type;
        }
        __builtin_unreachable();
    }
};

} // namespace doris
