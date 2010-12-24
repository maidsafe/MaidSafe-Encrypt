/* Copyright (c) 2010 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAIDSAFE_DEDUP_ANALYSER_FILESYSTEM_ANALYSER_H_
#define MAIDSAFE_DEDUP_ANALYSER_FILESYSTEM_ANALYSER_H_

#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
//  #include <boost/signals2.hpp>
//  #include <boost/signals2/connection.hpp>
#include <boost/asio.hpp>
//  #include <boost/thread.hpp>
//  #include <boost/bind.hpp>
#include <QObject>
#include <string>
#include <utility>
#include <iostream>
#include "maidsafe/base/crypto.h"

namespace fs3 = boost::filesystem3;
//  namespace bs2 = boost::signals2;

namespace maidsafe {

struct FileInfo {
  explicit FileInfo(const fs3::path &file_path_in)
      : file_path(file_path_in), file_hash(), file_size(0) {}
  fs3::path file_path;
  std::string file_hash;
  boost::uintmax_t file_size;
  bool operator < (const FileInfo &r) const { return file_hash < r.file_hash; }
};


std::string SHA1(const fs3::path &file_path);

class FilesystemAnalyser : public QObject {
  Q_OBJECT
 public:
  explicit FilesystemAnalyser(boost::shared_ptr<boost::asio::io_service> io)
      : asio_service_(io) {
//     work_.reset(new boost::asio::io_service::work(io_service_));
//     if (boost::thread::hardware_concurrency() > 1)
//       cores_ = boost::thread::hardware_concurrency() -1;
//     else
//       cores_ = 4;
//
//     for (uint i = 0; i < cores_ ; ++i) {
//       thread_group_.create_thread(boost::bind
//               (&boost::asio::io_service::run, &io_service_));
//     }
//     boost::shared_ptr<crypto::Crypto> crypt_(new crypto::Crypto);
//     crypt_->set_hash_algorithm(crypto::Adler_32);

  }
  ~FilesystemAnalyser() {
    Stop();
  }
  void Stop() {
//     work_.reset();
//     thread_group_.join_all();
  }
  fs3::path Normalise(const fs3::path &directory_path);
  void ProcessFile(const fs3::path &file_path);
  void ProcessDirectory(const fs3::path &directory_path);
 signals:
  void OnFileProcessed(FileInfo file_info);
  void OnDirectoryEntered(fs3::path directory_path);
  void OnFailure(std::string error_message);
 private:
  FilesystemAnalyser(const FilesystemAnalyser&);
  FilesystemAnalyser &operator=(const FilesystemAnalyser&);
  boost::shared_ptr<boost::asio::io_service> asio_service_;
//  boost::shared_ptr<boost::asio::io_service::work> work_;
//   boost::uint16_t cores_;
//   boost::thread_group thread_group_;
//   boost::shared_ptr<crypto::Crypto> crypt_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_DEDUP_ANALYSER_FILESYSTEM_ANALYSER_H_
