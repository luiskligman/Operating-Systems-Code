#ifndef PROJ2_LIB_DOMAIN_SOCKET_H_
#define PROJ2_LIB_DOMAIN_SOCKET_H_

#include <string>
#include <cstddef>

namespace proj2 {

// -----------------------------------------------------------------------------
// Base class
// -----------------------------------------------------------------------------
class UnixDomainBase {
 public:
  explicit UnixDomainBase(const std::string& path);
  virtual ~UnixDomainBase();

  int socket_fd() const;
 protected:
  std::string path_;
  int fd_;
};

// -----------------------------------------------------------------------------
// STREAM SERVER (SOCK_STREAM)
// -----------------------------------------------------------------------------
class UnixDomainStreamServer : public UnixDomainBase {
 public:
  explicit UnixDomainStreamServer(const std::string& path);
  ~UnixDomainStreamServer() override;

  void Init(std::size_t backlog = 4);     // bind + listen
  int Accept();                           // blocking

  std::string Read(int client_fd, std::size_t max_bytes);
  std::size_t Write(int client_fd, const void* buf, std::size_t count);
};

// -----------------------------------------------------------------------------
// STREAM CLIENT
// -----------------------------------------------------------------------------
class UnixDomainStreamClient : public UnixDomainBase {
 public:
  explicit UnixDomainStreamClient(const std::string& path);
  ~UnixDomainStreamClient() override;

  void Init();                            // connect

  std::string Read(std::size_t max_bytes);
  std::size_t Write(const void* buf, std::size_t count);
};

// -----------------------------------------------------------------------------
// DATAGRAM ENDPOINT (bound server or reply socket)
// -----------------------------------------------------------------------------
class UnixDomainDatagramEndpoint : public UnixDomainBase {
 public:
  explicit UnixDomainDatagramEndpoint(const std::string& path);
  ~UnixDomainDatagramEndpoint() override;

  void Init(); // bind

  std::size_t SendTo(const std::string& peer_path,
                     const void* buf,
                     std::size_t count);

  std::string RecvFrom(std::string* peer_path_out,
                       std::size_t max_bytes);
};


// -----------------------------------------------------------------------------
// DATAGRAM CLIENT
// -----------------------------------------------------------------------------
class UnixDomainDatagramClient : public UnixDomainBase {
 public:
  explicit UnixDomainDatagramClient(const std::string& local_path = "");
  ~UnixDomainDatagramClient() override;

  void Init(bool bind_local = false);

  std::size_t SendTo(const std::string& server_path,
                     const void* buf,
                     std::size_t count);

  std::string RecvFrom(std::string* peer_path_out,
                       std::size_t max_bytes);
};

} // namespace proj2

#endif  // PROJ2_LIB_DOMAIN_SOCKET_H_
